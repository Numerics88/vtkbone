"""
For Tensor of inertia refer to http://en.wikipedia.org/wiki/Moment_of_inertia 
"""

from __future__ import division
import sys
from math import *
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import test_geometries
import traceback
import itertools
import unittest


def vector_angle(u,v):
    return acos(inner(u,v)/sqrt(inner(u,u)*inner(v,v)))


def vector_smallest_angle(u,v):
    x = abs(inner(u,v))/sqrt(inner(u,u)*inner(v,v))
    # Need the following check occasionally because of fluky round-offs
    if x >= 1:
        return 0
    else:
        return acos(x)


def is_parallel(u,v,tol=1E-6):
    return vector_smallest_angle(u,v) < tol


def tensor_of_inertia_standard_analysis(toi, I_ref, COM_ref, count_ref, mass_ref):

    # Check some simple parameters.
    #
    assert(toi.GetCount() == count_ref)
    assert(toi.GetVolume() == mass_ref)
    assert(toi.GetMass() == mass_ref)
    COM = toi.GetCenterOfMass()
    assert(alltrue(abs(COM - COM_ref) < 1E-6))

    # Check the tensor of inertia about the origin.
    #
    I_vtk = vtkbone.vtkboneTensor()
    toi.GetTensorOfInertiaAboutOrigin(I_vtk)
    I = zeros((3,3), float)
    for i,j in itertools.product(list(range(3)),list(range(3))):
        I[i,j] = I_vtk.GetComponent(i,j)
    assert(alltrue(abs(I - I_ref) < 1E-6))
    
    # Check the tensor of inertia about the center of mass.
    #
    I_center_vtk = vtkbone.vtkboneTensor()
    toi.GetTensorOfInertia(I_center_vtk)
    I_center = zeros((3,3), float)
    for i,j in itertools.product(list(range(3)),list(range(3))):
        I_center[i,j] = I_center_vtk.GetComponent(i,j)
    I_center_ref = I - mass_ref*(inner(COM,COM)*identity(3) - outer(COM,COM))
    assert(alltrue(abs(I_center - I_center_ref) < 1E-6))

    # Now we'll check the principle axes and principle moments.
    #
    U_vtk = vtk.vtkMatrix3x3()
    toi.GetEigenvectors(U_vtk)
    U = zeros((3,3), float)
    for i,j in itertools.product(list(range(3)),list(range(3))):
        U[i,j] = U_vtk.GetElement(i,j)
    e = array(toi.GetPrincipalMoments())
    e_sort_key = argsort(e)
    e_sorted = e[e_sort_key]
    U_sorted = U.take(e_sort_key,axis=1)
    e_ref, U_ref = numpy.linalg.eig(I_center)
    e_ref_sort_key = argsort(e_ref)
    e_ref_sorted = e_ref[e_ref_sort_key]
    U_ref_sorted = U_ref.take(e_ref_sort_key,axis=1)
    assert(alltrue(e_sorted - e_ref_sorted < 1E-5))
    for j in range(3):
        assert(is_parallel(U_sorted[:,j], U_ref_sorted[:,j]))

    # Now we'll check the assertion that the principle axes come out
    # sorted in the order x,y,z .
    #
    assert(alltrue(toi.GetPrincipalAxisClosestToX() == U[:,0]))
    assert(alltrue(toi.GetPrincipalAxisClosestToY() == U[:,1]))
    assert(alltrue(toi.GetPrincipalAxisClosestToZ() == U[:,2]))
    assert(toi.GetPrincipalMoment0() == e[0])
    assert(toi.GetPrincipalMoment1() == e[1])
    assert(toi.GetPrincipalMoment2() == e[2])
    ux = array((1,0,0))
    uy = array((0,1,0))
    uz = array((0,0,1))
    assert(vector_smallest_angle(U[:,0], ux) <= vector_smallest_angle(U[:,1], ux))
    assert(vector_smallest_angle(U[:,0], ux) <= vector_smallest_angle(U[:,2], ux))
    assert(vector_smallest_angle(U[:,1], uy) <= vector_smallest_angle(U[:,2], uy))
    assert(vector_smallest_angle(U[:,1], uy) <= vector_smallest_angle(U[:,0], uy))
    assert(vector_smallest_angle(U[:,2], uz) <= vector_smallest_angle(U[:,0], uz))
    assert(vector_smallest_angle(U[:,2], uz) <= vector_smallest_angle(U[:,1], uz))


