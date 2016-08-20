from __future__ import division
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import unittest


# --------------------------------------------------------------------------
# Invent some random materials

material2 = vtkbone.vtkboneLinearIsotropicMaterial()
material2.SetYoungsModulus(1000.0)
material2.SetPoissonsRatio(0.2)
material2.SetName("Material2")

material3 = vtkbone.vtkboneLinearIsotropicMaterial()
material3.SetYoungsModulus(1200.0)
material3.SetPoissonsRatio(0.3)
material3.SetName("Material3")

material8 = vtkbone.vtkboneLinearIsotropicMaterial()
material8.SetYoungsModulus(1400.0)
material8.SetPoissonsRatio(0.4)
material8.SetName("Material8")

material42 = vtkbone.vtkboneLinearIsotropicMaterial()
material42.SetYoungsModulus(1442.0)
material42.SetPoissonsRatio(0.42)
material42.SetName("Material42")


class TestMaterialTable (unittest.TestCase):

    # Obviously, should break this into individual tests.
    def test_everything (self):
		# --------------------------------------------------------------------------
		# Create Material Table and add Materials

		material_table = vtkbone.vtkboneMaterialTable()
		self.assertEqual (material_table.GetNumberOfMaterials(), 0)

		# Note: Not adding Materials in order.
		material_table.AddMaterial(3, material3)
		material_table.AddMaterial(2, material2)
		material_table.AddMaterial(8, material8)
		self.assertEqual (material_table.GetNumberOfMaterials(), 3)
		self.assertEqual (material_table.GetMaximumIndex(), 8)

		# --------------------------------------------------------------------------
		# Try reading out the Materials by Index

		material = material_table.GetMaterial(2)
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1000.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.2)
		self.assertEqual (material.GetName(), "Material2")

		material = material_table.GetMaterial(8)
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1400.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.4)
		self.assertEqual (material.GetName(), "Material8")

		material = material_table.GetMaterial(3)
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1200.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.3)
		self.assertEqual (material.GetName(), "Material3")

		# --------------------------------------------------------------------------
		# Try reading out the Materials by Name

		material = material_table.GetMaterial("Material2")
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1000.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.2)

		material = material_table.GetMaterial("Material8")
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1400.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.4)

		material = material_table.GetMaterial("Material3")
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1200.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.3)

		# --------------------------------------------------------------------------
		# Now try reading out a non-existant material

		material = material_table.GetMaterial(1)
		self.assertTrue(material is None)

		material = material_table.GetMaterial("NonexistantMaterial")
		self.assertTrue(material is None)

		# --------------------------------------------------------------------------
		# Test Traversal

		material_table.InitTraversal()

		index = material_table.GetNextIndex()
		self.assertEqual (index, 2)
		material = material_table.GetCurrentMaterial()
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1000.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.2)

		index = material_table.GetNextIndex()
		self.assertEqual (index, 3)
		material = material_table.GetCurrentMaterial()
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1200.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.3)

		index = material_table.GetNextIndex()
		self.assertEqual (index, 8)
		material = material_table.GetCurrentMaterial()
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1400.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.4)

		index = material_table.GetNextIndex()
		self.assertEqual (index, 0)
		material = material_table.GetCurrentMaterial()
		self.assertTrue(material is None)

		# --------------------------------------------------------------------------
		# Test Removal of a Material

		material_table.RemoveMaterial(3)
		self.assertEqual (material_table.GetNumberOfMaterials(), 2)
		self.assertEqual (material_table.GetMaximumIndex(), 8)

		material = material_table.GetMaterial(2)
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1000.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.2)

		material = material_table.GetMaterial(8)
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1400.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.4)

		material = material_table.GetMaterial(3)
		self.assertTrue(material is None)

		material_table.RemoveMaterial(8)
		self.assertEqual (material_table.GetNumberOfMaterials(), 1)
		self.assertEqual (material_table.GetMaximumIndex(), 2)

		# --------------------------------------------------------------------------
		# Test replacing one material with another

		material_table.AddMaterial(2, material8)
		self.assertEqual (material_table.GetNumberOfMaterials(), 1)
		self.assertEqual (material_table.GetMaximumIndex(), 2)

		material = material_table.GetMaterial(2)
		self.assertFalse(material is None)
		self.assertEqual (material.GetYoungsModulus(), 1400.0)
		self.assertEqual (material.GetPoissonsRatio(), 0.4)


		# --------------------------------------------------------------------------
		# Test appending a material
		material_table.AppendMaterial (material42);
		self.assertEqual (material_table.GetNumberOfMaterials(), 2)
		material = material_table.GetMaterial(3)
		self.assertFalse(material is None)
		self.assertEqual (material.GetName(), "Material42")


		# --------------------------------------------------------------------------
		# Test Clearing the Material Table

		material_table.RemoveAll()
		self.assertEqual (material_table.GetNumberOfMaterials(), 0)

		# --------------------------------------------------------------------------
		# Test appending a material to empty table
		material_table.AppendMaterial (material42);
		self.assertEqual (material_table.GetNumberOfMaterials(), 1)
		material = material_table.GetMaterial(1)
		self.assertFalse(material is None)
		self.assertEqual (material.GetName(), "Material42")


		# --------------------------------------------------------------------------
		# Test CheckNames

		material_table.RemoveAll()
		material_table.AddMaterial(2, material2)
		material_table.AddMaterial(3, material3)
		material_table.AddMaterial(8, material8)
		self.assertEqual (material_table.CheckNames(), 1)
		# OK to add the same materials again
		material_table.AddMaterial(10, material2)
		material_table.AddMaterial(11, material2)
		material_table.AddMaterial(12, material2)
		self.assertEqual (material_table.CheckNames(), 1)
		# Not OK to have different materials with the same name.
		material8.SetName("Material2")
		self.assertEqual (material_table.CheckNames(), 0)

		# --------------------------------------------------------------------------
		# Test Unique Traversal

		material_table.RemoveAll()
		material_table.AddMaterial(3, material3)
		material_table.AddMaterial(6, material2)
		material_table.AddMaterial(8, material8)
		material_table.AddMaterial(7, material2)
		material_table.AddMaterial(5, material3)
		material_table.AddMaterial(100, material8)

		material_table.InitTraversal()

		index = material_table.GetNextUniqueIndex()
		self.assertEqual (index, 3)
		index = material_table.GetNextUniqueIndex()
		self.assertEqual (index, 6)
		index = material_table.GetNextUniqueIndex()
		self.assertEqual (index, 8)
		index = material_table.GetNextUniqueIndex()
		self.assertEqual (index, 0)

		# --------------------------------------------------------------------------
		# Test Material Array

		material_array = vtkbone.vtkboneLinearIsotropicMaterialArray()
		E = vtk.vtkFloatArray()
		E.SetNumberOfTuples(3)
		E.SetComponent (0,0,1234.5)
		E.SetComponent (1,0,678.9)
		E.SetComponent (2,0,700)
		nu = vtk.vtkFloatArray()
		nu.SetNumberOfTuples(3)
		nu.SetComponent (0,0,0.4)
		nu.SetComponent (1,0,0.5)
		nu.SetComponent (2,0,0.6)
		material_array.SetYoungsModulus(E)
		material_array.SetPoissonsRatio(nu)
		material_array.SetName ("Fred")

		material_table.AddMaterial(11, material_array)

		fetched_material = material_table.GetMaterialOrArray (12)
		offset = material_table.GetArrayOffset()
		self.assertEqual (offset, 1)
		self.assertTrue (fetched_material is material_array)

		fetched_material = material_table.GetMaterialOrArray (11)
		offset = material_table.GetArrayOffset()
		self.assertEqual (offset, 0)
		self.assertTrue (fetched_material is material_array)

		fetched_material = material_table.GetMaterialOrArray (5)
		offset = material_table.GetArrayOffset()
		self.assertEqual (offset, 0)
		self.assertTrue (fetched_material is material3)


if __name__ == '__main__':
    unittest.main()
