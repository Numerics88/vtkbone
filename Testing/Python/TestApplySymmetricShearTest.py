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


class TestApplySymmetricShearTest (unittest.TestCase):

    def test_shear_unconfined (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplySymmetricShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.ConfineSidesVerticallyOff()
        model_generator.ConfineTopAndBottomVerticallyOff()
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("face_x0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,2,2,4,4,6,6,8,8,10,10))
        expected_senses = array((0,1)*6)
        expected_values = array((0,0,0.005,0)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 1
        expected_values += array((0,0.005,0,0.005)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,1,1,4,4,5,5,8,8,9,9))
        expected_values = array((0,0,0,0.005)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 2
        expected_values += array((0.005,0,0.005,0)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_unconfined_strain (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplySymmetricShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.ConfineSidesVerticallyOff()
        model_generator.ConfineTopAndBottomVerticallyOff()
        model_generator.SetShearStrain(0.02)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("face_x0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,2,2,4,4,6,6,8,8,10,10))
        expected_senses = array((0,1)*6)
        expected_values = array((0,0,0.01,0)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 1
        expected_values += array((0,0.01,0,0.01)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,1,1,4,4,5,5,8,8,9,9))
        expected_values = array((0,0,0,0.01)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 2
        expected_values += array((0.01,0,0.01,0)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_unconfined_axis_x (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplySymmetricShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.ConfineSidesVerticallyOff()
        model_generator.ConfineTopAndBottomVerticallyOff()
        model_generator.SetTestAxis(0)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("face_x0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,1,1,4,4,5,5,8,8,9,9))
        expected_senses = array((1,2)*6)
        expected_values = array((0,)*4 + (0.005,0)*2 + (0.01,0)*2)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 2
        expected_values += array((0,0.005)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,1,1,2,2,3,3))
        expected_senses = array((1,2)*4)
        expected_values = array((0,)*4 + (0,0.005)*2)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 8
        expected_values += array((0.01,0)*4)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_unconfined_axis_y (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplySymmetricShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.ConfineSidesVerticallyOff()
        model_generator.ConfineTopAndBottomVerticallyOff()
        model_generator.SetTestAxis(1)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 4)

        constraint = model.GetConstraints().GetItem("face_x0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,1,1,2,2,3,3))
        expected_senses = array((0,2)*4)
        expected_values = array((0,0,0,0.005)*2)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 8
        expected_values += array((0.01,0)*4)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,2,2,4,4,6,6,8,8,10,10))
        expected_senses = array((0,2)*6)
        expected_values = array((0,)*4 + (0.005,0)*2 + (0.01,0)*2)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 1
        expected_values += array((0,0.005)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_confine_sides (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplySymmetricShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.ConfineTopAndBottomVerticallyOff()
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 8)

        constraint = model.GetConstraints().GetItem("face_x0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,2,2,4,4,6,6,8,8,10,10))
        expected_senses = array((0,1)*6)
        expected_values = array((0,0,0.005,0)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 1
        expected_values += array((0,0.005,0,0.005)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,1,1,4,4,5,5,8,8,9,9))
        expected_values = array((0,0,0,0.005)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 2
        expected_values += array((0.005,0,0.005,0)*3)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x0_vertical")
        self.assertFalse(constraint is None)
        expected_indices = array((0,2,4,6,8,10))
        expected_senses = array((2,)*6)
        expected_values = array((0,)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_vertical")
        self.assertFalse(constraint is None)
        expected_indices += 1 
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_vertical")
        self.assertFalse(constraint is None)
        expected_indices = array((0,1,4,5,8,9))
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_vertical")
        self.assertFalse(constraint is None)
        expected_indices += 2
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_confine_sides_axis_x (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplySymmetricShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.ConfineTopAndBottomVerticallyOff()
        model_generator.SetTestAxis(0)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 8)

        constraint = model.GetConstraints().GetItem("face_x0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,1,1,4,4,5,5,8,8,9,9))
        expected_senses = array((1,2)*6)
        expected_values = array((0,)*4 + (0.005,0)*2 + (0.01,0)*2)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 2
        expected_values += array((0,0.005)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,1,1,2,2,3,3))
        expected_senses = array((1,2)*4)
        expected_values = array((0,)*4 + (0,0.005)*2)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 8
        expected_values += array((0.01,0)*4)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x0_vertical")
        self.assertFalse(constraint is None)
        expected_indices = array((0,1,4,5,8,9))
        expected_senses = array((0,)*6)
        expected_values = array((0,)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_vertical")
        self.assertFalse(constraint is None)
        expected_indices += 2
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_vertical")
        self.assertFalse(constraint is None)
        expected_indices = array((0,1,2,3))
        expected_senses = array((0,)*4)
        expected_values = array((0,)*4)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_vertical")
        self.assertFalse(constraint is None)
        expected_indices += 8
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_shear_confine_sides_axis_y (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplySymmetricShearTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.ConfineTopAndBottomVerticallyOff()
        model_generator.SetTestAxis(1)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 8)

        constraint = model.GetConstraints().GetItem("face_x0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,1,1,2,2,3,3))
        expected_senses = array((0,2)*4)
        expected_values = array((0,0,0,0.005)*2)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 8
        expected_values += array((0.01,0)*4)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_lateral")
        self.assertFalse(constraint is None)
        expected_indices = array((0,0,2,2,4,4,6,6,8,8,10,10))
        expected_senses = array((0,2)*6)
        expected_values = array((0,)*4 + (0.005,0)*2 + (0.01,0)*2)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_lateral")
        self.assertFalse(constraint is None)
        expected_indices += 1
        expected_values += array((0,0.005)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x0_vertical")
        self.assertFalse(constraint is None)
        expected_indices = array((0,1,2,3))
        expected_senses = array((1,)*4)
        expected_values = array((0,)*4)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_vertical")
        self.assertFalse(constraint is None)
        expected_indices += 8
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_vertical")
        self.assertFalse(constraint is None)
        expected_indices = array((0,2,4,6,8,10))
        expected_senses = array((1,)*6)
        expected_values = array((0,)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_vertical")
        self.assertFalse(constraint is None)
        expected_indices += 1
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


if __name__ == '__main__':
    unittest.main()
