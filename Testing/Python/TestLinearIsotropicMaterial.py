from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import unittest


class TestLinearIsotropicMaterial (unittest.TestCase):

    def test_simple (self):
        material = vtkbone.vtkboneLinearIsotropicMaterial()
        material.SetYoungsModulus(1234.5)
        material.SetPoissonsRatio(0.246)
        self.assertEqual (material.GetYoungsModulus(), 1234.5)
        self.assertEqual (material.GetPoissonsRatio(), 0.246)


    def test_copy (self):
        material = vtkbone.vtkboneLinearIsotropicMaterial()
        material.SetYoungsModulus(1234.5)
        material.SetPoissonsRatio(0.246)
        scaled_material = material.Copy()
        self.assertEqual (scaled_material.GetYoungsModulus(), 1234.5)
        self.assertEqual (scaled_material.GetPoissonsRatio(), 0.246)


    def test_scaled_copy (self):
        material = vtkbone.vtkboneLinearIsotropicMaterial()
        material.SetYoungsModulus(1234.5)
        material.SetPoissonsRatio(0.246)
        scaled_material = material.ScaledCopy(0.5)
        self.assertEqual (material.GetYoungsModulus(), 1234.5)
        self.assertEqual (material.GetPoissonsRatio(), 0.246)
        self.assertEqual (scaled_material.GetYoungsModulus(), 0.5*1234.5)
        self.assertEqual (scaled_material.GetPoissonsRatio(), 0.246)


if __name__ == '__main__':
    unittest.main()
