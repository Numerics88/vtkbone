from __future__ import division
import sys
import numpy
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone
import traceback
import unittest


class TestLinearOrthotropicMaterial (unittest.TestCase):

    def test_isotropic (self):

        material = vtkbone.vtkboneLinearOrthotropicMaterial()
        material.SetYoungsModulusX(1234.5)
        material.SetYoungsModulusY(1234.5)
        material.SetYoungsModulusZ(1234.5)
        material.SetPoissonsRatioYZ(0.246)
        material.SetPoissonsRatioZX(0.246)
        material.SetPoissonsRatioXY(0.246)
        G = 1234.5/(2*(1+0.246))
        material.SetShearModulusYZ(G)
        material.SetShearModulusZX(G)
        material.SetShearModulusXY(G)

        self.assertEqual (material.GetYoungsModulusX(), 1234.5)
        self.assertEqual (material.GetYoungsModulusY(), 1234.5)
        self.assertEqual (material.GetYoungsModulusZ(), 1234.5)
        self.assertEqual (material.GetPoissonsRatioYZ(), 0.246)
        self.assertEqual (material.GetPoissonsRatioZY(), 0.246)
        self.assertEqual (material.GetPoissonsRatioZX(), 0.246)
        self.assertEqual (material.GetPoissonsRatioXZ(), 0.246)
        self.assertEqual (material.GetPoissonsRatioXY(), 0.246)
        self.assertEqual (material.GetPoissonsRatioYX(), 0.246)
        self.assertEqual (material.GetShearModulusYZ(), G)
        self.assertEqual (material.GetShearModulusZY(), G)
        self.assertEqual (material.GetShearModulusZX(), G)
        self.assertEqual (material.GetShearModulusXZ(), G)
        self.assertEqual (material.GetShearModulusXY(), G)
        self.assertEqual (material.GetShearModulusYX(), G)


    def test_orthotropic (self):

        material = vtkbone.vtkboneLinearOrthotropicMaterial()
        material.SetYoungsModulusX(1000)
        material.SetYoungsModulusY(1100)
        material.SetYoungsModulusZ(1200)
        material.SetPoissonsRatioYZ(0.25)
        material.SetPoissonsRatioZX(0.3)
        material.SetPoissonsRatioXY(0.2)
        # These values are not necessarily consistent
        GYZ = 1000/(2*(1+0.25))
        GZX = 1100/(2*(1+0.3))
        GXY = 1200/(2*(1+0.2))
        material.SetShearModulusYZ(GYZ)
        material.SetShearModulusZX(GZX)
        material.SetShearModulusXY(GXY)

        self.assertEqual (material.GetYoungsModulusX(), 1000)
        self.assertEqual (material.GetYoungsModulusY(), 1100)
        self.assertEqual (material.GetYoungsModulusZ(), 1200)
        self.assertEqual (material.GetPoissonsRatioYZ(), 0.25)
        self.assertEqual (material.GetPoissonsRatioZX(), 0.3)    
        self.assertEqual (material.GetPoissonsRatioXY(), 0.2)
        self.assertAlmostEqual (material.GetPoissonsRatioYZ() / material.GetYoungsModulusY(), material.GetPoissonsRatioZY() / material.GetYoungsModulusZ(), delta=1E-8)
        self.assertAlmostEqual(material.GetPoissonsRatioZX() / material.GetYoungsModulusZ(), material.GetPoissonsRatioXZ() / material.GetYoungsModulusX(), delta=1E-8 )
        self.assertAlmostEqual (material.GetPoissonsRatioXY() / material.GetYoungsModulusX(), material.GetPoissonsRatioYX() / material.GetYoungsModulusY(), delta=1E-8)
        self.assertEqual (material.GetShearModulusYZ(), GYZ)
        self.assertEqual (material.GetShearModulusZY(), GYZ)
        self.assertEqual (material.GetShearModulusZX(), GZX)
        self.assertEqual (material.GetShearModulusXZ(), GZX)
        self.assertEqual (material.GetShearModulusXY(), GXY)
        self.assertEqual (material.GetShearModulusYX(), GXY)


    def test_copy (self):
        
        material = vtkbone.vtkboneLinearOrthotropicMaterial()
        material.SetYoungsModulusX(1000)
        material.SetYoungsModulusY(1100)
        material.SetYoungsModulusZ(1200)
        material.SetPoissonsRatioYZ(0.25)
        material.SetPoissonsRatioZX(0.3)
        material.SetPoissonsRatioXY(0.2)
        # These values are not necessarily consistent
        GYZ = 1000/(2*(1+0.25))
        GZX = 1100/(2*(1+0.3))
        GXY = 1200/(2*(1+0.2))
        material.SetShearModulusYZ(GYZ)
        material.SetShearModulusZX(GZX)
        material.SetShearModulusXY(GXY)
        
        scaled_material = material.Copy()

        self.assertEqual (scaled_material.GetYoungsModulusX(), 1000)
        self.assertEqual (scaled_material.GetYoungsModulusY(), 1100)
        self.assertEqual (scaled_material.GetYoungsModulusZ(), 1200)
        self.assertEqual (scaled_material.GetPoissonsRatioYZ(), 0.25)
        self.assertEqual (scaled_material.GetPoissonsRatioZX(), 0.3)    
        self.assertEqual (scaled_material.GetPoissonsRatioXY(), 0.2)
        self.assertAlmostEqual (scaled_material.GetPoissonsRatioYZ() / scaled_material.GetYoungsModulusY(), scaled_material.GetPoissonsRatioZY() / scaled_material.GetYoungsModulusZ(), delta=1E-8)
        self.assertAlmostEqual (scaled_material.GetPoissonsRatioZX() / scaled_material.GetYoungsModulusZ(), scaled_material.GetPoissonsRatioXZ() / scaled_material.GetYoungsModulusX(), delta=1E-8)
        self.assertAlmostEqual (scaled_material.GetPoissonsRatioXY() / scaled_material.GetYoungsModulusX(), scaled_material.GetPoissonsRatioYX() / scaled_material.GetYoungsModulusY(), delta=1E-8)
        self.assertEqual (scaled_material.GetShearModulusYZ(), GYZ)
        self.assertEqual (scaled_material.GetShearModulusZY(), GYZ)
        self.assertEqual (scaled_material.GetShearModulusZX(), GZX)
        self.assertEqual (scaled_material.GetShearModulusXZ(), GZX)
        self.assertEqual (scaled_material.GetShearModulusXY(), GXY)
        self.assertEqual (scaled_material.GetShearModulusYX(), GXY)


    def test_scaled_copy (self):
        
        material = vtkbone.vtkboneLinearOrthotropicMaterial()
        material.SetYoungsModulusX(1000)
        material.SetYoungsModulusY(1100)
        material.SetYoungsModulusZ(1200)
        material.SetPoissonsRatioYZ(0.25)
        material.SetPoissonsRatioZX(0.3)
        material.SetPoissonsRatioXY(0.2)
        # These values are not necessarily consistent
        GYZ = 1000/(2*(1+0.25))
        GZX = 1100/(2*(1+0.3))
        GXY = 1200/(2*(1+0.2))
        material.SetShearModulusYZ(GYZ)
        material.SetShearModulusZX(GZX)
        material.SetShearModulusXY(GXY)
        
        scaled_material = material.ScaledCopy(0.5)

        self.assertEqual (scaled_material.GetYoungsModulusX(), 0.5*1000)
        self.assertEqual (scaled_material.GetYoungsModulusY(), 0.5*1100)
        self.assertEqual (scaled_material.GetYoungsModulusZ(), 0.5*1200)
        self.assertEqual (scaled_material.GetPoissonsRatioYZ(), 0.25)
        self.assertEqual (scaled_material.GetPoissonsRatioZX(), 0.3)    
        self.assertEqual (scaled_material.GetPoissonsRatioXY(), 0.2)
        self.assertAlmostEqual (scaled_material.GetPoissonsRatioYZ() / scaled_material.GetYoungsModulusY(), scaled_material.GetPoissonsRatioZY() / scaled_material.GetYoungsModulusZ(), delta=1E-8)
        self.assertAlmostEqual (scaled_material.GetPoissonsRatioZX() / scaled_material.GetYoungsModulusZ(), scaled_material.GetPoissonsRatioXZ() / scaled_material.GetYoungsModulusX(), delta=1E-8)
        self.assertAlmostEqual (scaled_material.GetPoissonsRatioXY() / scaled_material.GetYoungsModulusX(), scaled_material.GetPoissonsRatioYX() / scaled_material.GetYoungsModulusY(), delta=1E-8)
        self.assertEqual (scaled_material.GetShearModulusYZ(), 0.5*GYZ)
        self.assertEqual (scaled_material.GetShearModulusZY(), 0.5*GYZ)
        self.assertEqual (scaled_material.GetShearModulusZX(), 0.5*GZX)
        self.assertEqual (scaled_material.GetShearModulusXZ(), 0.5*GZX)
        self.assertEqual (scaled_material.GetShearModulusXY(), 0.5*GXY)
        self.assertEqual (scaled_material.GetShearModulusYX(), 0.5*GXY)


if __name__ == '__main__':
    unittest.main()
