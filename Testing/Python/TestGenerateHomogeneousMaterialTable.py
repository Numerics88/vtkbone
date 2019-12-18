from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import unittest


class TestGenerateHomogeneousMaterialTable (unittest.TestCase):

    def test_default_table(self):
        generator = vtkbone.vtkboneGenerateHomogeneousMaterialTable()
        generator.Update()
        material_table = generator.GetOutput()
        self.assertFalse(material_table is None)
        self.assertEqual(material_table.GetNumberOfMaterials(), 127)
        index = 1
        while (index <= 127):
            material = material_table.GetMaterial(index)
            self.assertFalse(material is None)
            self.assertEqual(material.GetClassName(), "vtkboneLinearIsotropicMaterial")
            self.assertEqual(material.GetYoungsModulus(), 6829.0)
            self.assertEqual(material.GetPoissonsRatio(), 0.3)
            index += 1


    def test_index_range(self):
        material = vtkbone.vtkboneLinearIsotropicMaterial()
        material.SetYoungsModulus(10000.0)
        material.SetPoissonsRatio(0.4)
        generator = vtkbone.vtkboneGenerateHomogeneousMaterialTable()
        generator.SetMaterial(material)
        generator.SetFirstIndex(101)
        generator.SetLastIndex(110)
        self.assertEqual(generator.GetFirstIndex(), 101)
        self.assertEqual(generator.GetLastIndex(), 110)
        generator.Update()
        material_table = generator.GetOutput()
        self.assertFalse(material_table is None)

        self.assertEqual(material_table.GetNumberOfMaterials(), 10)
        index = 101
        while (index <= 110):
            material = material_table.GetMaterial(index)
            self.assertFalse(material is None)
            self.assertEqual(material.GetClassName(), "vtkboneLinearIsotropicMaterial")
            self.assertEqual(material.GetYoungsModulus(), 10000.0)
            self.assertEqual(material.GetPoissonsRatio(), 0.4)
            index += 1


    def test_material_id_list(self):
        material = vtkbone.vtkboneLinearIsotropicMaterial()
        material.SetYoungsModulus(10000.0)
        material.SetPoissonsRatio(0.4)
        generator = vtkbone.vtkboneGenerateHomogeneousMaterialTable()
        generator.SetMaterial(material)
        indices = array((8,4,10))
        indices_vtk = numpy_to_vtk(indices, deep=1, array_type = vtk.VTK_ID_TYPE)
        generator.SetMaterialIdList(indices_vtk)
        generator.Update()
        material_table = generator.GetOutput()
        self.assertFalse(material_table is None)
        self.assertEqual(material_table.GetNumberOfMaterials(), 3)
        for index in indices:
            print(index)
            material = material_table.GetMaterial(index)
            self.assertFalse(material is None)
            self.assertEqual(material.GetClassName(), "vtkboneLinearIsotropicMaterial")
            self.assertEqual(material.GetYoungsModulus(), 10000.0)
            self.assertEqual(material.GetPoissonsRatio(), 0.4)


if __name__ == '__main__':
    unittest.main()
