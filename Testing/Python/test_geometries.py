from __future__ import division
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import geometry_utilities

# Define some functions to create very simple geometries.


# ==========================================================================
# Two-element geometry
#
#       
#           10--------11
#         / |       / |
#        /  |      /  |
#       8---------9   |
#       |   |     |   |
#       |   6-----|---7
#       | / |     | / |
#       |/  |     |/  |
#       4---------5   |
#       |   |     |   |
#       |   2-----|---3
#       |  /      |  /
#       |/        | /     z  y
#       0---------1       | /
#                         .-- x
#
def generate_two_element_geometry():
    points = vtk.vtkPoints()
    points.InsertPoint( 0, 0.0, 0.0, 0.0)
    points.InsertPoint( 1, 1.0, 0.0, 0.0)
    points.InsertPoint( 2, 0.0, 1.0, 0.0)
    points.InsertPoint( 3, 1.0, 1.0, 0.0)
    points.InsertPoint( 4, 0.0, 0.0, 1.0)
    points.InsertPoint( 5, 1.0, 0.0, 1.0)
    points.InsertPoint( 6, 0.0, 1.0, 1.0)
    points.InsertPoint( 7, 1.0, 1.0, 1.0)
    points.InsertPoint( 8, 0.0, 0.0, 2.0)
    points.InsertPoint( 9, 1.0, 0.0, 2.0)
    points.InsertPoint(10, 0.0, 1.0, 2.0)
    points.InsertPoint(11, 1.0, 1.0, 2.0)
    voxel0 = vtk.vtkVoxel()
    voxel0.GetPointIds().SetId(0, 0)
    voxel0.GetPointIds().SetId(1, 1)
    voxel0.GetPointIds().SetId(2, 2)
    voxel0.GetPointIds().SetId(3, 3)
    voxel0.GetPointIds().SetId(4, 4)
    voxel0.GetPointIds().SetId(5, 5)
    voxel0.GetPointIds().SetId(6, 6)
    voxel0.GetPointIds().SetId(7, 7)
    voxel1 = vtk.vtkVoxel()
    voxel1.GetPointIds().SetId(0, 4)
    voxel1.GetPointIds().SetId(1, 5)
    voxel1.GetPointIds().SetId(2, 6)
    voxel1.GetPointIds().SetId(3, 7)
    voxel1.GetPointIds().SetId(4, 8)
    voxel1.GetPointIds().SetId(5, 9)
    voxel1.GetPointIds().SetId(6, 10)
    voxel1.GetPointIds().SetId(7, 11)
    geometry = vtk.vtkUnstructuredGrid()
    geometry.Allocate(2,1)
    geometry.InsertNextCell(voxel0.GetCellType(), voxel0.GetPointIds())
    geometry.InsertNextCell(voxel1.GetCellType(), voxel1.GetPointIds())
    scalars = vtk.vtkFloatArray()
    scalars.SetNumberOfValues(2)
    scalars.SetValue(0, 1)
    scalars.SetValue(1, 1)
    geometry.GetCellData().SetScalars(scalars)
    geometry.SetPoints(points)
    return geometry


# ==========================================================================
# Two-element geometry, offset from origin and scaled
#
# Spacing is 0.8, 1.5, 0.5
# Origin is  -1.0, 0.5, 2.0
# Bounds are -1.0, -0.2, 0.5, 2.0, 2.0, 3.0
#
def generate_two_element_geometry_nonsquare():
    cellmap = ones((2,1,1), dtype=int)
    offset = array((-1.0,0.5,2.0))
    spacing = array((0.8,1.5,0.5))
    geometry = geometry_utilities.convert_cellmap_to_unstructuredgrid(cellmap, offset, spacing)
    return geometry


# ==========================================================================
# Step Shape, two materials
#
#                22------23
#              /        / |
#            /  200   /   |
#          20------21     |
#        /        / |    16-------17
#      /        /   |   /        / |
#    18------19     | /  200   /   |
#    |        |    13-------14     |
#    |  100   |   /        / |     8
#    |        | /        /   |   /
#    9-------10-------11     | /
#    |        |        |     5
#    |  100   |  100   |   /               z  y
#    |        |        | /                 | /
#    0--------1--------2                   .-- x
#
#
# z = 0 layer node numbers:
#
#   0  1  2
#   3  4  5
#   6  7  8
#
# z = 1 layer node numbers:
#
#   9 10 11
#  12 13 14
#  15 16 17
#
# z = 2 layer node numbers:
#
#  18 19
#  20 21
#  22 23
#
def generate_step_cellmap():
    cellmap = 100*ones((2,2,2), dtype=int)
    cellmap[:,1,:] = 200
    cellmap[1,:,1] = 0
    return cellmap
