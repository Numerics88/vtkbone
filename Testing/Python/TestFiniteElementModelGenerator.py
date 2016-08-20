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


class TestFiniteElementModelGenerator (unittest.TestCase):

    def test_BasicGeneration(self):
        """Make sure that the components output of the output are the same
        objects that were passed in.
        Note: the operator __this__ gives a string representation that includes
              the address of the object.  It will only match if the objects are
              indeed one and the same.
        """
        material_generator = vtkbone.vtkboneGenerateHomogeneousMaterialTable()
        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneFiniteElementModelGenerator()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputConnection(1, material_generator.GetOutputPort())
        model_generator.Update()
        model = model_generator.GetOutput()
        self.assertEqual(model.GetPoints().__this__, geometry.GetPoints().__this__)
        self.assertEqual(model.GetCells().__this__, geometry.GetCells().__this__)
        self.assertEqual(model.GetMaterialTable().__this__, material_generator.GetOutput().__this__)


    def test_PedigreeIds(self):
        """Make sure that generated vtkboneFiniteElementModel objects have
        PedigreeId arrays by default.
        """
        material_generator = vtkbone.vtkboneGenerateHomogeneousMaterialTable()
        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneFiniteElementModelGenerator()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputConnection(1, material_generator.GetOutputPort())
        model_generator.Update()
        model = model_generator.GetOutput()
        pointPedigreeIds_vtk = model.GetPointData().GetPedigreeIds()
        self.assertFalse(pointPedigreeIds_vtk is None)
        pointPedigreeIds = vtk_to_numpy(pointPedigreeIds_vtk)
        expected_pointPedigreeIds = arange(12)
        self.assertTrue(alltrue(pointPedigreeIds == expected_pointPedigreeIds))
        cellPedigreeIds_vtk = model.GetCellData().GetPedigreeIds()
        self.assertFalse(cellPedigreeIds_vtk is None)
        cellPedigreeIds = vtk_to_numpy(cellPedigreeIds_vtk)
        expected_cellPedigreeIds = arange(2)
        self.assertTrue(alltrue(cellPedigreeIds == expected_cellPedigreeIds))


    def test_No_PedigreeIds(self):
        """Make sure that generated vtkboneFiniteElementModel objects do not have
        PedigreeId arrays if so specified.
        """
        material_generator = vtkbone.vtkboneGenerateHomogeneousMaterialTable()
        geometry = test_geometries.generate_two_element_geometry()
        model_generator = vtkbone.vtkboneFiniteElementModelGenerator()
        model_generator.SetInputData(0, geometry)
        model_generator.SetInputConnection(1, material_generator.GetOutputPort())
        model_generator.AddPedigreeIdArraysOff()
        model_generator.Update()
        model = model_generator.GetOutput()
        pointPedigreeIds_vtk = model.GetPointData().GetPedigreeIds()
        self.assertTrue(pointPedigreeIds_vtk is None)
        cellPedigreeIds_vtk = model.GetCellData().GetPedigreeIds()
        self.assertTrue(cellPedigreeIds_vtk is None)
    

if __name__ == '__main__':
    unittest.main()
