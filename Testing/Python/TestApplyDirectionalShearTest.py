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
from apply_test_utilities import compare_constraint
import unittest


material_generator = vtkbone.vtkboneGenerateHomogeneousMaterialTable()
material_generator.Update()
materials = material_generator.GetOutput()
assert(materials != None)

class TestApplyDirectionalShearTest (unittest.TestCase):
    
    def test_shear_default (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyDirectionalShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0,1,2,3)*3)
        expected_senses = array((0,)*4 + (1,)*4 + (2,)*4)
        expected_values = zeros(12, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((8,9,10,11))
        expected_senses = array((2,)*4)
        expected_values = zeros(4, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8,9,10,11)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = array((0.01,)*4 + (0.0,)*4)
        expected_values *= 2
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_dont_scale (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyDirectionalShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.ScaleShearToHeightOff()
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0,1,2,3)*3)
        expected_senses = array((0,)*4 + (1,)*4 + (2,)*4)
        expected_values = zeros(12, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((8,9,10,11))
        expected_senses = array((2,)*4)
        expected_values = zeros(4, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8,9,10,11)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = array((0.01,)*4 + (0.0,)*4)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_diagonal (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyDirectionalShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetShearVector(0.005, 0.005)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0,1,2,3)*3)
        expected_senses = array((0,)*4 + (1,)*4 + (2,)*4)
        expected_values = zeros(12, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2,)*4)
        expected_values = zeros(4, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = array((0.005,)*4 + (0.005,)*4)
        expected_values *= 2
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_axis_x (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyDirectionalShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(0)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0,2,4,6,8,10)*3)
        expected_senses = array((0,)*6 + (1,)*6 + (2,)*6)
        expected_values = zeros(18, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((1,3,5,7,9,11))
        expected_senses = array((0,)*6)
        expected_values = zeros(6, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((1,3,5,7,9,11)*2)
        expected_senses = array((1,)*6 + (2,)*6)
        expected_values = array((0.01,)*6 + (0.0,)*6)
        expected_values *= 1   # the height
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_axis_x_diagonal (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyDirectionalShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(0)
        model_generator.SetShearVector(0.005, 0.005)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0,2,4,6,8,10)*3)
        expected_senses = array((0,)*6 + (1,)*6 + (2,)*6)
        expected_values = zeros(18, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((1,3,5,7,9,11))
        expected_senses = array((0,)*6)
        expected_values = zeros(6, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((1,3,5,7,9,11)*2)
        expected_senses = array((1,)*6 + (2,)*6)
        expected_values = array((0.005,)*6 + (0.005,)*6)
        expected_values *= 1   # the height
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_axis_y (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyDirectionalShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(1)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0,1,4,5,8,9)*3)
        expected_senses = array((0,)*6 + (1,)*6 + (2,)*6)
        expected_values = zeros(18, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((2,3,6,7,10,11))
        expected_senses = array((1,)*6)
        expected_values = zeros(6, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((2,3,6,7,10,11)*2)
        expected_senses = array((2,)*6 + (0,)*6)
        expected_values = array((0.01,)*6 + (0.0,)*6)
        expected_values *= 1   # the height
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


if __name__ == '__main__':
    unittest.main()