def generate_step_image():
    cellmap = generate_step_cellmap()
    geometry = geometry_utilities.convert_cellmap_to_image(cellmap)
    return geometry
def generate_step_geometry():
    cellmap = generate_step_cellmap()
    geometry = geometry_utilities.convert_cellmap_to_unstructuredgrid(cellmap)
    return geometry


# ==========================================================================
# Step Shape, two materials, offset from origin and scaled
#
# Spacing is 0.5, 1.0, 1.5
# Origin is  0.5, 1.0, -0.5
# Bounds are 0.5, 1.5, 1.0, 3.0, -0.5, 2.5
#
def generate_step_offset_cellmap():
    cellmap = 100*ones((2,2,2), dtype=int)
    cellmap[:,1,:] = 200
    cellmap[1,:,1] = 0
    return cellmap
def generate_step_offset_image():
    cellmap = generate_step_cellmap()
    offset = array((0.5,1.0,-0.5))
    spacing = array((0.5,1.0,1.5))
    geometry = geometry_utilities.convert_cellmap_to_image(cellmap, offset, spacing)
    return geometry
def generate_step_offset_geometry():
    cellmap = generate_step_cellmap()
    offset = array((0.5,1.0,-0.5))
    spacing = array((0.5,1.0,1.5))
    geometry = geometry_utilities.convert_cellmap_to_unstructuredgrid(cellmap, offset, spacing)
    return geometry


# ==========================================================================
# Two Step Shape
#
#          24------25
#        /        / |
#      /        /   |
#    22------23     |
#    |        |    20-------21
#    |        |   /        / |
#    |        | /        /   |
#    16------17-------18     |
#    |        |        |    14-------15
#    |        |        |   /        / |
#    |        |        | /        /   |
#    8--------9-------10-------11     |
#    |        |        |        |     7
#    |        |        |        |   /               z  y
#    |        |        |        | /                 | /
#    0--------1--------2--------3                   .-- x
#
#
# z = 0 layer node numbers:
#
#   0  1  2  3
#   4  5  6  7
#
# z = 1 layer node numbers:
#
#   8  9 10 11
#  12 13 14 15
#
# z = 2 layer node numbers:
#
#  16 17 18
#  19 20 21
#
# z = 3 layer node numbers:
#
#  22 23
#  24 25
#
def generate_two_step_cellmap():
    cellmap = 100*ones((3,1,3), dtype=int)
    cellmap[1,0,2] = 0
    cellmap[2,0,1] = 0
    cellmap[2,0,2] = 0
    return cellmap
def generate_two_step_image():
    cellmap = generate_two_step_cellmap()
    geometry = geometry_utilities.convert_cellmap_to_image(cellmap)
    return geometry
def generate_two_step_geometry():
    cellmap = generate_two_step_cellmap()
    geometry = geometry_utilities.convert_cellmap_to_unstructuredgrid(cellmap)
    return geometry


# ==========================================================================
# Upside Down Two Step Shape
#
#          22------23-------24-------25
#        /        /        /        / |
#      /        /        /        /   |
#    18------19-------20-------21     |
#    |        |        |        |    17
#    |        |        |        |   /
#    |        |        |        | /
#    10------11-------12----+--13
#    |        |        |    9
#    |        |        |   /
#    |        |        | /
#    4--------5-----+--6
#    |        |     3
#    |        |   /               z  y
#    |        | /                 | /
#    0--------1                   .-- x
#
#
# z = 0 layer node numbers:
#
#   0  1 
#   2  3
#
# z = 1 layer node numbers:
#
#   4  5  6
#   7  8  9
#
# z = 2 layer node numbers:
#
#  10 11 12 13
#  14 15 16 17
#
# z = 3 layer node numbers:
#
#  18 19 20 21
#  22 23 24 25
#
def generate_upside_down_two_step_cellmap():
    cellmap = 100*ones((3,1,3), dtype=int)
    cellmap[1,0,2] = 0
    cellmap[0,0,1] = 0
    cellmap[0,0,2] = 0
    return cellmap
def generate_upside_down_two_step_image():
    cellmap = generate_upside_down_two_step_cellmap()
    geometry = geometry_utilities.convert_cellmap_to_image(cellmap)
    return geometry
def generate_upside_down_two_step_geometry():
    cellmap = generate_upside_down_two_step_cellmap()
    geometry = geometry_utilities.convert_cellmap_to_unstructuredgrid(cellmap)
    return geometry