class TestTensorOfInertia (unittest.TestCase):

    def test_step_shape (self):
        
        # Generate geometry and analyze it with vtkboneTensorOfInertia.
        #
        image = test_geometries.generate_step_image()
        toi = vtkbone.vtkboneTensorOfInertia()
        toi.SetInputData(image)
        toi.Update()
        
        # Expected Mass and Center of Mass
        count_ref = 6
        mass_ref = count_ref
        COM_ref = array((5/6,1,5/6))

        # Calculate the expected tensor of inertia relative to the origin.
        #
        I_ref = zeros((3,3), float)
        I_ref[0,0] = 2*(0.5**2 + 0.5**2) + 3*(0.5**2 + 1.5**2) + (1.5**2 + 1.5**2)
        I_ref[1,1] = 2*(0.5**2 + 0.5**2) + 4*(0.5**2 + 1.5**2)
        I_ref[2,2] = I_ref[0,0]
        I_ref[1,0] = I_ref[0,1] = -(2*0.5*0.5 + 3*0.5*1.5 + 1.5*1.5)
        I_ref[2,0] = I_ref[0,2] = -(2*0.5*0.5 + 4*0.5*1.5)
        I_ref[1,2] = I_ref[2,1] = -(2*0.5*0.5 + 3*0.5*1.5 + 1.5*1.5)

        tensor_of_inertia_standard_analysis(toi, I_ref, COM_ref, count_ref, mass_ref)

        # Do the same thing with the mesh of the image
        hexa = vtkbone.vtkboneImageToMesh()
        hexa.SetInputData(image)
        hexa.Update()
        mesh = hexa.GetOutput()
        toi2 = vtkbone.vtkboneTensorOfInertia()
        toi2.SetInputData(mesh)
        toi2.Update()
        tensor_of_inertia_standard_analysis(toi2, I_ref, COM_ref, count_ref, mass_ref)


    def test_step_shape_nonsquare (self):
        
        # Generate geometry and analyze it with vtkboneTensorOfInertia.
        #
        image = test_geometries.generate_step_offset_image()
        toi = vtkbone.vtkboneTensorOfInertia()
        toi.SetInputData (image)
        toi.Update()
        
        # Expected Mass and Center of Mass
        count_ref = 6
        voxel_vol = 0.5*1*1.5
        mass_ref = voxel_vol*count_ref
        COM_ref = array(((4*1+0.75*2)/6,2,(4*0.25+2*1.75)/6))

        # Calculate the expected tensor of inertia relative to the origin.
        #
        cellmap = 100*ones((2,2,2))
        cellmap[:,1,:] = 200
        cellmap[1,:,1] = 0
        offset = array((0.5,1.0,-0.5))
        spacing = array((0.5,1.0,1.5))
        i,j,k = numpy.ogrid[0:2,0:2,0:2]
        x = offset[0] + (i+0.5)*spacing[0]
        y = offset[1] + (j+0.5)*spacing[1]
        z = offset[2] + (k+0.5)*spacing[2]
        mask = cellmap[i,j,k] != 0
        I_ref = zeros((3,3), float)
        I_ref[0,0] = voxel_vol * sum((y**2 + z**2)*mask)
        I_ref[1,1] = voxel_vol * sum((z**2 + x**2)*mask)
        I_ref[2,2] = voxel_vol * sum((x**2 + y**2)*mask)
        I_ref[1,0] = I_ref[0,1] = -voxel_vol * sum(x*y*mask)
        I_ref[2,0] = I_ref[0,2] = -voxel_vol * sum(x*z*mask)
        I_ref[1,2] = I_ref[2,1] = -voxel_vol * sum(y*z*mask)

        tensor_of_inertia_standard_analysis(toi, I_ref, COM_ref, count_ref, mass_ref)

        # Do the same thing with the mesh of the image
        hexa = vtkbone.vtkboneImageToMesh()
        hexa.SetInputData(image)
        hexa.Update()
        mesh = hexa.GetOutput()
        toi2 = vtkbone.vtkboneTensorOfInertia()
        toi2.SetInputData(mesh)
        toi2.Update()
        tensor_of_inertia_standard_analysis(toi2, I_ref, COM_ref, count_ref, mass_ref)

     
    def test_step_shape_specific_value (self):
        
        # Generate geometry and analyze it with vtkboneTensorOfInertia.
        #
        image = test_geometries.generate_step_image()
        toi = vtkbone.vtkboneTensorOfInertia()
        toi.SetInputData (image)
        toi.UseSpecificValueOn()
        toi.SetSpecificValue(200)
        toi.Update()
        
        # Expected Mass and Center of Mass
        count_ref = 3
        mass_ref = count_ref
        COM_ref = array((5/6,1.5,5/6))

        # Calculate the expected tensor of inertia relative to the origin.
        #
        I_ref = zeros((3,3), float)
        I_ref[0,0] = 2*(0.5**2 + 1.5**2) + (1.5**2 + 1.5**2)
        I_ref[1,1] = (0.5**2 + 0.5**2) + 2*(0.5**2 + 1.5**2)
        I_ref[2,2] = 2*(0.5**2 + 1.5**2) + (1.5**2 + 1.5**2)
        I_ref[1,0] = I_ref[0,1] = -(2*0.5*1.5 + 1.5*1.5)
        I_ref[2,0] = I_ref[0,2] = -(0.5*0.5 + 2*0.5*1.5)
        I_ref[1,2] = I_ref[2,1] = -(2*0.5*1.5 + 1.5*1.5)

        tensor_of_inertia_standard_analysis(toi, I_ref, COM_ref, count_ref, mass_ref)


    def test_step_shape_threshold (self):
        
        # Generate geometry and analyze it with vtkboneTensorOfInertia.
        #
        image = test_geometries.generate_step_image()
        toi = vtkbone.vtkboneTensorOfInertia()
        toi.SetInputData (image)
        toi.UseThresholdsOn()
        toi.SetLowerThreshold(150)
        toi.SetUpperThreshold(250)
        toi.Update()
        
        # Expected Mass and Center of Mass
        count_ref = 3
        mass_ref = count_ref
        COM_ref = array((5/6,1.5,5/6))

        # Calculate the expected tensor of inertia relative to the origin.
        #
        I_ref = zeros((3,3), float)
        I_ref[0,0] = 2*(0.5**2 + 1.5**2) + (1.5**2 + 1.5**2)
        I_ref[1,1] = (0.5**2 + 0.5**2) + 2*(0.5**2 + 1.5**2)
        I_ref[2,2] = 2*(0.5**2 + 1.5**2) + (1.5**2 + 1.5**2)
        I_ref[1,0] = I_ref[0,1] = -(2*0.5*1.5 + 1.5*1.5)
        I_ref[2,0] = I_ref[0,2] = -(0.5*0.5 + 2*0.5*1.5)
        I_ref[1,2] = I_ref[2,1] = -(2*0.5*1.5 + 1.5*1.5)

        tensor_of_inertia_standard_analysis(toi, I_ref, COM_ref, count_ref, mass_ref)


if __name__ == '__main__':
    unittest.main()
