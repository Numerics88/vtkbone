""" Utilities for generating Unstructured Grids from 3D images.
"""

from __future__ import division
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import itertools


def convert_map_to_ids (m):
    """Converts a map, which is simply a 3D image with non-zero values wherever
    a hexahedral  cell should be located, to an id map, in which each cell
    (i.e. image voxel) is assigned a unique id, starting at 0 and numbered
    sequentially, with x being the fastest changing coordinate."""
    m_flat = m.reshape((product(m.shape),))
    ids = zeros(m.shape , int)
    ids_flat = ids.reshape((product(ids.shape),))
    index = 0
    for i in xrange(m_flat.shape[0]):
        if m_flat[i]:
            ids_flat[i] = index
            index += 1
        else:
            ids_flat[i] = -1
    return ids, index

    
def pointmap_from_cellmap (cellmap):
    """Given a cell map, which is a 3D image with non-zero values wherever
    a hexahedral  cell should be located, to a point map.  The point map has size
    one larger in each dimension.  A point in the point map is non-zero
    if any of the cells for which the point is a corner is non-zero."""
    D = array(cellmap.shape)
    pointmap = zeros(D+1, int)
    pointmap[ :D[0]  , :D[1],   :D[2]  ] += cellmap
    pointmap[ :D[0]  , :D[1],1  :D[2]+1] += cellmap
    pointmap[ :D[0]  ,1:D[1]+1, :D[2]  ] += cellmap
    pointmap[ :D[0]  ,1:D[1]+1,1:D[2]+1] += cellmap
    pointmap[1:D[0]+1, :D[1],   :D[2]  ] += cellmap
    pointmap[1:D[0]+1, :D[1],1  :D[2]+1] += cellmap
    pointmap[1:D[0]+1,1:D[1]+1, :D[2]  ] += cellmap
    pointmap[1:D[0]+1,1:D[1]+1,1:D[2]+1] += cellmap
    pointmap, numberpoints = convert_map_to_ids(pointmap)
    return pointmap, numberpoints


def generate_point_coordinates (pointmap, numberpoints, offset=array((0.0,0.0,0.0)), spacing=array((1.0,1.0,1.0))):
    """Given a point id map (ie. a 3D image labelling point ids, with -1
    wherever a point should be exluded), generate a list of coordinates.
    The coordinate list can be indexed with the point ids."""
    coord = zeros((numberpoints, 3), float)
    for (k,j,i) in itertools.product(xrange(pointmap.shape[0]),
                                     xrange(pointmap.shape[1]),
                                     xrange(pointmap.shape[2])):
        if pointmap[k,j,i] != -1:
            coord[pointmap[k,j,i]] = offset + array((i,j,k))*spacing
    return coord


def convert_cellmap_to_image (cellmap, offset=array((0.0,0.0,0.0)), spacing=array((1.0,1.0,1.0))):
    """Given a cell map, which is a 3D image with non-zero values wherever
    a hexahedral cell should be located, generate a vtkImageData
    with data on the cells.
    """
    image = vtk.vtkImageData()
    image.SetOrigin(offset)
    image.SetSpacing(spacing)
    image.SetDimensions(cellmap.shape[2]+1, cellmap.shape[1]+1, cellmap.shape[0]+1)
    cellmap_flat = cellmap.flatten()
    scalars_as_vtk = numpy_to_vtk(cellmap_flat, deep=1)
    image.GetCellData().SetScalars(scalars_as_vtk)
    return image


def convert_cellmap_to_unstructuredgrid (cellmap, offset=array((0.0,0.0,0.0)), spacing=array((1.0,1.0,1.0))):
    """Given a cell map, which is a 3D image with non-zero values wherever
    a hexahedral cell should be located, generate a vtkUnstructuredGrid
    object with vtkVoxel type cells.  The values in the image are converted
    directly to VTK cell scalars."""
    pointmap, numpoints = pointmap_from_cellmap(cellmap)
    coord = generate_point_coordinates(pointmap, numpoints, offset, spacing)
    cellids, numcells = convert_map_to_ids(cellmap)
    points = vtk.vtkPoints()
    coord_as_vtk = numpy_to_vtk(coord, deep=1)
    points.SetData(coord_as_vtk)
    geometry = vtk.vtkUnstructuredGrid()
    geometry.Allocate(numcells,1)
    geometry.SetPoints(points)
    for (k,j,i) in itertools.product(xrange(cellmap.shape[0]),
                                     xrange(cellmap.shape[1]),
                                     xrange(cellmap.shape[2])):
        if cellmap[k,j,i]:
            voxel = vtk.vtkVoxel()
            voxel.GetPointIds().SetId(0, pointmap[k  ,j  ,i  ])
            voxel.GetPointIds().SetId(1, pointmap[k  ,j  ,i+1])
            voxel.GetPointIds().SetId(2, pointmap[k  ,j+1,i  ])
            voxel.GetPointIds().SetId(3, pointmap[k  ,j+1,i+1])
            voxel.GetPointIds().SetId(4, pointmap[k+1,j  ,i  ])
            voxel.GetPointIds().SetId(5, pointmap[k+1,j  ,i+1])
            voxel.GetPointIds().SetId(6, pointmap[k+1,j+1,i  ])
            voxel.GetPointIds().SetId(7, pointmap[k+1,j+1,i+1])
            geometry.InsertNextCell(voxel.GetCellType(), voxel.GetPointIds())
    scalars = zeros(numcells, float)
    for (k,j,i) in itertools.product(xrange(cellmap.shape[0]),
                                     xrange(cellmap.shape[1]),
                                     xrange(cellmap.shape[2])):
        if cellmap[k,j,i]:
            scalars[cellids[k,j,i]] = cellmap[k,j,i]
    assert(alltrue(scalars != 0.0))   # Should always pass
    scalars_as_vtk = numpy_to_vtk(scalars, deep=1)
    geometry.GetCellData().SetScalars(scalars_as_vtk)
    return geometry
