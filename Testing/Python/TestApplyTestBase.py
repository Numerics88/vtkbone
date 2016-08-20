from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import test_geometries
import traceback
import unittest


material_generator = vtkbone.vtkboneGenerateHomogeneousMaterialTable()
material_generator.Update()
materials = material_generator.GetOutput()
assert(materials != None)


class TestApplyTestBase (unittest.TestCase):

    def test_rotation_functions (self):
        base = vtkbone.vtkboneApplyTestBase()
        bounds = (10.0, 11.0, 12.0, 13.0, 14.0, 15.0)
        # test default
        self.assertEqual(base.DataFrameSense(0), 0)
        self.assertEqual(base.DataFrameSense(1), 1)
        self.assertEqual(base.DataFrameSense(2), 2)
        self.assertEqual(base.DataFrameSense(-1), 2)
        self.assertEqual(base.DataFrameSense(-2), 1)
        self.assertEqual(base.DataFrameSense(-3), 0)
        self.assertEqual(base.DataFrameSense(3), 0)
        self.assertEqual(base.TestFrameSense(0), 0)
        self.assertEqual(base.TestFrameSense(1), 1)
        self.assertEqual(base.TestFrameSense(2), 2)
        self.assertEqual(base.TestFrameSense(-1), 2)
        self.assertEqual(base.TestFrameSense(-2), 1)
        self.assertEqual(base.TestFrameSense(-3), 0)
        self.assertEqual(base.TestFrameSense(3), 0)
        self.assertEqual(base.DataFramePolarity(0, 0), 0)
        self.assertEqual(base.DataFramePolarity(1, 0), 0)
        self.assertEqual(base.DataFramePolarity(2, 0), 0)
        self.assertEqual(base.DataFramePolarity(0, 1), 1)
        self.assertEqual(base.DataFramePolarity(1, 1), 1)
        self.assertEqual(base.DataFramePolarity(2, 1), 1)
        self.assertEqual(base.TestFrameBound(bounds, 0, 0), 10.0)
        self.assertEqual(base.TestFrameBound(bounds, 0, 1), 11.0)
        self.assertEqual(base.TestFrameBound(bounds, 1, 0), 12.0)
        self.assertEqual(base.TestFrameBound(bounds, 1, 1), 13.0)
        self.assertEqual(base.TestFrameBound(bounds, 2, 0), 14.0)
        self.assertEqual(base.TestFrameBound(bounds, 2, 1), 15.0)
        self.assertEqual(base.GetVoxelLocalId(0), 0)
        self.assertEqual(base.GetVoxelLocalId(1), 1)
        self.assertEqual(base.GetVoxelLocalId(2), 2)
        self.assertEqual(base.GetVoxelLocalId(3), 3)
        self.assertEqual(base.GetVoxelLocalId(4), 4)
        self.assertEqual(base.GetVoxelLocalId(5), 5)
        self.assertEqual(base.GetVoxelLocalId(6), 6)
        self.assertEqual(base.GetVoxelLocalId(7), 7)
        # testing direction Z (should be same as default)
        base.SetTestAxis(2)
        self.assertEqual(base.DataFrameSense(0), 0)
        self.assertEqual(base.DataFrameSense(1), 1)
        self.assertEqual(base.DataFrameSense(2), 2)
        self.assertEqual(base.TestFrameSense(0), 0)
        self.assertEqual(base.TestFrameSense(1), 1)
        self.assertEqual(base.TestFrameSense(2), 2)
        self.assertEqual(base.DataFramePolarity(0, 0), 0)
        self.assertEqual(base.DataFramePolarity(1, 0), 0)
        self.assertEqual(base.DataFramePolarity(2, 0), 0)
        self.assertEqual(base.DataFramePolarity(0, 1), 1)
        self.assertEqual(base.DataFramePolarity(1, 1), 1)
        self.assertEqual(base.DataFramePolarity(2, 1), 1)
        self.assertEqual(base.TestFrameBound(bounds, 0, 0), 10.0)
        self.assertEqual(base.TestFrameBound(bounds, 0, 1), 11.0)
        self.assertEqual(base.TestFrameBound(bounds, 1, 0), 12.0)
        self.assertEqual(base.TestFrameBound(bounds, 1, 1), 13.0)
        self.assertEqual(base.TestFrameBound(bounds, 2, 0), 14.0)
        self.assertEqual(base.TestFrameBound(bounds, 2, 1), 15.0)
        # testing direction X
        base.SetTestAxis(0)
        self.assertEqual(base.DataFrameSense(0), 1)
        self.assertEqual(base.DataFrameSense(1), 2)
        self.assertEqual(base.DataFrameSense(2), 0)
        self.assertEqual(base.TestFrameSense(1), 0)
        self.assertEqual(base.TestFrameSense(2), 1)
        self.assertEqual(base.TestFrameSense(0), 2)
        self.assertEqual(base.DataFramePolarity(0, 0), 0)
        self.assertEqual(base.DataFramePolarity(1, 0), 0)
        self.assertEqual(base.DataFramePolarity(2, 0), 0)
        self.assertEqual(base.DataFramePolarity(0, 1), 1)
        self.assertEqual(base.DataFramePolarity(1, 1), 1)
        self.assertEqual(base.DataFramePolarity(2, 1), 1)
        self.assertEqual(base.TestFrameBound(bounds, 0, 0), 12.0)
        self.assertEqual(base.TestFrameBound(bounds, 0, 1), 13.0)
        self.assertEqual(base.TestFrameBound(bounds, 1, 0), 14.0)
        self.assertEqual(base.TestFrameBound(bounds, 1, 1), 15.0)
        self.assertEqual(base.TestFrameBound(bounds, 2, 0), 10.0)
        self.assertEqual(base.TestFrameBound(bounds, 2, 1), 11.0)
        # Three transformations should bring us back to the identity.
        for i in range(8):
          self.assertEqual(base.GetVoxelLocalId(base.GetVoxelLocalId(base.GetVoxelLocalId(i))), i)
        self.assertEqual(base.GetVoxelLocalId(0), 0)
        self.assertEqual(base.GetVoxelLocalId(1), 2)
        self.assertEqual(base.GetVoxelLocalId(2), 4)
        self.assertEqual(base.GetVoxelLocalId(3), 6)
        self.assertEqual(base.GetVoxelLocalId(4), 1)
        self.assertEqual(base.GetVoxelLocalId(5), 3)
        self.assertEqual(base.GetVoxelLocalId(6), 5)
        self.assertEqual(base.GetVoxelLocalId(7), 7)
        # testing direction Y
        base.SetTestAxis(1)
        self.assertEqual(base.DataFrameSense(0), 2)
        self.assertEqual(base.DataFrameSense(1), 0)
        self.assertEqual(base.DataFrameSense(2), 1)
        self.assertEqual(base.TestFrameSense(2), 0)
        self.assertEqual(base.TestFrameSense(0), 1)
        self.assertEqual(base.TestFrameSense(1), 2)
        self.assertEqual(base.DataFramePolarity(0, 0), 0)
        self.assertEqual(base.DataFramePolarity(1, 0), 0)
        self.assertEqual(base.DataFramePolarity(2, 0), 0)
        self.assertEqual(base.DataFramePolarity(0, 1), 1)
        self.assertEqual(base.DataFramePolarity(1, 1), 1)
        self.assertEqual(base.DataFramePolarity(2, 1), 1)
        self.assertEqual(base.TestFrameBound(bounds, 0, 0), 14.0)
        self.assertEqual(base.TestFrameBound(bounds, 0, 1), 15.0)
        self.assertEqual(base.TestFrameBound(bounds, 1, 0), 10.0)
        self.assertEqual(base.TestFrameBound(bounds, 1, 1), 11.0)
        self.assertEqual(base.TestFrameBound(bounds, 2, 0), 12.0)
        self.assertEqual(base.TestFrameBound(bounds, 2, 1), 13.0)
        # Three transformations should bring us back to the identity.
        for i in range(8):
          self.assertEqual(base.GetVoxelLocalId(base.GetVoxelLocalId(base.GetVoxelLocalId(i))), i)
        self.assertEqual(base.GetVoxelLocalId(0), 0)
        self.assertEqual(base.GetVoxelLocalId(1), 4)
        self.assertEqual(base.GetVoxelLocalId(2), 1)
        self.assertEqual(base.GetVoxelLocalId(3), 5)
        self.assertEqual(base.GetVoxelLocalId(4), 2)
        self.assertEqual(base.GetVoxelLocalId(5), 6)
        self.assertEqual(base.GetVoxelLocalId(6), 3)
        self.assertEqual(base.GetVoxelLocalId(7), 7)


    def test_node_sets_test_axis_z (self):
        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((0, 1, 2, 3))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((8, 9, 10, 11))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_x0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((0, 2, 4, 6, 8, 10))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_x1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((1, 3, 5, 7, 9, 11))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_y0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((0, 1, 4, 5, 8, 9))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_y1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((2, 3, 6, 7, 10, 11))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_node_sets_test_axis_x (self):
        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(0)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((0, 2, 4, 6, 8, 10))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((1, 3, 5, 7, 9, 11))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_x0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((0, 1, 4, 5, 8, 9))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_x1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((2, 3, 6, 7, 10, 11))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_y0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((0, 1, 2, 3))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_y1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((8, 9, 10, 11))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_node_sets_test_axis_y (self):
        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(1)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((0, 1, 4, 5, 8, 9))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((2, 3, 6, 7, 10, 11))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_x0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((0, 1, 2, 3))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_x1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((8, 9, 10, 11))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_y0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((0, 2, 4, 6, 8, 10))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_y1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((1, 3, 5, 7, 9, 11))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_top_uneven (self):
        geometry = test_geometries.generate_quasi_donut_geometry_two_materials()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.UnevenTopSurfaceOn()
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  2,  3,  4,  5,
                                   6,  7,  8,  9, 10, 11,
                                  16, 17,         18, 19,
                                  12, 13,         14, 15,
                                  20, 21, 22, 23, 24, 25,
                                  26, 27, 28, 29, 30, 31))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((104, 105, 106, 107, 108, 109,
                                  110, 111, 112, 113, 114, 115,
                                  116, 117,  82,  83, 118, 119,
                                  120, 121,  88,  89, 122, 123,
                                  124, 125, 126, 127, 128, 129,
                                  130, 131, 132, 133, 134, 135))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_top_uneven_max_depth_0_5 (self):
        geometry = test_geometries.generate_two_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.UnevenTopSurfaceOn()
        model_generator.UseTopSurfaceMaximumDepthOn()
        model_generator.SetTopSurfaceMaximumDepth(0.5)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  2,  3,
                                   4,  5,  6,  7))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((22, 23,
                                  24, 25))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_top_uneven_max_depth_1_5 (self):
        geometry = test_geometries.generate_two_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.UnevenTopSurfaceOn()
        model_generator.UseTopSurfaceMaximumDepthOn()
        model_generator.SetTopSurfaceMaximumDepth(1.5)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  2,  3,
                                   4,  5,  6,  7))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((22, 23, 18,
                                  24, 25, 21))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_top_uneven_max_depth_2_5 (self):
        geometry = test_geometries.generate_two_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.UnevenTopSurfaceOn()
        model_generator.UseTopSurfaceMaximumDepthOn()
        model_generator.SetTopSurfaceMaximumDepth(2.5)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  2,  3,
                                   4,  5,  6,  7))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((22, 23, 18, 11,
                                  24, 25, 21, 15))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_bottom_uneven_max_depth_0_5 (self):
        geometry = test_geometries.generate_upside_down_two_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.UnevenBottomSurfaceOn()
        model_generator.UseBottomSurfaceMaximumDepthOn()
        model_generator.SetBottomSurfaceMaximumDepth(0.5)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,
                                   2,  3))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((18, 19, 20, 21,
                                  22, 23, 24, 25))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_bottom_uneven_max_depth_1_5 (self):
        geometry = test_geometries.generate_upside_down_two_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.UnevenBottomSurfaceOn()
        model_generator.UseBottomSurfaceMaximumDepthOn()
        model_generator.SetBottomSurfaceMaximumDepth(1.5)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  6,
                                   2,  3,  9))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((18, 19, 20, 21,
                                  22, 23, 24, 25))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_bottom_uneven_max_depth_2_5 (self):
        geometry = test_geometries.generate_upside_down_two_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.UnevenBottomSurfaceOn()
        model_generator.UseBottomSurfaceMaximumDepthOn()
        model_generator.SetBottomSurfaceMaximumDepth(2.5)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  6, 13,
                                   2,  3,  9, 17))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((18, 19, 20, 21,
                                  22, 23, 24, 25))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_specific_top_material (self):
        geometry = test_geometries.generate_quasi_donut_geometry_two_materials()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTopConstraintSpecificMaterial(2)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  2,  3,  4,  5,
                                   6,  7,  8,  9, 10, 11,
                                  16, 17,         18, 19,
                                  12, 13,         14, 15,
                                  20, 21, 22, 23, 24, 25,
                                  26, 27, 28, 29, 30, 31))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((106, 107, 108, 109,
                                  112, 113, 114, 115,
                                            118, 119,
                                            122, 123,
                                  126, 127, 128, 129,
                                  132, 133, 134, 135))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_specific_bottom_material (self):
        geometry = test_geometries.generate_quasi_donut_geometry_two_materials()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetBottomConstraintSpecificMaterial(2)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 2,  3,  4,  5,
                                   8,  9, 10, 11,
                                          18, 19,
                                          14, 15,
                                  22, 23, 24, 25,
                                  28, 29, 30, 31))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((104, 105, 106, 107, 108, 109,
                                  110, 111, 112, 113, 114, 115,
                                  116, 117,           118, 119,
                                  120, 121,           122, 123,
                                  124, 125, 126, 127, 128, 129,
                                  130, 131, 132, 133, 134, 135))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_top_uneven_specific_material (self):
        geometry = test_geometries.generate_quasi_donut_geometry_two_materials()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTopConstraintSpecificMaterial(2)
        model_generator.UnevenTopSurfaceOn()
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  2,  3,  4,  5,
                                   6,  7,  8,  9, 10, 11,
                                  16, 17,         18, 19,
                                  12, 13,         14, 15,
                                  20, 21, 22, 23, 24, 25,
                                  26, 27, 28, 29, 30, 31))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((106, 107, 108, 109,
                                  112, 113, 114, 115,
                                   82,  83, 118, 119,
                                   88,  89, 122, 123,
                                  126, 127, 128, 129,
                                  132, 133, 134, 135))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_test_axis_x_uneven_surface (self):
        geometry = test_geometries.generate_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(0)
        model_generator.UnevenTopSurfaceOn()
        model_generator.UnevenBottomSurfaceOn()  # shouldn't make any difference
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  9, 18,
                                   3, 12, 20,
                                   6, 15, 22))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 19, 11, 2,
                                   21, 14, 5,
                                   23, 17, 8))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_test_axis_x_uneven_surface_top_specific_material (self):
        geometry = test_geometries.generate_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(0)
        model_generator.UnevenTopSurfaceOn()
        model_generator.UnevenBottomSurfaceOn()  # shouldn't make any difference
        model_generator.SetTopConstraintSpecificMaterial(200)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  9, 18,
                                   3, 12, 20,
                                   6, 15, 22))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 21, 14, 5,
                                   23, 17, 8))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_test_axis_x_uneven_surface_bottom_specific_material (self):
        geometry = test_geometries.generate_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(0)
        model_generator.UnevenTopSurfaceOn()
        model_generator.UnevenBottomSurfaceOn()  # shouldn't make any difference
        model_generator.SetBottomConstraintSpecificMaterial(100)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  9, 18,
                                   3, 12, 20))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 19, 11, 2,
                                   21, 14, 5,
                                   23, 17, 8))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_test_axis_y_top_uneven_max_depth_0_5 (self):
        geometry = test_geometries.generate_sideways_two_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(1)
        model_generator.UnevenTopSurfaceOn()
        model_generator.UseTopSurfaceMaximumDepthOn()
        model_generator.SetTopSurfaceMaximumDepth(0.5)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  2,  3,
                                  13, 14, 15, 16))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((11, 12,
                                  24, 25))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_test_axis_y_top_uneven_max_depth_1_5 (self):
        geometry = test_geometries.generate_sideways_two_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(1)
        model_generator.UnevenTopSurfaceOn()
        model_generator.UseTopSurfaceMaximumDepthOn()
        model_generator.SetTopSurfaceMaximumDepth(1.5)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  2,  3,
                                  13, 14, 15, 16))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((11, 12, 10,
                                  24, 25, 23))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


    def test_test_axis_y_top_uneven_max_depth_2_5 (self):
        geometry = test_geometries.generate_sideways_two_step_geometry()
        model_generator = vtkbone.vtkboneApplyTestBase()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputData(1, materials)
        model_generator.SetTestAxis(1)
        model_generator.UnevenTopSurfaceOn()
        model_generator.UseTopSurfaceMaximumDepthOn()
        model_generator.SetTopSurfaceMaximumDepth(2.5)
        model_generator.Update()
        model = model_generator.GetOutput()
        # Check Node Sets
        nodeset_vtk = model.GetNodeSet("face_z0")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array(( 0,  1,  2,  3,
                                  13, 14, 15, 16))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))
        nodeset_vtk = model.GetNodeSet("face_z1")
        self.assertFalse (nodeset_vtk is None)
        nodeset = vtk_to_numpy(nodeset_vtk)
        nodeset_expected = array((11, 12, 10, 7,
                                  24, 25, 23, 20))
        self.assertTrue(alltrue(sort(nodeset) == sort(nodeset_expected)))


if __name__ == '__main__':
    unittest.main()
