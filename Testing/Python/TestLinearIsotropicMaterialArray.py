from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import unittest


class TestLinearIsotropicMaterialArray (unittest.TestCase):

    def test_simple (self):
        material = vtkbone.vtkboneLinearIsotropicMaterialArray()
        E = vtk.vtkFloatArray()
        E.SetNumberOfTuples(2)
        E.SetComponent (0,0,1234.5)
        E.SetComponent (1,0,678.9)
        nu = vtk.vtkFloatArray()
        nu.SetNumberOfTuples(2)
        nu.SetComponent (0,0,0.4)
        nu.SetComponent (1,0,0.5)
        material.SetYoungsModulus(E)
        material.SetPoissonsRatio(nu)
        self.assertEqual (material.GetSize(), 2)
        E2_vtk = material.GetYoungsModulus()
        E2 = vtk_to_numpy(E2_vtk)
        self.assertEqual (len(E2), 2)
        self.assertAlmostEqual (E2[0], 1234.5, delta=1E-4)
        self.assertAlmostEqual (E2[1], 678.9, delta=1E-4)
        nu2_vtk = material.GetPoissonsRatio()
        nu2 = vtk_to_numpy(nu2_vtk)
        self.assertEqual (len(nu2), 2)
        self.assertAlmostEqual (nu2[0], 0.4, delta=1E-6)
        self.assertAlmostEqual (nu2[1], 0.5, delta=1E-6)


    def test_set_item (self):
        material = vtkbone.vtkboneLinearIsotropicMaterialArray()
        material.Resize(2)
        mat1 = vtkbone.vtkboneLinearIsotropicMaterial()
        mat1.SetYoungsModulus(1234.5)
        mat1.SetPoissonsRatio(0.4)
        material.SetItem(0, mat1)
        mat2 = vtkbone.vtkboneLinearIsotropicMaterial()
        mat2.SetYoungsModulus(678.9)
        mat2.SetPoissonsRatio(0.5)
        material.SetItem(1, mat2)
        E2_vtk = material.GetYoungsModulus()
        E2 = vtk_to_numpy(E2_vtk)
        self.assertEqual (len(E2), 2)
        self.assertAlmostEqual (E2[0], 1234.5, delta=1E-4)
        self.assertAlmostEqual (E2[1], 678.9, delta=1E-4)
        nu2_vtk = material.GetPoissonsRatio()
        nu2 = vtk_to_numpy(nu2_vtk)
        self.assertEqual (len(nu2), 2)
        self.assertAlmostEqual (nu2[0], 0.4, delta=1E-6)
        self.assertAlmostEqual (nu2[1], 0.5, delta=1E-6)


    def test_set_scaled_item (self):
        material = vtkbone.vtkboneLinearIsotropicMaterialArray()
        material.Resize(2)
        mat1 = vtkbone.vtkboneLinearIsotropicMaterial()
        mat1.SetYoungsModulus(1234.5)
        mat1.SetPoissonsRatio(0.4)
        material.SetScaledItem(0, mat1, 1.5)
        mat2 = vtkbone.vtkboneLinearIsotropicMaterial()
        mat2.SetYoungsModulus(678.9)
        mat2.SetPoissonsRatio(0.5)
        material.SetScaledItem(1, mat2, 2)
        E2_vtk = material.GetYoungsModulus()
        E2 = vtk_to_numpy(E2_vtk)
        self.assertEqual (len(E2), 2)
        self.assertAlmostEqual (E2[0], 1.5*1234.5, delta=1E-4)
        self.assertAlmostEqual (E2[1], 2*678.9, delta=1E-4)
        nu2_vtk = material.GetPoissonsRatio()
        nu2 = vtk_to_numpy(nu2_vtk)
        self.assertEqual (len(nu2), 2)
        self.assertAlmostEqual (nu2[0], 0.4, delta=1E-6)
        self.assertAlmostEqual (nu2[1], 0.5, delta=1E-6)


if __name__ == '__main__':
    unittest.main()
