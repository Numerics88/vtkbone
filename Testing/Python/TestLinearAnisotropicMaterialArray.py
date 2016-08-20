from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import unittest


class TestLinearAnisotropicMaterialArray (unittest.TestCase):

  def test_simple (self):
      K = array((
            (1571.653,   540.033,   513.822,     7.53 ,  -121.22 ,   -57.959),
            ( 540.033,  2029.046,   469.974,    78.591,   -53.69 ,   -50.673),
            ( 513.822,   469.974,  1803.998,    20.377,   -57.014,   -15.761),
            (   7.53 ,    78.591,    20.377,   734.405,   -23.127,   -36.557),
            (-121.22 ,   -53.69 ,   -57.014,   -23.127,   627.396,    13.969),
            ( -57.959,   -50.673,   -15.761,   -36.557,    13.969,   745.749)))
      Kut = zeros((2,21),float)
      Kut[0, 0]    = K[0,0]
      Kut[0, 1:3]  = K[1,:2]
      Kut[0, 3:6]  = K[2,:3]
      Kut[0, 6:10] = K[3,:4]
      Kut[0,10:15] = K[4,:5]
      Kut[0,15:]   = K[5]
      Kut[1] = 0.5*Kut[0]
      Kut_vtk = numpy_to_vtk(Kut, array_type=vtk.VTK_FLOAT)
      material = vtkbone.vtkboneLinearAnisotropicMaterialArray()
      material.SetStiffnessMatrixUpperTriangular(Kut_vtk)
      self.assertEqual (material.GetSize(), 2)
      Ka_vtk = material.GetStiffnessMatrixUpperTriangular()
      Ka = vtk_to_numpy (Ka_vtk)
      self.assertEqual (Ka.shape, (2,21))
      self.assertTrue (alltrue(abs(Kut - Ka) < 1E-3))


  def test_set_item (self):
      K1 = array((
            (1571.653,   540.033,   513.822,     7.53 ,  -121.22 ,   -57.959),
            ( 540.033,  2029.046,   469.974,    78.591,   -53.69 ,   -50.673),
            ( 513.822,   469.974,  1803.998,    20.377,   -57.014,   -15.761),
            (   7.53 ,    78.591,    20.377,   734.405,   -23.127,   -36.557),
            (-121.22 ,   -53.69 ,   -57.014,   -23.127,   627.396,    13.969),
            ( -57.959,   -50.673,   -15.761,   -36.557,    13.969,   745.749)))
      Kut1 = zeros(21,float)
      Kut1[ 0]    = K1[0,0]
      Kut1[ 1:3]  = K1[1,:2]
      Kut1[ 3:6]  = K1[2,:3]
      Kut1[ 6:10] = K1[3,:4]
      Kut1[10:15] = K1[4,:5]
      Kut1[15:]   = K1[5]
      K2 = 0.5*K1
      Kut2 = 0.5*Kut1
      material = vtkbone.vtkboneLinearAnisotropicMaterialArray()
      material.Resize(2)
      mat1 = vtkbone.vtkboneLinearAnisotropicMaterial()
      K1_vtk = numpy_to_vtk(K1, array_type=vtk.VTK_FLOAT)
      mat1.SetStiffnessMatrix(K1_vtk)
      material.SetItem(0, mat1)
      mat2 = vtkbone.vtkboneLinearAnisotropicMaterial()
      K2_vtk = numpy_to_vtk(K2, array_type=vtk.VTK_FLOAT)
      mat2.SetStiffnessMatrix(K2_vtk)
      material.SetItem(1, mat2)
      Ka_vtk = material.GetStiffnessMatrixUpperTriangular()
      Ka = vtk_to_numpy (Ka_vtk)
      self.assertEqual (Ka.shape, (2,21))
      self.assertTrue (alltrue(abs(Ka[0] - Kut1) < 1E-3))
      self.assertTrue (alltrue(abs(Ka[1] - Kut2) < 1E-3))


  def test_set_scaled_item (self):
      K1 = array((
            (1571.653,   540.033,   513.822,     7.53 ,  -121.22 ,   -57.959),
            ( 540.033,  2029.046,   469.974,    78.591,   -53.69 ,   -50.673),
            ( 513.822,   469.974,  1803.998,    20.377,   -57.014,   -15.761),
            (   7.53 ,    78.591,    20.377,   734.405,   -23.127,   -36.557),
            (-121.22 ,   -53.69 ,   -57.014,   -23.127,   627.396,    13.969),
            ( -57.959,   -50.673,   -15.761,   -36.557,    13.969,   745.749)))
      Kut1 = zeros(21,float)
      Kut1[ 0]    = K1[0,0]
      Kut1[ 1:3]  = K1[1,:2]
      Kut1[ 3:6]  = K1[2,:3]
      Kut1[ 6:10] = K1[3,:4]
      Kut1[10:15] = K1[4,:5]
      Kut1[15:]   = K1[5]
      K2 = 0.5*K1
      Kut2 = 0.5*Kut1
      material = vtkbone.vtkboneLinearAnisotropicMaterialArray()
      material.Resize(2)
      mat1 = vtkbone.vtkboneLinearAnisotropicMaterial()
      K1_vtk = numpy_to_vtk(K1, array_type=vtk.VTK_FLOAT)
      mat1.SetStiffnessMatrix(K1_vtk)
      material.SetScaledItem(0, mat1, 1.5)
      mat2 = vtkbone.vtkboneLinearAnisotropicMaterial()
      K2_vtk = numpy_to_vtk(K2, array_type=vtk.VTK_FLOAT)
      mat2.SetStiffnessMatrix(K2_vtk)
      material.SetScaledItem(1, mat2, 2.5)
      Ka_vtk = material.GetStiffnessMatrixUpperTriangular()
      Ka = vtk_to_numpy (Ka_vtk)
      self.assertEqual (Ka.shape, (2,21))
      self.assertTrue (alltrue(abs(Ka[0] - 1.5*Kut1) < 1E-3))
      self.assertTrue (alltrue(abs(Ka[1] - 2.5*Kut2) < 1E-3))


if __name__ == '__main__':
    unittest.main()
