from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import unittest


class TestImageConnectivityMap (unittest.TestCase):

    def test_data_on_points(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,3,3)
        input_image.SetOrigin (0.2,-1.0,1.5)
        input_image.SetSpacing (2,4,5.5)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data_vtk = input_image.GetPointData().GetScalars()
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        cmap = filter.GetOutput()
        self.assertEqual (cmap.GetExtent(), input_image.GetExtent())
        self.assertEqual (cmap.GetOrigin(), input_image.GetOrigin())
        self.assertEqual (cmap.GetSpacing(), input_image.GetSpacing())
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        self.assertEqual (cmap_data_vtk.GetNumberOfTuples(), 27)


    def test_data_on_cells(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,3,3)
        input_image.SetOrigin (0.2,-1.0,1.5)
        input_image.SetSpacing (2,4,5.5)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data_vtk = vtk.vtkShortArray()
        input_data_vtk.SetNumberOfTuples(8)
        input_image.GetCellData().SetScalars (input_data_vtk)
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        cmap = filter.GetOutput()
        self.assertEqual (cmap.GetExtent(), input_image.GetExtent())
        self.assertEqual (cmap.GetOrigin(), input_image.GetOrigin())
        self.assertEqual (cmap.GetSpacing(), input_image.GetSpacing())
        cmap_data_vtk = cmap.GetCellData().GetScalars()  
        self.assertEqual (cmap_data_vtk.GetNumberOfTuples(), 8)

    def test_blank(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars(vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 0)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        self.assertTrue (alltrue(cmap_data == 0))

    def test_solid(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 1
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        self.assertTrue (alltrue(cmap_data == 1))

    def test_lower_corner_point(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[0,0,0] = 1
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[1,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_upper_corner_point(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[2,3,2] = 1
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,1]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_central_point(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[1,2,1] = 1
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,1,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_two_disconnected_points(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[1,2,1] = 1
        input_data[2,3,1] = 1
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 2)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,1,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,2,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_two_connected_points(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[1,1,1] = 1
        input_data[1,0,1] = 1
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,1,0],
                                 [0,1,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_edge_line_x(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[0,0,:] = 3
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[1,1,1],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_central_line_x(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[1,2,:] = 3
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [1,1,1],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_edge_line_z(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[:,0,0] = 3
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[1,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[1,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]],
                                [[1,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,0,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_central_line_z(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[:,2,1] = 3
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[0,0,0],
                                 [0,0,0],
                                 [0,1,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,1,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,1,0],
                                 [0,0,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_two_disconnected_lines(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[1,1,:] = 1
        input_data[:,3,1] = 3
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 2)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,1,0]],
                                [[0,0,0],
                                 [2,2,2],
                                 [0,0,0],
                                 [0,1,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,0,0],
                                 [0,1,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_two_connected_lines(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[1,1,:] = 1
        input_data[:,2,1] = 3
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[0,0,0],
                                 [0,0,0],
                                 [0,1,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [1,1,1],
                                 [0,1,0],
                                 [0,0,0]],
                                [[0,0,0],
                                 [0,0,0],
                                 [0,1,0],
                                 [0,0,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_two_disconnected_planes(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[:,0,:] = 1
        input_data[:,2,:] = 100
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 2)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[1,1,1],
                                 [0,0,0],
                                 [2,2,2],
                                 [0,0,0]],
                                [[1,1,1],
                                 [0,0,0],
                                 [2,2,2],
                                 [0,0,0]],
                                [[1,1,1],
                                 [0,0,0],
                                 [2,2,2],
                                 [0,0,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))

    def test_two_bridged_planes(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,4,3)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data = vtk_to_numpy (input_image.GetPointData().GetScalars())
        input_data.resize(3,4,3)
        input_data[:,:,:] = 0
        input_data[:,0,:] = 1
        input_data[:,2,:] = 100
        input_data[0,1,2] = 1
        input_data[2,1,0] = 1
        filter = vtkbone.vtkboneImageConnectivityMap()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual (filter.GetNumberOfRegions(), 1)
        cmap = filter.GetOutput()
        cmap_data_vtk = cmap.GetPointData().GetScalars()  
        cmap_data = vtk_to_numpy (cmap_data_vtk)
        cmap_data.resize(3,4,3)
        expected_cmap = array( [[[1,1,1],
                                 [0,0,1],
                                 [1,1,1],
                                 [0,0,0]],
                                [[1,1,1],
                                 [0,0,0],
                                 [1,1,1],
                                 [0,0,0]],
                                [[1,1,1],
                                 [1,0,0],
                                 [1,1,1],
                                 [0,0,0]]] )
        self.assertTrue (alltrue(cmap_data == expected_cmap))


if __name__ == '__main__':
    unittest.main()
