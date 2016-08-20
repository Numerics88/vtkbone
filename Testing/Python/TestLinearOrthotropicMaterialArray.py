from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import unittest


class TestLinearOrthotropicMaterialArray (unittest.TestCase):

    def test_simple (self):
        material = vtkbone.vtkboneLinearOrthotropicMaterialArray()
        E = array (((1000,1100,1200), (2000,2200,2400)), float)
        E_vtk = numpy_to_vtk (E, array_type=vtk.VTK_FLOAT)
        nu = array (((0.25,0.3,0.35), (0.4,0.45,0.55)), float)
        nu_vtk = numpy_to_vtk (nu, array_type=vtk.VTK_FLOAT)
        # These values are not necessarily consistent
        G = E/(2*(1+nu))
        G_vtk = numpy_to_vtk(G, array_type=vtk.VTK_FLOAT)
        material.SetYoungsModulus(E_vtk)
        material.SetPoissonsRatio(nu_vtk)
        material.SetShearModulus(G_vtk)
        self.assertEqual (material.GetSize(), 2)
        E2_vtk = material.GetYoungsModulus()
        E2 = vtk_to_numpy(E2_vtk)
        self.assertEqual (E2.shape, (2,3))
        self.assertTrue (alltrue(abs(E2 - E) < 1E-4))
        nu2_vtk = material.GetPoissonsRatio()
        nu2 = vtk_to_numpy(nu2_vtk)
        self.assertEqual (nu2.shape, (2,3))
        self.assertTrue (alltrue(abs(nu2 - nu) < 1E-7))
        G2_vtk = material.GetShearModulus()
        G2 = vtk_to_numpy(G2_vtk)
        self.assertEqual (G2.shape, (2,3))
        self.assertTrue (alltrue(abs(G2 - G) < 1E-4))


    def test_set_item (self):
        material = vtkbone.vtkboneLinearOrthotropicMaterialArray()
        material.Resize(2)
        E = array (((1000,1100,1200), (2000,2200,2400)), float)
        nu = array (((0.25,0.3,0.35), (0.4,0.45,0.55)), float)
        # These values are not necessarily consistent
        G = E/(2*(1+nu))
        mat1 = vtkbone.vtkboneLinearOrthotropicMaterial()
        mat1.SetYoungsModulusX(E[0,0])
        mat1.SetYoungsModulusY(E[0,1])
        mat1.SetYoungsModulusZ(E[0,2])
        mat1.SetPoissonsRatioYZ(nu[0,0])
        mat1.SetPoissonsRatioZX(nu[0,1])
        mat1.SetPoissonsRatioXY(nu[0,2])
        mat1.SetShearModulusYZ (G[0,0])
        mat1.SetShearModulusZX (G[0,1])
        mat1.SetShearModulusXY (G[0,2])
        material.SetItem(0, mat1)
        mat2 = vtkbone.vtkboneLinearOrthotropicMaterial()
        mat2.SetYoungsModulusX(E[1,0])
        mat2.SetYoungsModulusY(E[1,1])
        mat2.SetYoungsModulusZ(E[1,2])
        mat2.SetPoissonsRatioYZ(nu[1,0])
        mat2.SetPoissonsRatioZX(nu[1,1])
        mat2.SetPoissonsRatioXY(nu[1,2])
        mat2.SetShearModulusYZ (G[1,0])
        mat2.SetShearModulusZX (G[1,1])
        mat2.SetShearModulusXY (G[1,2])
        material.SetItem(1, mat2)
        E2_vtk = material.GetYoungsModulus()
        E2 = vtk_to_numpy(E2_vtk)
        self.assertEqual (E2.shape, (2,3))
        self.assertTrue (alltrue(abs(E2 - E) < 1E-4))
        nu2_vtk = material.GetPoissonsRatio()
        nu2 = vtk_to_numpy(nu2_vtk)
        self.assertEqual (nu2.shape, (2,3))
        self.assertTrue (alltrue(abs(nu2 - nu) < 1E-7))
        G2_vtk = material.GetShearModulus()
        G2 = vtk_to_numpy(G2_vtk)
        self.assertEqual (G2.shape, (2,3))
        self.assertTrue (alltrue(abs(G2 - G) < 1E-4))


    def test_set_scaled_item (self):
        material = vtkbone.vtkboneLinearOrthotropicMaterialArray()
        material.Resize(2)
        E = array (((1000,1100,1200), (2000,2200,2400)), float)
        nu = array (((0.25,0.3,0.35), (0.4,0.45,0.55)), float)
        # These values are not necessarily consistent
        G = E/(2*(1+nu))
        mat1 = vtkbone.vtkboneLinearOrthotropicMaterial()
        mat1.SetYoungsModulusX(E[0,0])
        mat1.SetYoungsModulusY(E[0,1])
        mat1.SetYoungsModulusZ(E[0,2])
        mat1.SetPoissonsRatioYZ(nu[0,0])
        mat1.SetPoissonsRatioZX(nu[0,1])
        mat1.SetPoissonsRatioXY(nu[0,2])
        mat1.SetShearModulusYZ (G[0,0])
        mat1.SetShearModulusZX (G[0,1])
        mat1.SetShearModulusXY (G[0,2])
        material.SetScaledItem(0, mat1, 1.5)
        mat2 = vtkbone.vtkboneLinearOrthotropicMaterial()
        mat2.SetYoungsModulusX(E[1,0])
        mat2.SetYoungsModulusY(E[1,1])
        mat2.SetYoungsModulusZ(E[1,2])
        mat2.SetPoissonsRatioYZ(nu[1,0])
        mat2.SetPoissonsRatioZX(nu[1,1])
        mat2.SetPoissonsRatioXY(nu[1,2])
        mat2.SetShearModulusYZ (G[1,0])
        mat2.SetShearModulusZX (G[1,1])
        mat2.SetShearModulusXY (G[1,2])
        material.SetScaledItem(1, mat2, 2)
        E2_vtk = material.GetYoungsModulus()
        E2 = vtk_to_numpy(E2_vtk)
        self.assertEqual (E2.shape, (2,3))
        self.assertTrue (alltrue(abs(E2[0] - 1.5*E[0]) < 1E-4))
        self.assertTrue (alltrue(abs(E2[1] - 2*E[1]) < 1E-4))
        nu2_vtk = material.GetPoissonsRatio()
        nu2 = vtk_to_numpy(nu2_vtk)
        self.assertEqual (nu2.shape, (2,3))
        self.assertTrue (alltrue(abs(nu2 - nu) < 1E-7))
        G2_vtk = material.GetShearModulus()
        G2 = vtk_to_numpy(G2_vtk)
        self.assertEqual (G2.shape, (2,3))
        self.assertTrue (alltrue(abs(G2[0] - 1.5*G[0]) < 1E-4))
        self.assertTrue (alltrue(abs(G2[1] - 2*G[1]) < 1E-4))


if __name__ == '__main__':
    unittest.main()
