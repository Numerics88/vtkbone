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


class TestApplyBendingTest(unittest.TestCase):
    
    def test_bending_default (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyBendingTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetNeutralAxisAngle(0)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = zeros(8, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = zeros(8, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("bottom_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3))
        expected_senses = array((2,)*4)
        expected_values = array((0.0, 0.0, 1.0, 1.0))
        expected_values *= tan(0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2,)*4)
        expected_values = array((0.0, 0.0, 1.0, 1.0))
        expected_values *= tan(-0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_bending_two_degrees (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyBendingTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetNeutralAxisAngle(0)
        model_generator.SetBendingAngle(2.0*pi/180)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = zeros(8, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = zeros(8, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("bottom_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3))
        expected_senses = array((2,)*4)
        expected_values = array((0.0, 0.0, 1.0, 1.0))
        expected_values *= tan(pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2,)*4)
        expected_values = array((0.0, 0.0, 1.0, 1.0))
        expected_values *= tan(-pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_bending_axis_in_middle (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyBendingTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetNeutralAxisAngle(0)
        model_generator.SetNeutralAxisOrigin(0,0.5)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = zeros(8, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = zeros(8, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("bottom_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3))
        expected_senses = array((2,)*4)
        expected_values = array((-0.5, -0.5, 0.5, 0.5))
        expected_values *= tan(0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2,)*4)
        expected_values = array((-0.5, -0.5, 0.5, 0.5))
        expected_values *= tan(-0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_bending_axis_y (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyBendingTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetNeutralAxisAngle(pi/2)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = zeros(8, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = zeros(8, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("bottom_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3))
        expected_senses = array((2,)*4)
        expected_values = array((0.0, -1.0, 0.0, -1.0))
        expected_values *= tan(0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2,)*4)
        expected_values = array((0.0, -1.0, 0.0, -1.0))
        expected_values *= tan(-0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_bending_axis_diagonal (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyBendingTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetNeutralAxisOrigin(0.5,0.5)
        model_generator.SetNeutralAxisAngle(45*pi/180)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = zeros(8, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = zeros(8, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("bottom_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3))
        expected_senses = array((2,)*4)
        expected_values = array((0, -sqrt(2)/2, sqrt(2)/2, 0.0))
        expected_values *= tan(0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2,)*4)
        expected_values = array((0, -sqrt(2)/2, sqrt(2)/2, 0.0))
        expected_values *= tan(-0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_bending_test_axis_x (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyBendingTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetNeutralAxisAngle(0)
        model_generator.SetTestAxis(0)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 2, 4, 6, 8, 10)*2)
        expected_senses = array((1,)*6 + (2,)*6)
        expected_values = zeros(12, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((1, 3, 5, 7, 9, 11)*2)
        expected_senses = array((1,)*6 + (2,)*6)
        expected_values = zeros(12, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("bottom_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 2, 4, 6, 8, 10))
        expected_senses = array((0,)*6)
        expected_values = array((0.0, 0.0, 1.0, 1.0, 2.0, 2.0))
        expected_values *= tan(0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((1, 3, 5, 7, 9, 11))
        expected_senses = array((0,)*6)
        expected_values = array((0.0, 0.0, 1.0, 1.0, 2.0, 2.0))
        expected_values *= tan(-0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_bending_test_axis_x_bending_axis_z (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyBendingTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(0)
        model_generator.SetNeutralAxisAngle(pi/2)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 2, 4, 6, 8, 10)*2)
        expected_senses = array((1,)*6 + (2,)*6)
        expected_values = zeros(12, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((1, 3, 5, 7, 9, 11)*2)
        expected_senses = array((1,)*6 + (2,)*6)
        expected_values = zeros(12, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        
        constraint = model.GetConstraints().GetItem("bottom_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 2, 4, 6, 8, 10))
        expected_senses = array((0,)*6)
        expected_values = array((0.0, -1.0, 0.0, -1.0, 0.0, -1.0))
        expected_values *= tan(0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((1, 3, 5, 7, 9, 11))
        expected_senses = array((0,)*6)
        expected_values = array((0.0, -1.0, 0.0, -1.0, 0.0, -1.0))
        expected_values *= tan(-0.5*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


if __name__ == '__main__':
    unittest.main()
