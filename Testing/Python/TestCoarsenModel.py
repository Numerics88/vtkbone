from __future__ import division

import unittest
from numpy.core import *
import numpy
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
import vtkbone


def stress_strain_isotropic (E, nu):
    s = nu/(1-nu)
    t = (1-2*nu)/(2*(1-nu))
    D = array ([[ 1, s, s, 0, 0, 0],
                [ s, 1, s, 0, 0, 0],
                [ s, s, 1, 0, 0, 0],
                [ 0, 0, 0, t, 0, 0],
                [ 0, 0, 0, 0, t, 0],
                [ 0, 0, 0, 0, 0, t]]
               )
    D *= E*(1-nu)/((1+nu)*(1-2*nu))
    return D

def stress_strain_orthotropic (E, nu, G):
    nu12 = nu[0]
    nu20 = nu[1]
    nu01 = nu[2]
    nu21 = (E[2]/E[1])*nu12
    nu02 = (E[0]/E[2])*nu20
    nu10 = (E[1]/E[0])*nu01
    D = zeros((6,6), float)
    D[:3,:3] = array ([[     1/E[0], -nu10/E[1], -nu20/E[2]],
                       [ -nu01/E[0],     1/E[1], -nu21/E[2]],
                       [ -nu02/E[0], -nu12/E[1],     1/E[2]]])
    D[3,3] = 1/G[0]
    D[4,4] = 1/G[1]
    D[5,5] = 1/G[2]
    return numpy.linalg.inv(D)

