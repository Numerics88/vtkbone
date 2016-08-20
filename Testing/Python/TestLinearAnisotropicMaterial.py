from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import unittest


class TestLinearAnisotropicMaterial (unittest.TestCase):

  def test_anisotropic (self):
      material = vtkbone.vtkboneLinearAnisotropicMaterial()
      K = array((
            (1571.653,   540.033,   513.822,     7.53 ,  -121.22 ,   -57.959),
            ( 540.033,  2029.046,   469.974,    78.591,   -53.69 ,   -50.673),
            ( 513.822,   469.974,  1803.998,    20.377,   -57.014,   -15.761),
            (   7.53 ,    78.591,    20.377,   734.405,   -23.127,   -36.557),
            (-121.22 ,   -53.69 ,   -57.014,   -23.127,   627.396,    13.969),
            ( -57.959,   -50.673,   -15.761,   -36.557,    13.969,   745.749)))
      K_vtk = numpy_to_vtk (K, array_type=vtk.VTK_FLOAT)
      material.SetStressStrainMatrix(K_vtk)
      K2_vtk = vtk.vtkFloatArray()
      material.GetStressStrainMatrix (K2_vtk)
      K2 = vtk_to_numpy (K2_vtk)
      self.assertEqual (K2.shape, (6,6))
      self.assertTrue (alltrue(abs(K - K2) < 1E-3))


  def test_copy (self):
      material = vtkbone.vtkboneLinearAnisotropicMaterial()
      K = array((
            (1571.653,   540.033,   513.822,     7.53 ,  -121.22 ,   -57.959),
            ( 540.033,  2029.046,   469.974,    78.591,   -53.69 ,   -50.673),
            ( 513.822,   469.974,  1803.998,    20.377,   -57.014,   -15.761),
            (   7.53 ,    78.591,    20.377,   734.405,   -23.127,   -36.557),
            (-121.22 ,   -53.69 ,   -57.014,   -23.127,   627.396,    13.969),
            ( -57.959,   -50.673,   -15.761,   -36.557,    13.969,   745.749)))
      K_vtk = numpy_to_vtk (K, array_type=vtk.VTK_FLOAT)
      material.SetStressStrainMatrix(K_vtk)
      material2 = material.Copy()
      K2_vtk = vtk.vtkFloatArray()
      material2.GetStressStrainMatrix (K2_vtk)
      K2 = vtk_to_numpy (K2_vtk)
      self.assertEqual (K2.shape, (6,6))
      self.assertTrue (alltrue(abs(K - K2) < 1E-3))


  def test_scaled_copy (self):
      material = vtkbone.vtkboneLinearAnisotropicMaterial()
      K = array((
            (1571.653,   540.033,   513.822,     7.53 ,  -121.22 ,   -57.959),
            ( 540.033,  2029.046,   469.974,    78.591,   -53.69 ,   -50.673),
            ( 513.822,   469.974,  1803.998,    20.377,   -57.014,   -15.761),
            (   7.53 ,    78.591,    20.377,   734.405,   -23.127,   -36.557),
            (-121.22 ,   -53.69 ,   -57.014,   -23.127,   627.396,    13.969),
            ( -57.959,   -50.673,   -15.761,   -36.557,    13.969,   745.749)))
      K_vtk = numpy_to_vtk (K, array_type=vtk.VTK_FLOAT)
      material.SetStressStrainMatrix(K_vtk)
      material2 = material.ScaledCopy(0.5)
      K2_vtk = vtk.vtkFloatArray()
      material2.GetStressStrainMatrix (K2_vtk)
      K2 = vtk_to_numpy (K2_vtk)
      self.assertEqual (K2.shape, (6,6))
      self.assertTrue (alltrue(abs(K - 2*K2) < 1E-3))


if __name__ == '__main__':
    unittest.main()
