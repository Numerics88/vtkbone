from __future__ import division

import unittest
from numpy.core import *
import numpy
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone


def stress_strain_isotropic (E, nu):
    s = nu/(1-nu)
    t = (1-2*nu)/(2*(1-nu))
    D = array ([[ 1, s, s, 0, 0, 0],
                [ s, 1, s, 0, 0, 0],
                [ s, s, 1, 0, 0, 0],
                [ 0, 0, 0, t, 0, 0],
                [ 0, 0, 0, 0, t, 0],
                [ 0, 0, 0, 0, 0, t]]
               )
    D *= E*(1-nu)/((1+nu)*(1-2*nu))
    return D


def stress_strain_orthotropic (E, nu, G):
    nu12 = nu[0]
    nu20 = nu[1]
    nu01 = nu[2]
    nu21 = (E[2]/E[1])*nu12
    nu02 = (E[0]/E[2])*nu20
    nu10 = (E[1]/E[0])*nu01
    D = zeros((6,6), float)
    D[:3,:3] = array ([[     1/E[0], -nu10/E[1], -nu20/E[2]],
                       [ -nu01/E[0],     1/E[1], -nu21/E[2]],
                       [ -nu02/E[0], -nu12/E[1],     1/E[2]]])
    D[3,3] = 1/G[0]
    D[4,4] = 1/G[1]
    D[5,5] = 1/G[2]
    return numpy.linalg.inv(D)

class TestCoarsenModel (unittest.TestCase):

    def test_iso_x_gradient (self):
        D = array((( 0,  1,  3,  6, 10, 15),
                   ( 1,  2,  4,  7, 11, 16),
                   ( 3,  4,  5,  8, 12, 17),
                   ( 6,  7,  8,  9, 13, 18),
                   (10, 11, 12, 13, 14, 19),
                   (15, 16, 17, 18, 19, 20)))
        SS = vtkbone.vtkboneStressStrainMatrix()
        D_vtk = numpy_to_vtk (D)
        SS.SetStressStrainMatrix (D_vtk)
        ut = zeros (21)
        ut_vtk = vtk.vtkFloatArray()
        ut_vtk.SetNumberOfTuples(21)
        SS.GetUpperTriangularPacked(ut_vtk)
        ut = vtk_to_numpy (ut_vtk)
        for i in range(21):
            self.assertEqual (ut[i], i)

    def test_upper_packed_to_square (self):
        ut = array ((0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20))
        D_ref = array (( 0,  1,  3,  6, 10, 15,
                         1,  2,  4,  7, 11, 16,
                         3,  4,  5,  8, 12, 17,
                         6,  7,  8,  9, 13, 18,
                        10, 11, 12, 13, 14, 19,
                        15, 16, 17, 18, 19, 20))
        SS = vtkbone.vtkboneStressStrainMatrix()
        ut_vtk = numpy_to_vtk (ut)
        SS.SetUpperTriangularPacked(ut_vtk);
        D_vtk = vtk.vtkFloatArray()
        SS.GetStressStrainMatrix (D_vtk)
        D = vtk_to_numpy (D_vtk)
        self.assertEqual (D.shape, (36,))
        for i in range (36):
            self.assertEqual (D[i], D_ref[i])

    def test_set_isotropic (self):
        E = 6000
        nu = 0.4
        SS = vtkbone.vtkboneStressStrainMatrix()
        SS.SetIsotropic (E, nu)
        D_vtk = vtk.vtkFloatArray()
        SS.GetStressStrainMatrix (D_vtk)
        D = vtk_to_numpy (D_vtk)
        self.assertEqual (D.shape, (36,))
        D_ref = stress_strain_isotropic (E, nu)
        D_ref.shape = (36,)
        self.assertTrue (alltrue(D - D_ref) < 1E-4)

    def test_set_orthotropic (self):
        E = array((1000, 1100, 1200))
        nu = array((0.2, 0.25, 0.3))
        # These values are not necessarily consistent
        G = array((1100/(2*(1+0.25)), 1200/(2*(1+0.3)), 1000/(2*(1+0.2))))
        SS = vtkbone.vtkboneStressStrainMatrix()
        SS.SetOrthotropic (E, nu, G)
        D_vtk = vtk.vtkFloatArray()
        SS.GetStressStrainMatrix (D_vtk)
        D = vtk_to_numpy (D_vtk)
        self.assertEqual (D.shape, (36,))
        D_ref = stress_strain_orthotropic (E, nu, G)
        D_ref.shape = (36,)
        self.assertTrue (alltrue(D - D_ref) < 1E-4)

if __name__ == '__main__':
    unittest.main()
