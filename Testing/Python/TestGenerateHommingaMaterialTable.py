from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import unittest


class TestGenerateHommingaMaterialTable (unittest.TestCase):

    def test_default(self):
        material = vtkbone.vtkboneLinearIsotropicMaterial()
        generator = vtkbone.vtkboneGenerateHommingaMaterialTable()
        generator.SetFullScaleMaterial(material)
        self.assertEqual(generator.GetExponent(), 1.7)
        generator.Update()
        material_table = generator.GetOutput()
        self.assertFalse(material_table is None)
        self.assertEqual(material_table.GetNumberOfMaterials(), 1)
        material_array = material_table.GetMaterial(1)
        self.assertFalse(material_array is None)
        self.assertEqual(material_array.GetClassName(), "vtkboneLinearIsotropicMaterialArray")
        E_vtk = material_array.GetYoungsModulus()
        self.assertFalse(E_vtk is None)
        self.assertEqual(E_vtk.GetNumberOfTuples(), 127)
        E = vtk_to_numpy (E_vtk)
        E_expected = 6829.0*((arange(1,128)/127)**1.7)
        self.assertTrue(alltrue(abs(E - E_expected) < 1E-3))
        nu_vtk = material_array.GetPoissonsRatio()
        self.assertFalse(nu_vtk is None)
        self.assertEqual(nu_vtk.GetNumberOfTuples(), 127)
        nu = vtk_to_numpy (nu_vtk)
        self.assertTrue(alltrue(abs(nu - 0.3) < 1E-6))


    def test_custom(self):
        material = vtkbone.vtkboneLinearIsotropicMaterial()
        material.SetYoungsModulus(10000.0)
        material.SetPoissonsRatio(0.4)
        generator = vtkbone.vtkboneGenerateHommingaMaterialTable()
        generator.SetFullScaleMaterial(material)
        generator.SetExponent(1.5)
        generator.SetFirstIndex(101)
        generator.SetLastIndex(110)
        self.assertEqual(generator.GetExponent(), 1.5)
        self.assertEqual(generator.GetFirstIndex(), 101)
        self.assertEqual(generator.GetLastIndex(), 110)
        generator.Update()
        material_table = generator.GetOutput()
        self.assertFalse(material_table is None)
        self.assertEqual(material_table.GetNumberOfMaterials(), 1)
        material_array = material_table.GetMaterial(101)
        self.assertFalse(material_array is None)
        self.assertEqual(material_array.GetClassName(), "vtkboneLinearIsotropicMaterialArray")
        E_vtk = material_array.GetYoungsModulus()
        self.assertFalse(E_vtk is None)
        self.assertEqual(E_vtk.GetNumberOfTuples(), 10)
        E = vtk_to_numpy (E_vtk)
        E_expected = 10000.0*((arange(1,11)/10)**1.5)
        self.assertTrue(alltrue(abs(E - E_expected) < 1E-3))
        nu_vtk = material_array.GetPoissonsRatio()
        self.assertFalse(nu_vtk is None)
        self.assertEqual(nu_vtk.GetNumberOfTuples(), 10)
        nu = vtk_to_numpy (nu_vtk)
        self.assertTrue(alltrue(abs(nu - 0.4) < 1E-6))


if __name__ == '__main__':
    unittest.main()
