from __future__ import division
import sys
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

class TestApplyCompressionTest (unittest.TestCase):
    
    def test_uniaxial (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyCompressionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetAppliedStrain(-0.02)
        model_generator.PinOff()
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 2)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3))
        expected_senses = array((2, 2, 2, 2))
        expected_values = array((0.0, 0.0, 0.0, 0.0))
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2, 2, 2, 2))
        expected_values = 2*(-0.02)*ones(4, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))
        

    def test_uniaxial_with_pin_cell0 (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyCompressionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetAppliedStrain(-0.02)
        model_generator.PinOn()
        model_generator.PinCellClosestToXYCenterOff()
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3))
        expected_senses = array((2, 2, 2, 2))
        expected_values = array((0.0, 0.0, 0.0, 0.0))
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2, 2, 2, 2))
        expected_values = 2*(-0.02)*ones(4, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        # This test is too strict, since there would be many possible valid pins.
        constraint = model.GetConstraints().GetItem("pinned_nodes")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 0, 1))
        expected_senses = array((0, 1, 1))
        expected_values = array((0.0, 0.0, 0.0))
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_uniaxial_absolute_displacement (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyCompressionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetAppliedDisplacement(-0.02)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 2)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3))
        expected_senses = array((2, 2, 2, 2))
        expected_values = array((0.0, 0.0, 0.0, 0.0))
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2, 2, 2, 2))
        expected_values = (-0.02)*ones(4, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_uniaxial_quasi_donut (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_quasi_donut_geometry()
        # The following conditional is useful for writing out the model.
        if 0:
            writer = vtk.vtkXMLUnstructuredGridWriter()
            writer.SetInputData(geometry)
            writer.SetFileName("geometry.vtu")
            writer.Update()
        model_generator = vtkbone.vtkboneApplyCompressionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetAppliedStrain(-0.02)
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 2)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array(( 0,  1,  2,  3,  4,  5,
                                   6,  7,  8,  9, 10, 11,
                                  16, 17,         18, 19,
                                  12, 13,         14, 15,
                                  20, 21, 22, 23, 24, 25,
                                  26, 27, 28, 29, 30, 31))
        expected_senses = 2*ones(expected_indices.shape[0], int)
        expected_values = zeros(expected_indices.shape[0], float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((104, 105, 106, 107, 108, 109,
                                  110, 111, 112, 113, 114, 115,
                                  116, 117,           118, 119,
                                  120, 121,           122, 123,
                                  124, 125, 126, 127, 128, 129,
                                  130, 131, 132, 133, 134, 135))
        expected_senses = 2 * ones(expected_indices.shape[0], int)
        expected_values = 3*(-0.02)*ones(expected_indices.shape[0], float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_uniaxial_quasi_donut_with_center_pin (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_quasi_donut_geometry_two_materials_offset()
        # The following conditional is useful for writing out the model.
        if 0:
            writer = vtk.vtkXMLUnstructuredGridWriter()
            writer.SetInputData(geometry)
            writer.SetFileName("geometry.vtu")
            writer.Update()
        model_generator = vtkbone.vtkboneApplyCompressionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetAppliedStrain(-0.02)
        model_generator.PinOn()
        model_generator.PinCellClosestToXYCenterOn()
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array(( 0,  1,  2,  3,  4,  5,
                                   6,  7,  8,  9, 10, 11,
                                  16, 17,         18, 19,
                                  12, 13,         14, 15,
                                  20, 21, 22, 23, 24, 25,
                                  26, 27, 28, 29, 30, 31))
        expected_senses = 2*ones(expected_indices.shape[0], int)
        expected_values = zeros(expected_indices.shape[0], float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((104, 105, 106, 107, 108, 109,
                                  110, 111, 112, 113, 114, 115,
                                  116, 117,           118, 119,
                                  120, 121,           122, 123,
                                  124, 125, 126, 127, 128, 129,
                                  130, 131, 132, 133, 134, 135))
        expected_senses = 2 * ones(expected_indices.shape[0], int)
        expected_values = 2*3*(-0.02)*ones(expected_indices.shape[0], float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("pinned_nodes")
        self.assertFalse(constraint is None)
        expected_indices = array((46, 46, 47))
        expected_senses = array((0,1,1))
        expected_values = zeros(3, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_uniaxial_quasi_donut_with_center_pin_axis_x (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_quasi_donut_geometry_two_materials_offset()
        # The following conditional is useful for writing out the model.
        if 0:
            writer = vtk.vtkXMLUnstructuredGridWriter()
            writer.SetInputData(geometry)
            writer.SetFileName("geometry.vtu")
            writer.Update()
        model_generator = vtkbone.vtkboneApplyCompressionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(0)
        model_generator.SetAppliedStrain(-0.02)
        model_generator.PinOn()
        model_generator.PinCellClosestToXYCenterOn()
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((26, 62, 98, 130,
                                  20, 56, 92, 124, 
                                  16, 50, 86, 120, 
                                  12, 44, 80, 116, 
                                   6, 38, 74, 110, 
                                   0, 32, 68, 104))
        expected_senses = zeros(expected_indices.shape[0], int)
        expected_values = zeros(expected_indices.shape[0], float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((31, 67, 103, 135,
                                  25, 61,  97, 129,
                                  19, 55,  91, 123,
                                  15, 49,  85, 119,
                                  11, 43,  79, 115,
                                   5, 37,  73, 109))
        expected_senses = zeros(expected_indices.shape[0], int)
        expected_values = 0.5*5*(-0.02)*ones(expected_indices.shape[0], float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("pinned_nodes")
        self.assertFalse(constraint is None)
        expected_indices = array((44, 44, 50))
        expected_senses = array((1,2,2))
        expected_values = zeros(3, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_axial (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyCompressionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetAppliedStrain(-0.02)
        model_generator.TopSurfaceContactFrictionOn();
        model_generator.BottomSurfaceContactFrictionOn();
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 3)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*3)
        expected_senses = array((0,)*4 + (1,)*4 + (2,)*4)
        expected_values = array((0.0,)*12)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = array((0.0,)*8)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2, 2, 2, 2))
        expected_values = 2*(-0.02)*ones(4, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


    def test_confined (self):

        # --------------
        # Generate model

        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyCompressionTest()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetAppliedStrain(-0.02)
        model_generator.TopSurfaceContactFrictionOn();
        model_generator.BottomSurfaceContactFrictionOn();
        model_generator.ConfineSidesOn();
        model_generator.Update()
        model = model_generator.GetOutput()

        # -----------------
        # Check constraints

        self.assertEqual(model.GetConstraints().GetNumberOfItems(), 7)

        constraint = model.GetConstraints().GetItem("bottom_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 2, 3)*3)
        expected_senses = array((0,)*4 + (1,)*4 + (2,)*4)
        expected_values = array((0.0,)*12)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11)*2)
        expected_senses = array((0,)*4 + (1,)*4)
        expected_values = array((0.0,)*8)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("top_displacement")
        self.assertFalse(constraint is None)
        expected_indices = array((8, 9, 10, 11))
        expected_senses = array((2, 2, 2, 2))
        expected_values = 2*(-0.02)*ones(4, float)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x0_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 2, 4, 6, 8, 10))
        expected_senses = array((0,)*6)
        expected_values = array((0.0,)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_x1_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((1, 3, 5, 7, 9, 11))
        expected_senses = array((0,)*6)
        expected_values = array((0.0,)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y0_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((0, 1, 4, 5, 8, 9))
        expected_senses = array((1,)*6)
        expected_values = array((0.0,)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))

        constraint = model.GetConstraints().GetItem("face_y1_fixed")
        self.assertFalse(constraint is None)
        expected_indices = array((2, 3, 6, 7, 10, 11))
        expected_senses = array((1,)*6)
        expected_values = array((0.0,)*6)
        self.assertTrue(compare_constraint(constraint, expected_indices, expected_senses, expected_values))


if __name__ == '__main__':
    unittest.main()