# ==========================================================================
# Sideways Two Step Shape
#
#                      24------25
#                    /        / |
#                  /        /   |
#                21------22-------23
#              /        /        / |
#            /        /        /   |
#          17------18-------19-------20
#        /        /        /        / |
#      /        /        /        /   |
#    13------14-------15-------16     |                z  y
#    |        |        |        |     7                | /
#    |        |        |        |   /                  .-- x
#    |        |        |        | /
#    0--------1--------2--------3
#
#
# z = 0 layer node numbers:
#
#   0  1  2  3
#   4  5  6  7
#   8  9 10
#  11 12
#
# z = 1 layer node numbers:
#
#   13 14 15 16
#   17 18 19 20
#   21 22 23
#   24 25
#
def generate_sideways_two_step_cellmap():
    cellmap = 100*ones((1,3,3), dtype=int)
    cellmap[0,1,2] = 0
    cellmap[0,2,1] = 0
    cellmap[0,2,2] = 0
    return cellmap
def generate_sideways_two_step_image():
    cellmap = generate_sideways_two_step_cellmap()
    geometry = geometry_utilities.convert_cellmap_to_image(cellmap)
    return geometry
def generate_sideways_two_step_geometry():
    cellmap = generate_sideways_two_step_cellmap()
    geometry = geometry_utilities.convert_cellmap_to_unstructuredgrid(cellmap)
    return geometry


# ==========================================================================
# Quasi-donut geometry
#
# This geometry is useful for checking uneven surface node identification.
#
#  Bottom layer node numbering (z=0):
#
#  26  27  28  29  30  31
#  20  21  22  23  24  25      y
#  16  17          18  19      ^
#  12  13          14  15      |
#   6   7   8   9  10  11      |
#   0   1   2   3   4   5      .---> x
#
#  z=1 layer:
#
#  62  63  64  65  66  67
#  56  57  58  59  60  61
#  50  51  52  53  54  55
#  44  45  46  47  48  49
#  38  39  40  41  42  43
#  32  33  34  35  36  37
#
#  z=2 layer:
#
#  98  99 100 101 102 103
#  92  93  94  95  96  97
#  86  87  88  89  90  91
#  80  81  82  83  84  85
#  74  75  76  77  78  79
#  68  69  70  71  72  73
#
#  z=3 layer:
#
# 130 131 132 133 134 135
# 124 125 126 127 128 129
# 120 121         122 123
# 116 117         118 119
# 110 111 112 113 114 115
# 104 105 106 107 108 109
#
#
# Element numbering, layers from bottom up:
#
#   11  12  13  14  15
#    9              10
#    7               8  
#    5               6
#    0   1   2   3   4 
#
#
#   36  37  38  39  40
#   31  32  33  34  35
#   26  27  28  29  30
#   21  22  23  24  25
#   16  17  18  19  20
#
#
#   52  53  54  55  56
#   50              51
#   48              49
#   46              47
#   41  42  43  44  45
#
def generate_quasi_donut_geometry():
    cellmap = ones((3,5,5), dtype=int)
    cellmap[(0,2),1:4,1:4] = 0
    geometry = geometry_utilities.convert_cellmap_to_unstructuredgrid(cellmap)
    return geometry 


# ==========================================================================
# Quasi-donut geometry, two materials
#
# Similar to Quasi-donut, except that two materials are used.
# For cells with x < 2, value 1 is assigned;
# for cells with x > 2, value 2 is assigned.
def generate_quasi_donut_geometry_two_materials():
    cellmap = ones((3,5,5), dtype=int)
    cellmap[:,:,2:5] = 2
    cellmap[(0,2),1:4,1:4] = 0
    geometry = geometry_utilities.convert_cellmap_to_unstructuredgrid(cellmap)
    return geometry 

# ==========================================================================
# Quasi-donut geometry with two materials, offset from origin and scaled
#
# Spacing is 0.5, 1.0, 2.0
# Origin is  0.5, 1.0, -0.5
# Bounds are 0.5, 3.0, 1.0, 6.0, -0.5, 5.5
#
def generate_quasi_donut_geometry_two_materials_offset():
    cellmap = ones((3,5,5), dtype=int)
    cellmap[:,:,2:5] = 2
    cellmap[(0,2),1:4,1:4] = 0
    offset = array((0.5,1.0,-0.5))
    spacing = array((0.5,1.0,2.0))
    geometry = geometry_utilities.convert_cellmap_to_unstructuredgrid(cellmap, offset, spacing)
    return geometry 
