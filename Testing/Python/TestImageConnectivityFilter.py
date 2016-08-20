from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import unittest


def generate_test_image():
    regions = []
    # regions[0] has volume 60
    regions.append (zeros((20,20,20), int))
    regions[0][9:12,13:18,10:14] = 42
    # region[1] has volume 36
    regions.append (zeros((20,20,20), int))
    regions[1][2:5,14:17,2:6] = 142
    # regions[2] has volume 8
    regions.append (zeros((20,20,20), int))
    regions[2][16:18,10:12,10:12] = 33
    # regions[3] has volume 1
    regions.append (zeros((20,20,20), int))
    regions[3][17,4,8] = 1
    input_image = vtk.vtkImageData()
    input_image.SetDimensions (20,20,20)
    input_image.SetOrigin (0.2,-1.0,1.5)
    input_image.SetSpacing (2,4,5.5)
    input_image.AllocateScalars (vtk.VTK_INT, 1)
    input_data_vtk = input_image.GetPointData().GetScalars()
    input_data = vtk_to_numpy (input_data_vtk)
    input_data.resize((20,20,20))
    input_data[:,:,:] = 0
    for r in regions:
        input_data += r
    return regions, input_image


class TestImageConnectivityFilter (unittest.TestCase):

    def test_data_on_points(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,3,3)
        input_image.SetOrigin (0.2,-1.0,1.5)
        input_image.SetSpacing (2,4,5.5)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data_vtk = input_image.GetPointData().GetScalars()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.Update()
        output_image = filter.GetOutput()
        self.assertEqual(output_image.GetExtent(), input_image.GetExtent())
        self.assertEqual(output_image.GetOrigin(), input_image.GetOrigin())
        self.assertEqual(output_image.GetSpacing(), input_image.GetSpacing())
        output_data_vtk = output_image.GetPointData().GetScalars()  
        self.assertEqual(output_data_vtk.GetDataType(), input_data_vtk.GetDataType())
        self.assertEqual(output_data_vtk.GetNumberOfTuples(), 27)

    def test_data_on_cells(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,3,3)
        input_image.SetOrigin (0.2,-1.0,1.5)
        input_image.SetSpacing (2,4,5.5)
        input_data_vtk = vtk.vtkShortArray()
        input_data_vtk.SetNumberOfTuples(8)
        input_image.GetCellData().SetScalars (input_data_vtk)
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.Update()
        output_image = filter.GetOutput()
        self.assertEqual(output_image.GetExtent(), input_image.GetExtent())
        self.assertEqual(output_image.GetOrigin(), input_image.GetOrigin())
        self.assertEqual(output_image.GetSpacing(), input_image.GetSpacing())
        output_data_vtk = output_image.GetCellData().GetScalars()  
        self.assertEqual(output_data_vtk.GetDataType(), input_data_vtk.GetDataType())
        self.assertEqual(output_data_vtk.GetNumberOfTuples(), 8)

    def test_blank(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,3,3)
        input_image.SetOrigin (0.2,-1.0,1.5)
        input_image.SetSpacing (2,4,5.5)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data_vtk = input_image.GetPointData().GetScalars()
        input_data = vtk_to_numpy (input_data_vtk)
        input_data.resize((3,3,3))
        input_data[:,:,:] = 0
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.Update()
        output_image = filter.GetOutput()
        self.assertEqual(output_image.GetDimensions(), (3,3,3))
        output_data_vtk = output_image.GetPointData().GetScalars()
        self.assertEqual(output_data_vtk.GetNumberOfTuples(), 27)
        output_data = vtk_to_numpy (output_data_vtk)
        self.assertTrue (alltrue (output_data == 0))

    def test_solid(self):
        input_image = vtk.vtkImageData()
        input_image.SetDimensions (3,3,3)
        input_image.SetOrigin (0.2,-1.0,1.5)
        input_image.SetSpacing (2,4,5.5)
        input_image.AllocateScalars (vtk.VTK_SHORT, 1)
        input_data_vtk = input_image.GetPointData().GetScalars()
        input_data = vtk_to_numpy (input_data_vtk)
        input_data.resize((3,3,3))
        input_data[:,:,:] = 42
        input_data[1,:,:] = 142
        input_data[:,2,:] = 242
        input_data[:,:,0] = 342
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.Update()
        output_image = filter.GetOutput()
        self.assertEqual(output_image.GetDimensions(), (3,3,3))
        output_data_vtk = output_image.GetPointData().GetScalars()
        self.assertEqual(output_data_vtk.GetNumberOfTuples(), 27)
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize (output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == input_data))

    def test_largest_region(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 1)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize(output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == regions[0]))

    def test_all_regions(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.SetExtractionMode (vtkbone.vtkboneImageConnectivityFilter.EXTRACT_ALL_REGIONS)
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 4)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize(output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == regions[0] + regions[1] + regions[2] + regions[3]))

    def test_minimum_size_1(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.SetExtractionMode (vtkbone.vtkboneImageConnectivityFilter.EXTRACT_REGIONS_OF_SPECIFIED_SIZE)
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 4)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize(output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == regions[0] + regions[1] + regions[2] + regions[3]))

    def test_minimum_size_36(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.SetExtractionMode (vtkbone.vtkboneImageConnectivityFilter.EXTRACT_REGIONS_OF_SPECIFIED_SIZE)
        filter.SetMinimumRegionSize (36)
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 2)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize(output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == regions[0] + regions[1]))

    def test_minimum_size_1000(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.SetExtractionMode (vtkbone.vtkboneImageConnectivityFilter.EXTRACT_REGIONS_OF_SPECIFIED_SIZE)
        filter.SetMinimumRegionSize (1000)
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 0)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        self.assertTrue (alltrue (output_data == 0))

    def test_closest_point_origin(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.SetExtractionMode (vtkbone.vtkboneImageConnectivityFilter.EXTRACT_CLOSEST_POINT_REGION)
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 1)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize(output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == regions[1]))

    def test_closest_point_far_corner(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.SetExtractionMode (vtkbone.vtkboneImageConnectivityFilter.EXTRACT_CLOSEST_POINT_REGION)
        coord = array ((19.5,19.5,19.5))
        origin = array ((0.2,-1.0,1.5))
        spacing = array ((2,4,5.5))
        filter.SetClosestPoint (origin + (coord + 0.5) * spacing);
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 1)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize(output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == regions[2]))

    def test_closest_point_in_region_2(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.SetExtractionMode (vtkbone.vtkboneImageConnectivityFilter.EXTRACT_CLOSEST_POINT_REGION)
        coord = array ((11,11,17))
        origin = array ((0.2,-1.0,1.5))
        spacing = array ((2,4,5.5))
        filter.SetClosestPoint (origin + (coord + 0.5) * spacing);
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 1)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize(output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == regions[2]))

    def test_closest_point_near_region_2_A(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.SetExtractionMode (vtkbone.vtkboneImageConnectivityFilter.EXTRACT_CLOSEST_POINT_REGION)
        coord = array ((8.5,8.5,14.5))
        origin = array ((0.2,-1.0,1.5))
        spacing = array ((2,4,5.5))
        filter.SetClosestPoint (origin + (coord + 0.5) * spacing);
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 1)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize(output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == regions[2]))

    def test_closest_point_near_region_2_B(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.SetExtractionMode (vtkbone.vtkboneImageConnectivityFilter.EXTRACT_CLOSEST_POINT_REGION)
        coord = array ((13,13,20))
        origin = array ((0.2,-1.0,1.5))
        spacing = array ((2,4,5.5))
        filter.SetClosestPoint (origin + (coord + 0.5) * spacing);
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 1)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize(output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == regions[2]))

    def test_seeded(self):
        regions, input_image = generate_test_image()
        filter = vtkbone.vtkboneImageConnectivityFilter()
        filter.SetInputData (input_image)
        filter.SetExtractionMode (vtkbone.vtkboneImageConnectivityFilter.EXTRACT_SEEDED_REGIONS)
        coord = array ((1000,1000,1000))  # way out of region
        filter.AddSeed ((20*coord[2] + coord[1])*20+coord[0])
        coord = array ((8,4,17))  # in region 3
        filter.AddSeed ((20*coord[2] + coord[1])*20+coord[0])
        coord = array ((1,1,1))   # in no region
        filter.AddSeed ((20*coord[2] + coord[1])*20+coord[0])
        coord = array ((11,13,11))   # in region 0
        filter.AddSeed ((20*coord[2] + coord[1])*20+coord[0])
        coord = array ((11,12,11))   # in region 0
        filter.AddSeed ((20*coord[2] + coord[1])*20+coord[0])
        filter.Update()
        self.assertEqual(filter.GetNumberOfExtractedRegions(), 2)
        output_image = filter.GetOutput()
        output_data_vtk = output_image.GetPointData().GetScalars()
        output_data = vtk_to_numpy (output_data_vtk)
        output_data.resize(output_image.GetDimensions())
        self.assertTrue (alltrue (output_data == regions[0] + regions[3]))


if __name__ == '__main__':
    unittest.main()
