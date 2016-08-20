from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import test_geometries
import unittest


class TestConstraint (unittest.TestCase):

  def test_merge (self):
    ids1 = array((8,9,10,11))
    ids1_vtk = numpy_to_vtk(ids1, array_type=vtk.VTK_ID_TYPE)
    constraint1 = vtkbone.vtkboneConstraint()
    constraint1.SetIndices(ids1_vtk)
    senses1 = array((1,1,1,1))
    senses1_vtk = numpy_to_vtk(senses1)
    senses1_vtk.SetName("SENSE")
    constraint1.GetAttributes().AddArray(senses1_vtk)
    values1 = array((1.0,1.1,1.2,1.3))
    values1_vtk = numpy_to_vtk(values1)
    values1_vtk.SetName("VALUE")
    constraint1.GetAttributes().AddArray(values1_vtk)
    ids2 = array((1,2,3))
    ids2_vtk = numpy_to_vtk(ids2, array_type=vtk.VTK_ID_TYPE)
    constraint2 = vtkbone.vtkboneConstraint()
    constraint2.SetIndices(ids2_vtk)
    senses2 = array((2,2,2))
    senses2_vtk = numpy_to_vtk(senses2)
    senses2_vtk.SetName("SENSE")
    constraint2.GetAttributes().AddArray(senses2_vtk)
    values2 = array((2.0,2.2,2.2))
    values2_vtk = numpy_to_vtk(values2)
    values2_vtk.SetName("VALUE")
    constraint2.GetAttributes().AddArray(values2_vtk)
    return_val = constraint1.Merge(constraint2)
    self.assertEqual(return_val, 1)
    self.assertEqual(constraint1.GetNumberOfValues(), 7)
    indices_vtk = constraint1.GetIndices()
    assert(indices_vtk != None)
    indices = vtk_to_numpy(indices_vtk)
    self.assertTrue(alltrue(indices[:4] == ids1))
    self.assertTrue(alltrue(indices[4:] == ids2))
    senses_vtk = constraint1.GetAttributes().GetArray("SENSE")
    assert(senses_vtk != None)
    senses = vtk_to_numpy(senses_vtk)
    self.assertTrue(alltrue(senses[:4] == senses1))
    self.assertTrue(alltrue(senses[4:] == senses2))
    values_vtk = constraint1.GetAttributes().GetArray("VALUE")
    assert(values_vtk != None)
    values = vtk_to_numpy(values_vtk)
    self.assertTrue(alltrue(values[:4] == values1))
    self.assertTrue(alltrue(values[4:] == values2))


if __name__ == '__main__':
    unittest.main()
