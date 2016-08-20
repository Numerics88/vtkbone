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


class TestNodeSetsByGeometry (unittest.TestCase):

    def test_DetermineMaterialBounds (self):
        
        geometry = test_geometries.generate_quasi_donut_geometry_two_materials()
        bounds = zeros(6, float)
        vtkbone.vtkboneNodeSetsByGeometry.DetermineMaterialBounds(geometry, bounds, -1)
        expected_bounds = array([0.0, 5.0, 0.0, 5.0, 0.0, 3.0])
        self.assertTrue (alltrue(bounds == expected_bounds))
        
        # Material 1 has 0 < x < 2
        vtkbone.vtkboneNodeSetsByGeometry.DetermineMaterialBounds(geometry, bounds, 1)
        expected_bounds = array([0.0, 2.0, 0.0, 5.0, 0.0, 3.0])
        self.assertTrue (alltrue(bounds == expected_bounds))

        # Material 1 has 2 < x < 5
        vtkbone.vtkboneNodeSetsByGeometry.DetermineMaterialBounds(geometry, bounds, 2)
        expected_bounds = array([2.0, 5.0, 0.0, 5.0, 0.0, 3.0])
        self.assertTrue (alltrue(bounds == expected_bounds))

        # Try shifted origin
        geometry2 = test_geometries.generate_quasi_donut_geometry_two_materials_offset()
        # The following conditional is useful for writing out the model.
        if 0:
            writer = vtk.vtkXMLUnstructuredGridWriter()
            writer.SetInput(geometry2)
            writer.SetFileName("geometry2.vtu")
            writer.Update()
        vtkbone.vtkboneNodeSetsByGeometry.DetermineMaterialBounds(geometry2, bounds, -1)
        expected_bounds = array((0.5, 3.0, 1.0, 6.0, -0.5, 5.5))
        self.assertTrue (alltrue(bounds == expected_bounds))


    def test_FindNodesOnPlane (self):
        
        geometry = test_geometries.generate_quasi_donut_geometry_two_materials_offset()
        expected_bounds = array((0.5, 3.0, 1.0, 6.0, -0.5, 5.5))

        # X min surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(0, 0.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26, 62, 98, 130,
                                   20, 56, 92, 124,
                                   16, 50, 86, 120,
                                   12, 44, 80, 116,
                                    6, 38, 74, 110,
                                    0, 32, 68, 104)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min surface, material type 1 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(0, 0.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min surface, material type 2 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(0, 0.5, ids_vtk, geometry, 2)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)

        # X max surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(0, 3.0, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((31, 67, 103, 135,
                                   25, 61,  97, 129,
                                   19, 55,  91, 123,
                                   15, 49,  85, 119,
                                   11, 43,  79, 115,
                                    5, 37,  73, 109)))
        self.assertTrue (alltrue(ids == expected_ids))

        # X max surface, material type 1 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(0, 3.0, ids_vtk, geometry, 1)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)

        # Y min surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(1, 1.0, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((  0,   1,   2,   3,   4,   5,
                                    32,  33,  34,  35,  36,  37,
                                    68,  69,  70,  71,  72,  73,
                                   104, 105, 106, 107, 108, 109)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min surface, material type 1 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(1, 1.0, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((  0,   1,   2,
                                    32,  33,  34,
                                    68,  69,  70,
                                   104, 105, 106)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min surface, material type 2 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(1, 1.0, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((  2,   3,   4,   5,
                                    34,  35,  36,  37,
                                    70,  71,  72,  73,
                                   106, 107, 108, 109)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(1, 6.0, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array(( 26,  27,  28,  29,  30,  31,
                                    62,  63,  64,  65,  66,  67,
                                    98,  99, 100, 101, 102, 103,
                                   130, 131, 132, 133, 134, 135)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max surface, material type 1 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(1, 6.0, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array(( 26,  27,  28,
                                    62,  63,  64,
                                    98,  99, 100,
                                   130, 131, 132)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max surface, material type 2 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(1, 6.0, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((  28,  29,  30,  31,
                                     64,  65,  66,  67,
                                    100, 101, 102, 103,
                                    132, 133, 134, 135)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z min surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(2, -0.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26,  27,  28,  29,  30,  31,
                                   20,  21,  22,  23,  24,  25,
                                   16,  17,            18,  19,
                                   12,  13,            14,  15,
                                    6,   7,   8,   9,  10,  11,
                                    0,   1,   2,   3,   4,   5)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z min surface, material type 1 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(2, -0.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26,  27,  28,
                                   20,  21,  22,
                                   16,  17,
                                   12,  13,
                                    6,   7,   8,
                                    0,   1,   2)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z min surface, material type 2 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(2, -0.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((28,  29,  30,  31,
                                   22,  23,  24,  25,
                                             18,  19,
                                             14,  15,
                                    8,   9,  10,  11,
                                    2,   3,   4,   5)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z max surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(2, 5.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((130, 131, 132, 133, 134, 135,
                                   124, 125, 126, 127, 128, 129,
                                   120, 121,           122, 123,
                                   116, 117,           118, 119,
                                   110, 111, 112, 113, 114, 115,
                                   104, 105, 106, 107, 108, 109)))
        self.assertTrue (alltrue(ids == expected_ids))

        # Z max surface, material type 1 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(2, 5.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((130, 131, 132,
                                   124, 125, 126,
                                   120, 121,
                                   116, 117,
                                   110, 111, 112,
                                   104, 105, 106)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z max surface, material type 2 elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnPlane(2, 5.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((132, 133, 134, 135,
                                   126, 127, 128, 129,
                                             122, 123,
                                             118, 119,
                                   112, 113, 114, 115,
                                   106, 107, 108, 109)))
        self.assertTrue (alltrue(ids == expected_ids))


    def test_FindNodesIntersectingTwoPlanes (self):
        
        geometry = test_geometries.generate_quasi_donut_geometry_two_materials_offset()
        expected_bounds = array((0.5, 3.0, 1.0, 6.0, -0.5, 5.5))
        
        # X min, Y min edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 1, 1.0, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((0, 32, 68, 104)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y min edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 1, 1.0, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y min edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 1, 1.0, ids_vtk, geometry, 2)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)

        # X max, Y min edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 1, 1.0, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((5, 37, 73, 109)))
        self.assertTrue (alltrue(ids == expected_ids))

        # X max, Y min edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 1, 1.0, ids_vtk, geometry, 1)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y min edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 1, 1.0, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y max edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 1, 6.0, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26, 62, 98, 130)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y max edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 1, 6.0, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y max edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 1, 6.0, ids_vtk, geometry, 2)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y max edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 1, 6.0, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((31, 67, 103, 135)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X max, Y max edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 1, 6.0, ids_vtk, geometry, 1)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y max edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 1, 6.0, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Z min edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 2, -0.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26, 20, 16, 12, 6, 0)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Z min edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 2, -0.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Z min edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 2, -0.5, ids_vtk, geometry, 2)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Z min edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 2, -0.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((31, 25, 19, 15, 11, 5)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X max, Z min edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 2, -0.5, ids_vtk, geometry, 1)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Z min edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 2, -0.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Z max edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 2, 5.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((130, 124, 120, 116, 110, 104)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Z max edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 2, 5.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((130, 124, 120, 116, 110, 104)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Z max edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 0.5, 2, 5.5, ids_vtk, geometry, 2)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Z max edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 2, 5.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((135, 129, 123, 119, 115, 109)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X max, Z max edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 2, 5.5, ids_vtk, geometry, 1)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Z max edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          0, 3.0, 2, 5.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min, Z min edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 1.0, 2, -0.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((0, 1, 2, 3, 4, 5)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min, Z min edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 1.0, 2, -0.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((0, 1, 2)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min, Z min edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 1.0, 2, -0.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((2, 3, 4, 5)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Try the previous test with a different order of axes.
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          2, -0.5, 1, 1.0, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max, Z min edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 6.0, 2, -0.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26, 27, 28, 29, 30, 31)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max, Z min edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 6.0, 2, -0.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26, 27, 28)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max, Z min edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 6.0, 2, -0.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((28, 29, 30, 31)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min, Z max edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 1.0, 2, 5.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((104, 105, 106, 107, 108, 109)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min, Z max edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 1.0, 2, 5.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((104, 105, 106)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min, Z max edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 1.0, 2, 5.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((106, 107, 108, 109)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max, Z max edge, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 6.0, 2, 5.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((130, 131, 132, 133, 134, 135)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max, Z max edge, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 6.0, 2, 5.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((130, 131, 132)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max, Z max edge, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingTwoPlanes(
                                          1, 6.0, 2, 5.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((132, 133, 134, 135)))
        self.assertTrue (alltrue(ids == expected_ids))


    def test_FindNodesIntersectingThreePlanes (self):
        
        geometry = test_geometries.generate_quasi_donut_geometry_two_materials_offset()
        expected_bounds = array((0.5, 3.0, 1.0, 6.0, -0.5, 5.5))
        
        # X min, Y min, Z min corner, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 1.0, 2, -0.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((0,)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y min, Z min corner, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 1.0, 2, -0.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y min, Z min corner, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 1.0, 2, -0.5, ids_vtk, geometry, 2)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y min, Z min corner, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 1.0, 2, -0.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((5,)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X max, Y min, Z min corner, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 1.0, 2, -0.5, ids_vtk, geometry, 1)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y min, Z min corner, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 1.0, 2, -0.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y max, Z min corner, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 6.0, 2, -0.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26,)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y max, Z min corner, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 6.0, 2, -0.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y max, Z min corner, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 6.0, 2, -0.5, ids_vtk, geometry, 2)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y max, Z min corner, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 6.0, 2, -0.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((31,)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X max, Y max, Z min corner, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 6.0, 2, -0.5, ids_vtk, geometry, 1)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y max, Z min corner, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 6.0, 2, -0.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y min, Z max corner, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 1.0, 2, 5.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((104,)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y min, Z max corner, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 1.0, 2, 5.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y min, Z max corner, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 1.0, 2, 5.5, ids_vtk, geometry, 2)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y min, Z max corner, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 1.0, 2, 5.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((109,)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # same as previous, but change order of axes
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  1, 1.0, 2, 5.5, 0, 3.0, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # same as previous, but change order of axes
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  2, 5.5, 0, 3.0, 1, 1.0, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X max, Y min, Z max corner, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 1.0, 2, 5.5, ids_vtk, geometry, 1)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y min, Z max corner, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 1.0, 2, 5.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y max, Z max corner, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 6.0, 2, 5.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((130,)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y max, Z max corner, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 6.0, 2, 5.5, ids_vtk, geometry, 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min, Y max, Z max corner, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 0.5, 1, 6.0, 2, 5.5, ids_vtk, geometry, 2)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y max, Z max corner, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 6.0, 2, 5.5, ids_vtk, geometry, -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((135,)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X max, Y max, Z max corner, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 6.0, 2, 5.5, ids_vtk, geometry, 1)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max, Y max, Z max corner, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesIntersectingThreePlanes(
                                  0, 3.0, 1, 6.0, 2, 5.5, ids_vtk, geometry, 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))


      # static void FindNodesOnVisibleSurface(
      #   vtkIdTypeArray *visibleNodesIds,
      #   vtkUnstructuredGrid *ug,
      #   double normalVector[3],
      #   int specificMaterial = -1);
    def test_FindNodesOnVisibleSurface (self):
        
        geometry = test_geometries.generate_quasi_donut_geometry_two_materials_offset()
        
        # X min surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (-1,0,0), -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26, 62, 98, 130,
                                   20, 56, 92, 124,
                                   16, 50, 86, 120,
                                   12, 44, 80, 116,
                                    6, 38, 74, 110,
                                    0, 32, 68, 104)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X min surface, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (-1,0,0), 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))

        # X min surface, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (-1,0,0), 2)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (1,0,0), -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((31, 67, 103, 135,
                                   25, 61,  97, 129,
                                   19, 55,  91, 123,
                                   15, 49,  85, 119,
                                   11, 43,  79, 115,
                                    5, 37,  73, 109)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # X max surface, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (1,0,0), 1)
        self.assertEqual (ids_vtk.GetNumberOfTuples(), 0)
        
        # X max surface, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (1,0,0), 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,-1,0), -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((  0,   1,   2,   3,   4,   5,
                                    32,  33,  34,  35,  36,  37,
                                    68,  69,  70,  71,  72,  73,
                                   104, 105, 106, 107, 108, 109)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min surface, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,-1,0), 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((  0,   1,   2,
                                    32,  33,  34,
                                    68,  69,  70,
                                   104, 105, 106)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y min surface, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,-1,0), 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((  2,   3,   4,   5,
                                    34,  35,  36,  37,
                                    70,  71,  72,  73,
                                   106, 107, 108, 109)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,1,0), -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array(( 26,  27,  28,  29,  30,  31,
                                    62,  63,  64,  65,  66,  67,
                                    98,  99, 100, 101, 102, 103,
                                   130, 131, 132, 133, 134, 135)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max surface, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,1,0), 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array(( 26,  27,  28,
                                    62,  63,  64,
                                    98,  99, 100,
                                   130, 131, 132)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Y max surface, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,1,0), 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array(( 28,  29,  30,  31,
                                    64,  65,  66,  67,
                                   100, 101, 102, 103,
                                   132, 133, 134, 135)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z min surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,0,-1), -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26,  27,  28,  29,  30,  31,
                                   20,  21,  22,  23,  24,  25,
                                   16,  17,  52,  53,  18,  19,
                                   12,  13,  46,  47,  14,  15,
                                    6,   7,   8,   9,  10,  11,
                                    0,   1,   2,   3,   4,   5)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z min surface, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,0,-1), 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((26,  27,  28,
                                   20,  21,  22,
                                   16,  17,  52,
                                   12,  13,  46,
                                    6,   7,   8,
                                    0,   1,   2)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z min surface, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,0,-1), 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((28,  29,  30,  31,
                                   22,  23,  24,  25,
                                   52,  53,  18,  19,
                                   46,  47,  14,  15,
                                    8,   9,  10,  11,
                                    2,   3,   4,   5)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z max surface, all elements
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,0,1), -1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((130, 131, 132, 133, 134, 135,
                                   124, 125, 126, 127, 128, 129,
                                   120, 121,  88,  89, 122, 123,
                                   116, 117,  82,  83, 118, 119,
                                   110, 111, 112, 113, 114, 115,
                                   104, 105, 106, 107, 108, 109)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z max surface, material ID 1
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,0,1), 1)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((130, 131, 132,
                                   124, 125, 126,
                                   120, 121,  88,
                                   116, 117,  82,
                                   110, 111, 112,
                                   104, 105, 106)))
        self.assertTrue (alltrue(ids == expected_ids))
        
        # Z max surface, material ID 2
        ids_vtk = vtk.vtkIdTypeArray()
        vtkbone.vtkboneNodeSetsByGeometry.FindNodesOnVisibleSurface(ids_vtk, geometry, (0,0,1), 2)
        ids = sort(vtk_to_numpy(ids_vtk))
        expected_ids = sort(array((132, 133, 134, 135,
                                   126, 127, 128, 129,
                                    88,  89, 122, 123,
                                    82,  83, 118, 119,
                                   112, 113, 114, 115,
                                   106, 107, 108, 109)))
        self.assertTrue (alltrue(ids == expected_ids))


if __name__ == '__main__':
    unittest.main()
