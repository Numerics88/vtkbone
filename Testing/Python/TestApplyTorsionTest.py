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


class TestApplyTorsionTest (unittest.TestCase):

    def test_torsion_default (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyTorsionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTwistAxisOrigin(0.5, 0.5)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*3)
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
        expected_indices = array((8,8,9,9,10,10,11,11))
        expected_senses = array((0,1)*4 )
        dx = 0.5 - sqrt(0.5)*sin(44*pi/180)
        dy = 0.5 - sqrt(0.5)*cos(44*pi/180)
        expected_values = array((dx,dy, -dy,dx, dy,-dx, -dx,-dy))
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_torsion_large_rotation (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyTorsionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTwistAxisOrigin(0.5, 0.5)
        model_generator.SetTwistAngle(10*pi/180)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*3)
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
        expected_indices = array((8,8,9,9,10,10,11,11))
        expected_senses = array((0,1)*4 )
        dx = 0.5 - sqrt(0.5)*sin(35*pi/180)
        dy = 0.5 - sqrt(0.5)*cos(35*pi/180)
        expected_values = array((dx,dy, -dy,dx, dy,-dx, -dx,-dy))
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_torsion_origin (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyTorsionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTwistAxisOrigin(1, 0)
        model_generator.SetTwistAngle(10*pi/180)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*3)
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
        expected_indices = array((8,8,9,9,10,10,11,11))
        expected_senses = array((0,1)*4 )
        dx = 1 - cos(10*pi/180)
        dy = -sin(10*pi/180)
        expected_values = array((dx,dy, 0,0, 0,0, dy,-dx))
        dx = 1 - sqrt(2)*cos(35*pi/180)
        dy = -1 + sqrt(2)*sin(35*pi/180) 
        expected_values[4:6] = array((dx,dy))
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_torsion_test_axis_x (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyTorsionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(0)
        model_generator.SetTwistAxisOrigin(0.0, 1.0)
        model_generator.SetTwistAngle(10*pi/180)
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
        expected_indices = array((1,1,3,3,5,5,7,7,9,9,11,11))
        expected_senses = array((1,2)*6)
        expected_values = zeros(12, float)
        expected_values[0] = sin(10*pi/180)
        expected_values[1] = 1 - cos(10*pi/180)
        expected_values[2] = -1 + sqrt(2)*cos(35*pi/180)
        expected_values[3] = 1 - sqrt(2)*sin(35*pi/180)
        expected_values[6] = -1 + cos(10*pi/180)
        expected_values[7] = sin(10*pi/180)
        expected_values[8] = -sin(10*pi/180)
        expected_values[9] = -1 + cos(10*pi/180)
        expected_values[10] = -1 + sqrt(2)*cos(55*pi/180)
        expected_values[11] = -1 + sqrt(2)*sin(55*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_torsion_default_nonsquare (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry_nonsquare()
        model_generator = vtkbone.vtkboneApplyTorsionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTwistAxisOrigin(-0.6, 1.25)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*3)
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
        expected_indices = array((8,8,9,9,10,10,11,11))
        expected_senses = array((0,1)*4 )
        expected_values = zeros(8,float)
        theta0 = atan(1.5/0.8)
        s0 = sqrt((0.8/2)**2 + (1.5/2)**2)
        expected_values[0] = 0.4 - s0*cos(theta0 + pi/180)
        expected_values[1] = 0.75 - s0*sin(theta0 + pi/180)
        expected_values[2] = -0.4 + s0*cos(theta0 - pi/180)
        expected_values[3] = 0.75 - s0*sin(theta0 - pi/180)
        expected_values[4] = -expected_values[2]
        expected_values[5] = -expected_values[3]
        expected_values[6] = -expected_values[0]
        expected_values[7] = -expected_values[1]
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_torsion_large_rotation_nonsquare (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry_nonsquare()
        model_generator = vtkbone.vtkboneApplyTorsionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTwistAxisOrigin(-0.6, 1.25)
        model_generator.SetTwistAngle(10*pi/180)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*3)
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
        expected_indices = array((8,8,9,9,10,10,11,11))
        expected_senses = array((0,1)*4 )
        expected_values = zeros(8,float)
        theta0 = atan(1.5/0.8)
        s0 = sqrt((0.8/2)**2 + (1.5/2)**2)
        expected_values[0] = 0.4 - s0*cos(theta0 + 10*pi/180)
        expected_values[1] = 0.75 - s0*sin(theta0 + 10*pi/180)
        expected_values[2] = -0.4 + s0*cos(theta0 - 10*pi/180)
        expected_values[3] = 0.75 - s0*sin(theta0 - 10*pi/180)
        expected_values[4] = -expected_values[2]
        expected_values[5] = -expected_values[3]
        expected_values[6] = -expected_values[0]
        expected_values[7] = -expected_values[1]
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_torsion_origin_nonsquare (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry_nonsquare()
        model_generator = vtkbone.vtkboneApplyTorsionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTwistAxisOrigin(-0.2, 0.5)
        model_generator.SetTwistAngle(10*pi/180)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*3)
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
        expected_indices = array((8,8,9,9,10,10,11,11))
        expected_senses = array((0,1)*4 )
        expected_values = zeros(8,float)
        expected_values[0] = 0.8 - 0.8*cos(10*pi/180)
        expected_values[1] = -0.8*sin(10*pi/180)
        theta0 = atan(1.5/0.8)
        s0 = sqrt(0.8**2 + 1.5**2)
        expected_values[4] = 0.8 - s0*cos(theta0 - 10*pi/180)
        expected_values[5] = -1.5 + s0*sin(theta0 - 10*pi/180)
        expected_values[6] = -1.5*sin(10*pi/180)
        expected_values[7] = -1.5 + 1.5*cos(10*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_torsion_test_axis_x_nonsquare (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry_nonsquare()
        model_generator = vtkbone.vtkboneApplyTorsionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(0)
        model_generator.SetTwistAxisOrigin(0.5, 2.5)
        model_generator.SetTwistAngle(10*pi/180)
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
        expected_indices = array((1,1,3,3,5,5,7,7,9,9,11,11))
        expected_senses = array((1,2)*6)
        expected_values = zeros(12, float)
        expected_values[0] = 0.5*sin(10*pi/180)
        expected_values[1] = 0.5 - 0.5*cos(10*pi/180)
        theta0 = atan(0.5/1.5)
        s0 = sqrt(0.5**2 + 1.5**2)
        expected_values[2] = -1.5 + s0*cos(theta0 - 10*pi/180)
        expected_values[3] = 0.5 - s0*sin(theta0 - 10*pi/180)
        expected_values[6] = -1.5 + 1.5*cos(10*pi/180)
        expected_values[7] = 1.5*sin(10*pi/180)
        expected_values[8] = -expected_values[0]
        expected_values[9] = -expected_values[1]
        expected_values[10] = -1.5 + s0*cos(theta0 + 10*pi/180)
        expected_values[11] = -0.5 + s0*sin(theta0 + 10*pi/180)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


if __name__ == '__main__':
    unittest.main()
