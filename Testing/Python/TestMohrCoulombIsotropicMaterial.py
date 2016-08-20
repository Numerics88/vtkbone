from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
from math import atan
import unittest


class TestMohrCoulombIsotropicMaterial (unittest.TestCase):

    def test_simple (self):
        material = vtkbone.vtkboneMohrCoulombIsotropicMaterial()
        material.SetYoungsModulus(1234.5)
        material.SetPoissonsRatio(0.246)
        material.SetC(11111.1)
        material.SetPhi(0.1)
        self.assertEqual (material.GetYoungsModulus(), 1234.5)
        self.assertEqual (material.GetPoissonsRatio(), 0.246)
        self.assertEqual (material.GetC(), 11111.1)
        self.assertEqual (material.GetPhi(), 0.1)

    def test_set_yield_strengths (self):
        material = vtkbone.vtkboneMohrCoulombIsotropicMaterial()
        material.SetYoungsModulus(1234.5)
        material.SetPoissonsRatio(0.246)
        YT = 5000
        YC = 7000
        material.SetYieldStrengths(YT,YC)
        c = sqrt(YT*YC)/2
        phi = pi/2 - 2*atan(sqrt(YT/YC))
        self.assertEqual (material.GetYoungsModulus(), 1234.5)
        self.assertEqual (material.GetPoissonsRatio(), 0.246)
        self.assertAlmostEqual (material.GetC(), c, delta=1E-6)
        self.assertAlmostEqual (material.GetPhi(), phi, delta=1E-10)

    def test_copy (self):
        material = vtkbone.vtkboneMohrCoulombIsotropicMaterial()
        material.SetYoungsModulus(1234.5)
        material.SetPoissonsRatio(0.246)
        material.SetC(11111.1)
        material.SetPhi(0.1)
        scaled_material = material.Copy()
        self.assertEqual (scaled_material.GetYoungsModulus(), 1234.5)
        self.assertEqual (scaled_material.GetPoissonsRatio(), 0.246)
        self.assertEqual (scaled_material.GetC(), 11111.1)
        self.assertEqual (scaled_material.GetPhi(), 0.1)


    def test_scaled_copy (self):
        material = vtkbone.vtkboneMohrCoulombIsotropicMaterial()
        material.SetYoungsModulus(1234.5)
        material.SetPoissonsRatio(0.246)
        material.SetC(11111.1)
        material.SetPhi(0.1)
        scaled_material = material.ScaledCopy(0.5)
        self.assertEqual (material.GetYoungsModulus(), 1234.5)
        self.assertEqual (material.GetPoissonsRatio(), 0.246)
        self.assertEqual (material.GetC(), 11111.1)
        self.assertEqual (material.GetPhi(), 0.1)
        self.assertEqual (scaled_material.GetYoungsModulus(), 0.5*1234.5)
        self.assertEqual (scaled_material.GetPoissonsRatio(), 0.246)
        self.assertEqual (scaled_material.GetC(), 0.5*11111.1)
        self.assertEqual (scaled_material.GetPhi(), 0.1)


if __name__ == '__main__':
    unittest.main()