def upper_triangular_to_square (ut):
    s = zeros((6,6), dtype=float)
    s[0,0] = ut[0]
    s[1,:2] = ut[1:3]
    s[:2,1] = ut[1:3]
    s[2,:3] = ut[3:6]
    s[:3,2] = ut[3:6]
    s[3,:4] = ut[6:10]
    s[:4,3] = ut[6:10]
    s[4,:5] = ut[10:15]
    s[:5,4] = ut[10:15]
    s[5,:6] = ut[15:21]
    s[:6,5] = ut[15:21]
    return s


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
    coarsener.SetMaterialAveragingMethod (vtkbone.vtkboneCoarsenModel.LINEAR)
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
    coarsener.SetMaterialAveragingMethod (vtkbone.vtkboneCoarsenModel.LINEAR)
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
    coarsener.SetMaterialAveragingMethod (vtkbone.vtkboneCoarsenModel.LINEAR)
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


  def test_cube_with_holes_linear (self):
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
    coarsener.SetMaterialAveragingMethod (vtkbone.vtkboneCoarsenModel.LINEAR)
    coarsener.Update()
    coarse_model = coarsener.GetOutput()

    # Check bounds
    bounds = coarse_model.GetBounds()
    self.assertAlmostEqual (bounds[0], 3.5)
    self.assertAlmostEqual (bounds[1], 9.5)
    self.assertAlmostEqual (bounds[2], 4.5)
    self.assertAlmostEqual (bounds[3], 10.5)
    self.assertAlmostEqual (bounds[4], 5.5)
    self.assertAlmostEqual (bounds[5], 11.5)

    # Check materials: material array with 8 possible output materials
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
    coarse_nu = vtk_to_numpy (coarse_material.GetPoissonsRatio())
    self.assertAlmostEqual (coarse_nu[0], 0.3)
    self.assertAlmostEqual (coarse_nu[1], 0.3)
    self.assertAlmostEqual (coarse_nu[2], 0.3)
    self.assertAlmostEqual (coarse_nu[3], 0.3)
    self.assertAlmostEqual (coarse_nu[4], 0.3)
    self.assertAlmostEqual (coarse_nu[5], 0.3)
    self.assertAlmostEqual (coarse_nu[6], 0.3)

    # Check cell scalars: point to appropriate material ID
    cell_scalars = vtk_to_numpy (coarse_model.GetCellData().GetScalars())
    self.assertEqual (len(cell_scalars), 7)
    self.assertEqual (cell_scalars[0], 7)
    self.assertEqual (cell_scalars[1], 6)
    self.assertEqual (cell_scalars[2], 5)
    self.assertEqual (cell_scalars[3], 4)
    self.assertEqual (cell_scalars[4], 3)
    self.assertEqual (cell_scalars[5], 2)
    self.assertEqual (cell_scalars[6], 1)


  def test_cube_with_holes_homminga_density (self):
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
    coarsener.SetMaterialAveragingMethod (vtkbone.vtkboneCoarsenModel.HOMMINGA_DENSITY)
    coarsener.Update()
    coarse_model = coarsener.GetOutput()

    # Check bounds
    bounds = coarse_model.GetBounds()
    self.assertAlmostEqual (bounds[0], 3.5)
    self.assertAlmostEqual (bounds[1], 9.5)
    self.assertAlmostEqual (bounds[2], 4.5)
    self.assertAlmostEqual (bounds[3], 10.5)
    self.assertAlmostEqual (bounds[4], 5.5)
    self.assertAlmostEqual (bounds[5], 11.5)

    # Check materials: material array with 8 possible output materials
    coarse_material = coarse_model.GetMaterialTable().GetMaterial(1)
    self.assertTrue (isinstance (coarse_material, vtkbone.vtkboneLinearIsotropicMaterialArray))
    self.assertEqual (coarse_material.GetSize(), 8)
    coarse_E = vtk_to_numpy (coarse_material.GetYoungsModulus())
    self.assertAlmostEqual (coarse_E[0], 6000*(1/8)**1.7, delta=1E-2)
    self.assertAlmostEqual (coarse_E[1], 6000*(2/8)**1.7, delta=1E-2)
    self.assertAlmostEqual (coarse_E[2], 6000*(3/8)**1.7, delta=1E-2)
    self.assertAlmostEqual (coarse_E[3], 6000*(4/8)**1.7, delta=1E-2)
    self.assertAlmostEqual (coarse_E[4], 6000*(5/8)**1.7, delta=1E-2)
    self.assertAlmostEqual (coarse_E[5], 6000*(6/8)**1.7, delta=1E-2)
    self.assertAlmostEqual (coarse_E[6], 6000*(7/8)**1.7, delta=1E-2)
    self.assertAlmostEqual (coarse_E[7], 6000)
    coarse_nu = vtk_to_numpy (coarse_material.GetPoissonsRatio())
    self.assertAlmostEqual (coarse_nu[0], 0.3)
    self.assertAlmostEqual (coarse_nu[1], 0.3)
    self.assertAlmostEqual (coarse_nu[2], 0.3)
    self.assertAlmostEqual (coarse_nu[3], 0.3)
    self.assertAlmostEqual (coarse_nu[4], 0.3)
    self.assertAlmostEqual (coarse_nu[5], 0.3)
    self.assertAlmostEqual (coarse_nu[6], 0.3)

    # Check cell scalars: point to appropriate material ID
    cell_scalars = vtk_to_numpy (coarse_model.GetCellData().GetScalars())
    self.assertEqual (len(cell_scalars), 7)
    self.assertEqual (cell_scalars[0], 7)
    self.assertEqual (cell_scalars[1], 6)
    self.assertEqual (cell_scalars[2], 5)
    self.assertEqual (cell_scalars[3], 4)
    self.assertEqual (cell_scalars[4], 3)
    self.assertEqual (cell_scalars[5], 2)
    self.assertEqual (cell_scalars[6], 1)


  def test_two_isotropic_materials_linear (self):
    # Create 2x2x4 cube image
    cellmap = zeros((4,2,2), int16)   # z,y,x order
    # Bottom output cell has 6 input cells, of which 2 are materialA (ID 10), 4 material B (ID 14)
    cellmap[0,0,1] = 10
    cellmap[0,1,1] = 12
    cellmap[1,0,0] = 12
    cellmap[1,0,1] = 10
    cellmap[1,1,0] = 10
    cellmap[1,1,1] = 10
    # Top output cell has 5 input cells, of which 1 is material B
    cellmap[2,0,0] = 10
    cellmap[2,1,1] = 10
    cellmap[2,1,0] = 10
    cellmap[3,0,0] = 12
    cellmap[3,1,0] = 10
    cellmap_flat = cellmap.flatten().copy()
    cellmap_vtk = numpy_to_vtk(cellmap_flat, deep=1)
    image = vtk.vtkImageData()
    image.SetDimensions((3,3,5))     # x,y,z order
    image.SetSpacing(1.5,1.5,1.5)
    image.SetOrigin(3.5,4.5,5.5)
    image.GetCellData().SetScalars(cellmap_vtk)

    # Convert to mesh
    geometry_generator = vtkbone.vtkboneImageToMesh()
    geometry_generator.SetInputData(image)
    geometry_generator.Update()
    geometry = geometry_generator.GetOutput()

    # Generate materials.
    materialA = vtkbone.vtkboneLinearIsotropicMaterial()
    materialA.SetName("materialA")
    materialA.SetYoungsModulus(6000)
    materialA.SetPoissonsRatio(0.3)
    materialB = vtkbone.vtkboneLinearIsotropicMaterial()
    materialB.SetName("materialB")
    materialB.SetYoungsModulus(4000)
    materialB.SetPoissonsRatio(0.4)
    material_table = vtkbone.vtkboneMaterialTable()
    material_table.AddMaterial (10, materialA)
    material_table.AddMaterial (12, materialB)

    # Generate model
    generator = vtkbone.vtkboneApplyCompressionTest()
    generator.SetInputData(0, geometry)
    generator.SetInputData(1, material_table)
    generator.Update()
    model = generator.GetOutput()

    # Apply coarsener
    coarsener = vtkbone.vtkboneCoarsenModel()
    coarsener.SetInputData (model)
    coarsener.SetMaterialAveragingMethod (vtkbone.vtkboneCoarsenModel.LINEAR)
    coarsener.Update()
    coarse_model = coarsener.GetOutput()

    # Check bounds
    bounds = coarse_model.GetBounds()
    self.assertAlmostEqual (bounds[0], 3.5)
    self.assertAlmostEqual (bounds[1], 6.5)
    self.assertAlmostEqual (bounds[2], 4.5)
    self.assertAlmostEqual (bounds[3], 7.5)
    self.assertAlmostEqual (bounds[4], 5.5)
    self.assertAlmostEqual (bounds[5], 11.5)

    # Check cell scalars: sequence
    cell_scalars = vtk_to_numpy (coarse_model.GetCellData().GetScalars())
    self.assertEqual (len(cell_scalars), 2)
    self.assertEqual (cell_scalars[0], 1)
    self.assertEqual (cell_scalars[1], 2)

    # Check materials
    coarse_material = coarse_model.GetMaterialTable().GetMaterial(1)
    self.assertTrue (isinstance (coarse_material, vtkbone.vtkboneLinearIsotropicMaterialArray))
    self.assertEqual (coarse_material.GetSize(), 2)
    coarse_E = vtk_to_numpy (coarse_material.GetYoungsModulus())
    self.assertAlmostEqual (coarse_E[0], (4*6000 + 2*4000)/8)
    self.assertAlmostEqual (coarse_E[1], (4*6000 + 1*4000)/8)
    coarse_nu = vtk_to_numpy (coarse_material.GetPoissonsRatio())
    self.assertAlmostEqual (coarse_nu[0], (4*0.3 + 2*0.4)/6)
    self.assertAlmostEqual (coarse_nu[1], (4*0.3 + 1*0.4)/5)


  def test_two_isotropic_materials_homminga_density (self):
    # Create 2x2x4 cube image
    cellmap = zeros((4,2,2), int16)   # z,y,x order
    # Bottom output cell has 6 input cells, of which 2 are materialA (ID 10), 4 material B (ID 14)
    cellmap[0,0,1] = 10
    cellmap[0,1,1] = 12
    cellmap[1,0,0] = 12
    cellmap[1,0,1] = 10
    cellmap[1,1,0] = 10
    cellmap[1,1,1] = 10
    # Top output cell has 5 input cells, of which 1 is material B
    cellmap[2,0,0] = 10
    cellmap[2,1,1] = 10
    cellmap[2,1,0] = 10
    cellmap[3,0,0] = 12
    cellmap[3,1,0] = 10
    cellmap_flat = cellmap.flatten().copy()
    cellmap_vtk = numpy_to_vtk(cellmap_flat, deep=1)
    image = vtk.vtkImageData()
    image.SetDimensions((3,3,5))     # x,y,z order
    image.SetSpacing(1.5,1.5,1.5)
    image.SetOrigin(3.5,4.5,5.5)
    image.GetCellData().SetScalars(cellmap_vtk)

    # Convert to mesh
    geometry_generator = vtkbone.vtkboneImageToMesh()
    geometry_generator.SetInputData(image)
    geometry_generator.Update()
    geometry = geometry_generator.GetOutput()

    # Generate materials.
    materialA = vtkbone.vtkboneLinearIsotropicMaterial()
    materialA.SetName("materialA")
    materialA.SetYoungsModulus(6000)
    materialA.SetPoissonsRatio(0.3)
    materialB = vtkbone.vtkboneLinearIsotropicMaterial()
    materialB.SetName("materialB")
    materialB.SetYoungsModulus(4000)
    materialB.SetPoissonsRatio(0.4)
    material_table = vtkbone.vtkboneMaterialTable()
    material_table.AddMaterial (10, materialA)
    material_table.AddMaterial (12, materialB)

    # Generate model
    generator = vtkbone.vtkboneApplyCompressionTest()
    generator.SetInputData(0, geometry)
    generator.SetInputData(1, material_table)
    generator.Update()
    model = generator.GetOutput()

    # Apply coarsener
    coarsener = vtkbone.vtkboneCoarsenModel()
    coarsener.SetInputData (model)
    coarsener.SetMaterialAveragingMethod (vtkbone.vtkboneCoarsenModel.HOMMINGA_DENSITY)
    coarsener.Update()
    coarse_model = coarsener.GetOutput()

    # Check bounds
    bounds = coarse_model.GetBounds()
    self.assertAlmostEqual (bounds[0], 3.5)
    self.assertAlmostEqual (bounds[1], 6.5)
    self.assertAlmostEqual (bounds[2], 4.5)
    self.assertAlmostEqual (bounds[3], 7.5)
    self.assertAlmostEqual (bounds[4], 5.5)
    self.assertAlmostEqual (bounds[5], 11.5)

    # Check cell scalars: sequence
    cell_scalars = vtk_to_numpy (coarse_model.GetCellData().GetScalars())
    self.assertEqual (len(cell_scalars), 2)
    self.assertEqual (cell_scalars[0], 1)
    self.assertEqual (cell_scalars[1], 2)

    # Check materials
    coarse_material = coarse_model.GetMaterialTable().GetMaterial(1)
    self.assertTrue (isinstance (coarse_material, vtkbone.vtkboneLinearIsotropicMaterialArray))
    self.assertEqual (coarse_material.GetSize(), 2)
    coarse_E = vtk_to_numpy (coarse_material.GetYoungsModulus())
    self.assertAlmostEqual (coarse_E[0], ((4*6000**(1/1.7) + 2*4000**(1/1.7))/8)**1.7, delta=1E-2)
    self.assertAlmostEqual (coarse_E[1], ((4*6000**(1/1.7) + 1*4000**(1/1.7))/8)**1.7, delta=1E-2)
    coarse_nu = vtk_to_numpy (coarse_material.GetPoissonsRatio())
    self.assertAlmostEqual (coarse_nu[0], (4*0.3 + 2*0.4)/6)
    self.assertAlmostEqual (coarse_nu[1], (4*0.3 + 1*0.4)/5)


  def test_mixed_materials_linear (self):
    # Create 2x2x4 cube image
    cellmap = zeros((4,2,2), int16)   # z,y,x order
    # Bottom output cell has 6 input cells, of which:
    #   3 are materialA (ID 10)
    #   2 material B (ID 12)
    #   1 is material C (ID 15)
    cellmap[0,0,1] = 10
    cellmap[0,1,1] = 12
    cellmap[1,0,0] = 12
    cellmap[1,0,1] = 15
    cellmap[1,1,0] = 10
    cellmap[1,1,1] = 10
    # Top output cell has 5 input cells, of which:
    #   2 are materialA (ID 10)
    #   1 is material B (ID 12)
    #   2 are materialC (ID 15)
    cellmap[2,0,0] = 15
    cellmap[2,1,1] = 10
    cellmap[2,1,0] = 10
    cellmap[3,0,0] = 12
    cellmap[3,1,0] = 15
    cellmap_flat = cellmap.flatten().copy()
    cellmap_vtk = numpy_to_vtk(cellmap_flat, deep=1)
    image = vtk.vtkImageData()
    image.SetDimensions((3,3,5))     # x,y,z order
    image.SetSpacing(1.5,1.5,1.5)
    image.SetOrigin(3.5,4.5,5.5)
    image.GetCellData().SetScalars(cellmap_vtk)

    # Convert to mesh
    geometry_generator = vtkbone.vtkboneImageToMesh()
    geometry_generator.SetInputData(image)
    geometry_generator.Update()
    geometry = geometry_generator.GetOutput()

    # Generate materials.
    materialA = vtkbone.vtkboneLinearIsotropicMaterial()
    materialA.SetName("materialA")
    materialA.SetYoungsModulus(6000)
    materialA.SetPoissonsRatio(0.3)
    DA = stress_strain_isotropic (6000.0, 0.3)
    materialB = vtkbone.vtkboneLinearOrthotropicMaterial()
    materialB.SetYoungsModulusX(1000)
    materialB.SetYoungsModulusY(1100)
    materialB.SetYoungsModulusZ(1200)
    materialB.SetPoissonsRatioYZ(0.25)
    materialB.SetPoissonsRatioZX(0.3)
    materialB.SetPoissonsRatioXY(0.2)
    # These values are not necessarily consistent
    GYZ = 1000/(2*(1+0.25))
    GZX = 1100/(2*(1+0.3))
    GXY = 1200/(2*(1+0.2))
    materialB.SetShearModulusYZ(GYZ)
    materialB.SetShearModulusZX(GZX)
    materialB.SetShearModulusXY(GXY)
    DB = stress_strain_orthotropic ((1000.0, 1100.0, 1200.0),
                                    (   0.25,    0.3,   0.2),
                                    (    GYZ,    GZX,   GXY))
    materialC = vtkbone.vtkboneLinearAnisotropicMaterial()
    DC = array((
          (1571.653,   540.033,   513.822,     7.53 ,  -121.22 ,   -57.959),
          ( 540.033,  2029.046,   469.974,    78.591,   -53.69 ,   -50.673),
          ( 513.822,   469.974,  1803.998,    20.377,   -57.014,   -15.761),
          (   7.53 ,    78.591,    20.377,   734.405,   -23.127,   -36.557),
          (-121.22 ,   -53.69 ,   -57.014,   -23.127,   627.396,    13.969),
          ( -57.959,   -50.673,   -15.761,   -36.557,    13.969,   745.749)))
    DC_vtk = numpy_to_vtk (DC, array_type=vtk.VTK_FLOAT)
    materialC.SetStressStrainMatrix(DC_vtk)
    material_table = vtkbone.vtkboneMaterialTable()
    material_table.AddMaterial (10, materialA)
    material_table.AddMaterial (12, materialB)
    material_table.AddMaterial (15, materialC)

    # Generate model
    generator = vtkbone.vtkboneApplyCompressionTest()
    generator.SetInputData(0, geometry)
    generator.SetInputData(1, material_table)
    generator.Update()
    model = generator.GetOutput()

    # Apply coarsener
    coarsener = vtkbone.vtkboneCoarsenModel()
    coarsener.SetInputData (model)
    coarsener.SetMaterialAveragingMethod (vtkbone.vtkboneCoarsenModel.LINEAR)
    coarsener.Update()
    coarse_model = coarsener.GetOutput()

    # Check bounds
    bounds = coarse_model.GetBounds()
    self.assertAlmostEqual (bounds[0], 3.5)
    self.assertAlmostEqual (bounds[1], 6.5)
    self.assertAlmostEqual (bounds[2], 4.5)
    self.assertAlmostEqual (bounds[3], 7.5)
    self.assertAlmostEqual (bounds[4], 5.5)
    self.assertAlmostEqual (bounds[5], 11.5)

    # Check cell scalars: sequence
    cell_scalars = vtk_to_numpy (coarse_model.GetCellData().GetScalars())
    self.assertEqual (len(cell_scalars), 2)
    self.assertEqual (cell_scalars[0], 1)
    self.assertEqual (cell_scalars[1], 2)

    # Check materials
    coarse_material = coarse_model.GetMaterialTable().GetMaterial(1)
    self.assertTrue (isinstance (coarse_material, vtkbone.vtkboneLinearAnisotropicMaterialArray))
    self.assertEqual (coarse_material.GetSize(), 2)
    ut_vtk = coarse_material.GetStressStrainMatrixUpperTriangular()
    ut = vtk_to_numpy (ut_vtk)
    self.assertEqual (ut.shape, (2,21))
    D1 = upper_triangular_to_square (ut[0])
    D1_ref = (3*DA + 2*DB + 1*DC)/8
    self.assertTrue (alltrue(abs(D1-D1_ref) < 1E-3))
    D2 = upper_triangular_to_square (ut[1])
    D2_ref = (2*DA + 1*DB + 2*DC)/8
    self.assertTrue (alltrue(abs(D2-D2_ref) < 1E-3))


  def test_mixed_materials_homminga_density (self):
    # Create 2x2x4 cube image
    cellmap = zeros((4,2,2), int16)   # z,y,x order
    # Bottom output cell has 6 input cells, of which:
    #   3 are materialA (ID 10)
    #   2 material B (ID 12)
    #   1 is material C (ID 15)
    cellmap[0,0,1] = 10
    cellmap[0,1,1] = 12
    cellmap[1,0,0] = 12
    cellmap[1,0,1] = 15
    cellmap[1,1,0] = 10
    cellmap[1,1,1] = 10
    # Top output cell has 5 input cells, of which:
    #   2 are materialA (ID 10)
    #   1 is material B (ID 12)
    #   2 are materialA (ID 15)
    cellmap[2,0,0] = 15
    cellmap[2,1,1] = 10
    cellmap[2,1,0] = 10
    cellmap[3,0,0] = 12
    cellmap[3,1,0] = 15
    cellmap_flat = cellmap.flatten().copy()
    cellmap_vtk = numpy_to_vtk(cellmap_flat, deep=1)
    image = vtk.vtkImageData()
    image.SetDimensions((3,3,5))     # x,y,z order
    image.SetSpacing(1.5,1.5,1.5)
    image.SetOrigin(3.5,4.5,5.5)
    image.GetCellData().SetScalars(cellmap_vtk)

    # Convert to mesh
    geometry_generator = vtkbone.vtkboneImageToMesh()
    geometry_generator.SetInputData(image)
    geometry_generator.Update()
    geometry = geometry_generator.GetOutput()

    # Generate materials.
    materialA = vtkbone.vtkboneLinearIsotropicMaterial()
    materialA.SetName("materialA")
    materialA.SetYoungsModulus(6000)
    materialA.SetPoissonsRatio(0.3)
    DA = stress_strain_isotropic (6000.0, 0.3)
    materialB = vtkbone.vtkboneLinearOrthotropicMaterial()
    materialB.SetYoungsModulusX(1000)
    materialB.SetYoungsModulusY(1100)
    materialB.SetYoungsModulusZ(1200)
    materialB.SetPoissonsRatioYZ(0.25)
    materialB.SetPoissonsRatioZX(0.3)
    materialB.SetPoissonsRatioXY(0.2)
    # These values are not necessarily consistent
    GYZ = 1000/(2*(1+0.25))
    GZX = 1100/(2*(1+0.3))
    GXY = 1200/(2*(1+0.2))
    materialB.SetShearModulusYZ(GYZ)
    materialB.SetShearModulusZX(GZX)
    materialB.SetShearModulusXY(GXY)
    DB = stress_strain_orthotropic ((1000.0, 1100.0, 1200.0),
                                    (   0.25,    0.3,   0.2),
                                    (    GYZ,    GZX,   GXY))
    materialC = vtkbone.vtkboneLinearAnisotropicMaterial()
    DC = array((
          (1571.653,   540.033,   513.822,     7.53 ,  -121.22 ,   -57.959),
          ( 540.033,  2029.046,   469.974,    78.591,   -53.69 ,   -50.673),
          ( 513.822,   469.974,  1803.998,    20.377,   -57.014,   -15.761),
          (   7.53 ,    78.591,    20.377,   734.405,   -23.127,   -36.557),
          (-121.22 ,   -53.69 ,   -57.014,   -23.127,   627.396,    13.969),
          ( -57.959,   -50.673,   -15.761,   -36.557,    13.969,   745.749)))
    DC_vtk = numpy_to_vtk (DC, array_type=vtk.VTK_FLOAT)
    materialC.SetStressStrainMatrix(DC_vtk)
    material_table = vtkbone.vtkboneMaterialTable()
    material_table.AddMaterial (10, materialA)
    material_table.AddMaterial (12, materialB)
    material_table.AddMaterial (15, materialC)

    # Generate model
    generator = vtkbone.vtkboneApplyCompressionTest()
    generator.SetInputData(0, geometry)
    generator.SetInputData(1, material_table)
    generator.Update()
    model = generator.GetOutput()

    # Apply coarsener
    coarsener = vtkbone.vtkboneCoarsenModel()
    coarsener.SetInputData (model)
    coarsener.SetMaterialAveragingMethod (vtkbone.vtkboneCoarsenModel.HOMMINGA_DENSITY)
    coarsener.Update()
    coarse_model = coarsener.GetOutput()

    # Check bounds
    bounds = coarse_model.GetBounds()
    self.assertAlmostEqual (bounds[0], 3.5)
    self.assertAlmostEqual (bounds[1], 6.5)
    self.assertAlmostEqual (bounds[2], 4.5)
    self.assertAlmostEqual (bounds[3], 7.5)
    self.assertAlmostEqual (bounds[4], 5.5)
    self.assertAlmostEqual (bounds[5], 11.5)

    # Check cell scalars: sequence
    cell_scalars = vtk_to_numpy (coarse_model.GetCellData().GetScalars())
    self.assertEqual (len(cell_scalars), 2)
    self.assertEqual (cell_scalars[0], 1)
    self.assertEqual (cell_scalars[1], 2)

    # Check materials
    coarse_material = coarse_model.GetMaterialTable().GetMaterial(1)
    self.assertTrue (isinstance (coarse_material, vtkbone.vtkboneLinearAnisotropicMaterialArray))
    self.assertEqual (coarse_material.GetSize(), 2)
    ut_vtk = coarse_material.GetStressStrainMatrixUpperTriangular()
    ut = vtk_to_numpy (ut_vtk)
    self.assertEqual (ut.shape, (2,21))
    D1 = upper_triangular_to_square (ut[0])
    D1_ref = zeros((6,6), dtype=float)
    for i in range(6):
        for j in range (6):
            x = 0.0
            a = DA[i,j]
            b = DB[i,j]
            c = DC[i,j]
            if a > 0:
                x += 3*a**(1/1.7)
            if a < 0:
                x -= 3*(-a)**(1/1.7)
            if b > 0:
                x += 2*b**(1/1.7)
            if b < 0:
                x -= 2*(-b)**(1/1.7)
            if c > 0:
                x += 1*c**(1/1.7)
            if c < 0:
                x -= 1*(-c)**(1/1.7)
            x /= 8
            if x > 0:
                D1_ref[i,j] = x**1.7
            if x < 0:
                D1_ref[i,j] = -((-x)**1.7)
    self.assertTrue (alltrue(abs(D1-D1_ref) < 1E-2))
    D2 = upper_triangular_to_square (ut[1])
    D2_ref = zeros((6,6), dtype=float)
    for i in range(6):
        for j in range (6):
            x = 0.0
            a = DA[i,j]
            b = DB[i,j]
            c = DC[i,j]
            if a > 0:
                x += 2*a**(1/1.7)
            if a < 0:
                x -= 2*(-a)**(1/1.7)
            if b > 0:
                x += 1*b**(1/1.7)
            if b < 0:
                x -= 1*(-b)**(1/1.7)
            if c > 0:
                x += 2*c**(1/1.7)
            if c < 0:
                x -= 2*(-c)**(1/1.7)
            x /= 8
            if x > 0:
                D2_ref[i,j] = x**1.7
            if x < 0:
                D2_ref[i,j] = -((-x)**1.7)
    self.assertTrue (alltrue(abs(D2-D2_ref) < 1E-2))


if __name__ == '__main__':
    unittest.main()
