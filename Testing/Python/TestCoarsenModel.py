from __future__ import division

import unittest
from numpy.core import *
import numpy
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone


class TestCoarsenModel (unittest.TestCase):

  def test_iso_x_gradient (self):
	# Create 5x5x5 cube image
    cellmap = arange (1, (5*5*5)+1, dtype=int16)
    cellmap.shape = (5,5,5)
    cellmap_flat = cellmap.flatten().copy()
    cellmap_vtk = numpy_to_vtk(cellmap_flat, deep=1)
    image = vtk.vtkImageData()
    image.SetDimensions((6,6,6))     # x,y,z order
    image.SetSpacing(1.5,1.5,1.5)
    image.SetOrigin(3.5,4.5,5.5)
    image.GetCellData().SetScalars(cellmap_vtk)

    # Convert to mesh
    geometry_generator = vtkbone.vtkboneImageToMesh()
    geometry_generator.SetInputData(image)
    geometry_generator.Update()
    geometry = geometry_generator.GetOutput()

    # Generate materials with gradient in E along x.
    E = zeros((5,5,5),float32)
    E[:,:,:] = arange(1.0,6.0,dtype=float32)
    E_vtk = numpy_to_vtk (E.flatten(), deep=1)
    nu = 0.3*ones(125,float32)
    nu_vtk = numpy_to_vtk (nu, deep=1)
    material = vtkbone.vtkboneLinearIsotropicMaterialArray()
    material.SetYoungsModulus(E_vtk)
    material.SetPoissonsRatio(nu_vtk)
    self.assertEqual (material.GetSize(), 125)
    material_table = vtkbone.vtkboneMaterialTable()
    material_table.AddMaterial (1, material)

    # Generate model
    generator = vtkbone.vtkboneApplyCompressionTest()
    generator.SetInputData(0, geometry)
    generator.SetInputData(1, material_table)
    generator.Update()
    model = generator.GetOutput()

    # Apply coarsener
    coarsener = vtkbone.vtkboneCoarsenModel()
    coarsener.SetInputData (model)
    coarsener.Update()
    coarse_model = coarsener.GetOutput()

    bounds = coarse_model.GetBounds()
    self.assertAlmostEqual (bounds[0], 3.5)
    self.assertAlmostEqual (bounds[1], 12.5)
    self.assertAlmostEqual (bounds[2], 4.5)
    self.assertAlmostEqual (bounds[3], 13.5)
    self.assertAlmostEqual (bounds[4], 5.5)
    self.assertAlmostEqual (bounds[5], 14.5)

    coarse_material = coarse_model.GetMaterialTable().GetMaterial(1)
    self.assertTrue (isinstance (coarse_material, vtkbone.vtkboneLinearIsotropicMaterialArray))
    self.assertEqual (coarse_material.GetSize(), 3**3)
    coarse_E = vtk_to_numpy (coarse_material.GetYoungsModulus())
    coarse_E.shape = (3,3,3)
    self.assertTrue (alltrue(abs(coarse_E[:,:,0] - 1.5) < 1E-6))
    self.assertTrue (alltrue(abs(coarse_E[:,:,1] - 3.5) < 1E-6))
    self.assertTrue (alltrue(abs(coarse_E[:,:,2] - 5) < 1E-6))


  def test_iso_y_gradient (self):
	# Create 5x5x5 cube image
    cellmap = arange (1, (5*5*5)+1, dtype=int16)
    cellmap.shape = (5,5,5)
    cellmap_flat = cellmap.flatten().copy()
    cellmap_vtk = numpy_to_vtk(cellmap_flat, deep=1)
    image = vtk.vtkImageData()
    image.SetDimensions((6,6,6))     # x,y,z order
    image.SetSpacing(1.5,1.5,1.5)
    image.SetOrigin(3.5,4.5,5.5)
    image.GetCellData().SetScalars(cellmap_vtk)

    # Convert to mesh
    geometry_generator = vtkbone.vtkboneImageToMesh()
    geometry_generator.SetInputData(image)
    geometry_generator.Update()
    geometry = geometry_generator.GetOutput()

    # Generate materials with gradient in E along y.
    E = zeros((5,5,5),float32)
    k,j,i = numpy.mgrid[0:5,0:5,0:5]
    E[k,j,i] = 1+j
    E_vtk = numpy_to_vtk (E.flatten(), deep=1)
    nu = 0.3*ones(125,float32)
    nu_vtk = numpy_to_vtk (nu, deep=1)
    material = vtkbone.vtkboneLinearIsotropicMaterialArray()
    material.SetYoungsModulus(E_vtk)
    material.SetPoissonsRatio(nu_vtk)
    self.assertEqual (material.GetSize(), 125)
    material_table = vtkbone.vtkboneMaterialTable()
    material_table.AddMaterial (1, material)

    # Generate model
    generator = vtkbone.vtkboneApplyCompressionTest()
    generator.SetInputData(0, geometry)
    generator.SetInputData(1, material_table)
    generator.Update()
    model = generator.GetOutput()

    # Apply coarsener
    coarsener = vtkbone.vtkboneCoarsenModel()
    coarsener.SetInputData (model)
    coarsener.Update()
    coarse_model = coarsener.GetOutput()

    bounds = coarse_model.GetBounds()
    self.assertAlmostEqual (bounds[0], 3.5)
    self.assertAlmostEqual (bounds[1], 12.5)
    self.assertAlmostEqual (bounds[2], 4.5)
    self.assertAlmostEqual (bounds[3], 13.5)
    self.assertAlmostEqual (bounds[4], 5.5)
    self.assertAlmostEqual (bounds[5], 14.5)

    coarse_material = coarse_model.GetMaterialTable().GetMaterial(1)
    self.assertTrue (isinstance (coarse_material, vtkbone.vtkboneLinearIsotropicMaterialArray))
    self.assertEqual (coarse_material.GetSize(), 3**3)
    coarse_E = vtk_to_numpy (coarse_material.GetYoungsModulus())
    coarse_E.shape = (3,3,3)
    self.assertTrue (alltrue(abs(coarse_E[:,0,:] - 1.5) < 1E-6))
    self.assertTrue (alltrue(abs(coarse_E[:,1,:] - 3.5) < 1E-6))
    self.assertTrue (alltrue(abs(coarse_E[:,2,:] - 5) < 1E-6))


  def test_iso_z_gradient (self):
	# Create 5x5x5 cube image
    cellmap = arange (1, (5*5*5)+1, dtype=int16)
    cellmap.shape = (5,5,5)
    cellmap_flat = cellmap.flatten().copy()
    cellmap_vtk = numpy_to_vtk(cellmap_flat, deep=1)
    image = vtk.vtkImageData()
    image.SetDimensions((6,6,6))     # x,y,z order
    image.SetSpacing(1.5,1.5,1.5)
    image.SetOrigin(3.5,4.5,5.5)
    image.GetCellData().SetScalars(cellmap_vtk)

    # Convert to mesh
    geometry_generator = vtkbone.vtkboneImageToMesh()
    geometry_generator.SetInputData(image)
    geometry_generator.Update()
    geometry = geometry_generator.GetOutput()

    # Generate materials with gradient in E along y.
    E = zeros((5,5,5),float32)
    k,j,i = numpy.mgrid[0:5,0:5,0:5]
    E[k,j,i] = 1+k
    E_vtk = numpy_to_vtk (E.flatten(), deep=1)
    nu = 0.3*ones(125,float32)
    nu_vtk = numpy_to_vtk (nu, deep=1)
    material = vtkbone.vtkboneLinearIsotropicMaterialArray()
    material.SetYoungsModulus(E_vtk)
    material.SetPoissonsRatio(nu_vtk)
    self.assertEqual (material.GetSize(), 125)
    material_table = vtkbone.vtkboneMaterialTable()
    material_table.AddMaterial (1, material)

    # Generate model
    generator = vtkbone.vtkboneApplyCompressionTest()
    generator.SetInputData(0, geometry)
    generator.SetInputData(1, material_table)
    generator.Update()
    model = generator.GetOutput()

    # Apply coarsener
    coarsener = vtkbone.vtkboneCoarsenModel()
    coarsener.SetInputData (model)
    coarsener.Update()
    coarse_model = coarsener.GetOutput()

    bounds = coarse_model.GetBounds()
    self.assertAlmostEqual (bounds[0], 3.5)
    self.assertAlmostEqual (bounds[1], 12.5)
    self.assertAlmostEqual (bounds[2], 4.5)
    self.assertAlmostEqual (bounds[3], 13.5)
    self.assertAlmostEqual (bounds[4], 5.5)
    self.assertAlmostEqual (bounds[5], 14.5)

    coarse_material = coarse_model.GetMaterialTable().GetMaterial(1)
    self.assertTrue (isinstance (coarse_material, vtkbone.vtkboneLinearIsotropicMaterialArray))
    self.assertEqual (coarse_material.GetSize(), 3**3)
    coarse_E = vtk_to_numpy (coarse_material.GetYoungsModulus())
    coarse_E.shape = (3,3,3)
    self.assertTrue (alltrue(abs(coarse_E[0,:,:] - 1.5) < 1E-6))
    self.assertTrue (alltrue(abs(coarse_E[1,:,:] - 3.5) < 1E-6))
    self.assertTrue (alltrue(abs(coarse_E[2,:,:] - 5) < 1E-6))


  def test_cube_with_holes (self):
	# Create 4x4x4 cube image
    cellmap = ones((4,4,4), int16)   # z,y,x order
    # Punch some holes in such as way that the first coarse element
    # has one hole, the second two, etc... Make sure 1,1,1 is the
    # last to be knocked out, to avoid reduced the bounds of the
    # FE model.
    offsets = array([[0,2,0,2,0,2,0,2],
    	             [0,0,2,2,0,0,2,2],
    	             [0,0,0,0,2,2,2,2]])
    cellmap[1+offsets[2],1+offsets[1],0+offsets[0]] = 0
    offsets = offsets[:,1:]
    cellmap[1+offsets[2],0+offsets[1],0+offsets[0]] = 0
    offsets = offsets[:,1:]
    cellmap[0+offsets[2],0+offsets[1],0+offsets[0]] = 0
    offsets = offsets[:,1:]
    cellmap[0+offsets[2],1+offsets[1],1+offsets[0]] = 0
    offsets = offsets[:,1:]
    cellmap[1+offsets[2],0+offsets[1],1+offsets[0]] = 0
    offsets = offsets[:,1:]
    cellmap[0+offsets[2],0+offsets[1],1+offsets[0]] = 0
    offsets = offsets[:,1:]
    cellmap[0+offsets[2],1+offsets[1],0+offsets[0]] = 0
    offsets = offsets[:,1:]
    cellmap[1+offsets[2],1+offsets[1],1+offsets[0]] = 0
    cellmap_flat = cellmap.flatten().copy()
    cellmap_vtk = numpy_to_vtk(cellmap_flat, deep=1)
    image = vtk.vtkImageData()
    image.SetDimensions((5,5,5))     # x,y,z order
    image.SetSpacing(1.5,1.5,1.5)
    image.SetOrigin(3.5,4.5,5.5)
    image.GetCellData().SetScalars(cellmap_vtk)

    # Convert to mesh
    geometry_generator = vtkbone.vtkboneImageToMesh()
    geometry_generator.SetInputData(image)
    geometry_generator.Update()
    geometry = geometry_generator.GetOutput()

    # Generate material.
    material = vtkbone.vtkboneLinearIsotropicMaterial()
    material.SetName("linear_iso_material")
    material.SetYoungsModulus(6000)
    material.SetPoissonsRatio(0.3)
    mt_generator = vtkbone.vtkboneGenerateHomogeneousMaterialTable()
    mt_generator.SetMaterial(material)
    mt_generator.SetMaterialIdList(image.GetCellData().GetScalars())
    mt_generator.Update()
    material_table = mt_generator.GetOutput()

    # Generate model
    generator = vtkbone.vtkboneApplyCompressionTest()
    generator.SetInputData(0, geometry)
    generator.SetInputData(1, material_table)
    generator.Update()
    model = generator.GetOutput()

    # Apply coarsener
    coarsener = vtkbone.vtkboneCoarsenModel()
    coarsener.SetInputData (model)
    coarsener.Update()
    coarse_model = coarsener.GetOutput()

    bounds = coarse_model.GetBounds()
    self.assertAlmostEqual (bounds[0], 3.5)
    self.assertAlmostEqual (bounds[1], 9.5)
    self.assertAlmostEqual (bounds[2], 4.5)
    self.assertAlmostEqual (bounds[3], 10.5)
    self.assertAlmostEqual (bounds[4], 5.5)
    self.assertAlmostEqual (bounds[5], 11.5)

    coarse_material = coarse_model.GetMaterialTable().GetMaterial(1)
    self.assertTrue (isinstance (coarse_material, vtkbone.vtkboneLinearIsotropicMaterialArray))
    self.assertEqual (coarse_material.GetSize(), 8)
    coarse_E = vtk_to_numpy (coarse_material.GetYoungsModulus())
    self.assertAlmostEqual (coarse_E[0], 6000*1/8)
    self.assertAlmostEqual (coarse_E[1], 6000*2/8)
    self.assertAlmostEqual (coarse_E[2], 6000*3/8)
    self.assertAlmostEqual (coarse_E[3], 6000*4/8)
    self.assertAlmostEqual (coarse_E[4], 6000*5/8)
    self.assertAlmostEqual (coarse_E[5], 6000*6/8)
    self.assertAlmostEqual (coarse_E[6], 6000*7/8)
    self.assertAlmostEqual (coarse_E[7], 6000)
    cell_scalars = vtk_to_numpy (coarse_model.GetCellData().GetScalars())
    self.assertEqual (len(cell_scalars), 7)
    self.assertEqual (cell_scalars[0], 7)
    self.assertEqual (cell_scalars[1], 6)
    self.assertEqual (cell_scalars[2], 5)
    self.assertEqual (cell_scalars[3], 4)
    self.assertEqual (cell_scalars[4], 3)
    self.assertEqual (cell_scalars[5], 2)
    self.assertEqual (cell_scalars[6], 1)
    coarse_nu = vtk_to_numpy (coarse_material.GetPoissonsRatio())
    self.assertAlmostEqual (coarse_nu[0], 0.3)
    self.assertAlmostEqual (coarse_nu[1], 0.3)
    self.assertAlmostEqual (coarse_nu[2], 0.3)
    self.assertAlmostEqual (coarse_nu[3], 0.3)
    self.assertAlmostEqual (coarse_nu[4], 0.3)
    self.assertAlmostEqual (coarse_nu[5], 0.3)
    self.assertAlmostEqual (coarse_nu[6], 0.3)


if __name__ == '__main__':
    unittest.main()
