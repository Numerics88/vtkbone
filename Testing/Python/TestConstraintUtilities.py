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


class TestConstaintUtilities (unittest.TestCase):

    def test_create_boundary_condition(self):
        nodes = array((8,9,10,11))
        nodes_vtk = numpy_to_vtk(nodes, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateBoundaryCondition(
                              nodes_vtk, 2, 0.1, "TEST DISPLACEMENT CONSTRAINT")
        self.assertFalse(constraint is None)
        self.assertEqual(constraint.GetName(), "TEST DISPLACEMENT CONSTRAINT")
        self.assertEqual(constraint.GetConstraintType(), vtkbone.vtkboneConstraint.DISPLACEMENT)
        self.assertEqual(constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == nodes))
        senses_vtk = constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1))


    def test_create_single_boundary_condition(self):
        constraint = vtkbone.vtkboneConstraintUtilities.CreateBoundaryCondition(
                              8, 2, 0.1, "TEST DISPLACEMENT CONSTRAINT")
        self.assertFalse(constraint is None)
        self.assertEqual(constraint.GetName(), "TEST DISPLACEMENT CONSTRAINT")
        self.assertEqual(constraint.GetConstraintType(), vtkbone.vtkboneConstraint.DISPLACEMENT)
        self.assertEqual(constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == 8))
        senses_vtk = constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1))


    def test_create_fixed_nodes(self):
        nodes = array((8,9))
        nodes_vtk = numpy_to_vtk(nodes, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateFixedNodes(
                              nodes_vtk, "TEST FIXED CONSTRAINT")
        self.assertFalse(constraint is None)
        self.assertEqual(constraint.GetName(), "TEST FIXED CONSTRAINT")
        self.assertEqual(constraint.GetConstraintType(), vtkbone.vtkboneConstraint.DISPLACEMENT)
        self.assertEqual(constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (8,8,8,9,9,9)))
        senses_vtk = constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == (0,1,2,0,1,2)))
        values_vtk = constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0))


    def test_create_single_fixed_node(self):
        constraint = vtkbone.vtkboneConstraintUtilities.CreateFixedNodes(
                              8, "TEST FIXED CONSTRAINT")
        self.assertFalse(constraint is None)
        self.assertEqual(constraint.GetName(), "TEST FIXED CONSTRAINT")
        self.assertEqual(constraint.GetConstraintType(), vtkbone.vtkboneConstraint.DISPLACEMENT)
        self.assertEqual(constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (8,8,8)))
        senses_vtk = constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == (0,1,2)))
        values_vtk = constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0))


    def test_create_applied_load(self):
        elements = array((2,11))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_Y0_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT")
        self.assertFalse(constraint is None)
        self.assertEqual(constraint.GetName(), "TEST FORCE CONSTRAINT")
        self.assertEqual(constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.ELEMENTS)
        ids_vtk = constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == elements))
        distributions_vtk = constraint.GetAttributes().GetArray("DISTRIBUTION")
        self.assertFalse(distributions_vtk is None)
        distributions = vtk_to_numpy(distributions_vtk)
        self.assertTrue(alltrue(distributions == vtkbone.vtkboneConstraint.FACE_Y0_DISTRIBUTION))
        senses_vtk = constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1))


    def test_create_single_applied_load(self):
        constraint = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           8, vtkbone.vtkboneConstraint.FACE_Y0_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT")
        self.assertFalse(constraint is None)
        self.assertEqual(constraint.GetName(), "TEST FORCE CONSTRAINT")
        self.assertEqual(constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.ELEMENTS)
        ids_vtk = constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == 8))
        distributions_vtk = constraint.GetAttributes().GetArray("DISTRIBUTION")
        self.assertFalse(distributions_vtk is None)
        distributions = vtk_to_numpy(distributions_vtk)
        self.assertTrue(alltrue(distributions == vtkbone.vtkboneConstraint.FACE_Y0_DISTRIBUTION))
        senses_vtk = constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1))


    def test_gather_displacement_constraint(self):
        nodes1 = array((10,11,9,8))
        nodes1_vtk = numpy_to_vtk(nodes1, deep=1, array_type=vtk.VTK_ID_TYPE)
        senses1 = array((2,0,1,0))
        senses1_vtk = numpy_to_vtk(senses1, deep=1)
        values1 = array((0.2,0.1,0.1,0.1))
        values1_vtk = numpy_to_vtk(values1, deep=1)
        constraint1 = vtkbone.vtkboneConstraintUtilities.CreateBoundaryCondition(
                  nodes1_vtk, senses1_vtk, values1_vtk, "TEST DISPLACEMENT CONSTRAINT 1")
        nodes2 = array((8,0,10))
        nodes2_vtk = numpy_to_vtk(nodes2, deep=2, array_type=vtk.VTK_ID_TYPE)
        senses2 = array((0,2,0))
        senses2_vtk = numpy_to_vtk(senses2, deep=2)
        values2 = array((0.1,0.4,0.4))
        values2_vtk = numpy_to_vtk(values2, deep=2)
        constraint2 = vtkbone.vtkboneConstraintUtilities.CreateBoundaryCondition(
                  nodes2_vtk, senses2_vtk, values2_vtk, "TEST DISPLACEMENT CONSTRAINT 2")
        constraints = vtkbone.vtkboneConstraintCollection()
        constraints.AddItem(constraint1)
        constraints.AddItem(constraint2)
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.GatherDisplacementConstraints(constraints)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.DISPLACEMENT)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (0,8,9,10,10,11)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        expected_senses = array((2,0,1,0,2,0))
        self.assertTrue(alltrue(senses == expected_senses))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        expected_values = array((0.4,0.1,0.1,0.4,0.2,0.1))
        self.assertTrue(alltrue(values == expected_values))


    def test_gather_zero_valued_displacement_constraint(self):
        nodes1 = array((10,11,9,8,8))
        nodes1_vtk = numpy_to_vtk(nodes1, deep=1, array_type=vtk.VTK_ID_TYPE)
        senses1 = array((2,0,1,0,1))
        senses1_vtk = numpy_to_vtk(senses1, deep=1)
        values1 = array((0.0,0.1,0.1,0.0,0.0))
        values1_vtk = numpy_to_vtk(values1, deep=1)
        constraint1 = vtkbone.vtkboneConstraintUtilities.CreateBoundaryCondition(
                  nodes1_vtk, senses1_vtk, values1_vtk, "TEST DISPLACEMENT CONSTRAINT 1")
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.GatherZeroValuedDisplacementConstraints(
            constraint1, 1E-8)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.DISPLACEMENT)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (8,8,10)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        expected_senses = array((0,1,2))
        self.assertTrue(alltrue(senses == expected_senses))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0))


    def test_gather_nonzero_displacement_constraint(self):
        nodes1 = array((10,11,9,8,8))
        nodes1_vtk = numpy_to_vtk(nodes1, deep=1, array_type=vtk.VTK_ID_TYPE)
        senses1 = array((2,0,1,0,1))
        senses1_vtk = numpy_to_vtk(senses1, deep=1)
        values1 = array((0.0,0.2,0.1,0.0,0.0))
        values1_vtk = numpy_to_vtk(values1, deep=1)
        constraint1 = vtkbone.vtkboneConstraintUtilities.CreateBoundaryCondition(
                  nodes1_vtk, senses1_vtk, values1_vtk, "TEST DISPLACEMENT CONSTRAINT 1")
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.GatherNonzeroDisplacementConstraints(
            constraint1, 1E-8)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.DISPLACEMENT)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (9,11)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        expected_senses = array((1,0))
        self.assertTrue(alltrue(senses == expected_senses))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == (0.1,0.2)))


    # The following 7 tests verify that each of the 7 possible distributions is
    # correct.

    def test_distribute_force_constraint_x0(self):
        geometry = test_geometries.generate_two_element_geometry()
        elements = array((1,))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_X0_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT")
        self.assertFalse(constraint is None)
        distributions_vtk = constraint.GetAttributes().GetArray("DISTRIBUTION")
        self.assertFalse(distributions_vtk is None)
        distributions = vtk_to_numpy(distributions_vtk)
        self.assertTrue(alltrue(distributions == vtkbone.vtkboneConstraint.FACE_X0_DISTRIBUTION))
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.DistributeConstraintToNodes(
                geometry, constraint)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (4,6,8,10)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1/4))


    def test_distribute_force_constraint_x1(self):
        geometry = test_geometries.generate_two_element_geometry()
        elements = array((1,))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_X1_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT")
        self.assertFalse(constraint is None)
        distributions_vtk = constraint.GetAttributes().GetArray("DISTRIBUTION")
        self.assertFalse(distributions_vtk is None)
        distributions = vtk_to_numpy(distributions_vtk)
        self.assertTrue(alltrue(distributions == vtkbone.vtkboneConstraint.FACE_X1_DISTRIBUTION))
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.DistributeConstraintToNodes(
                geometry, constraint)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (5,7,9,11)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1/4))


    def test_distribute_force_constraint_y0(self):
        geometry = test_geometries.generate_two_element_geometry()
        elements = array((1,))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_Y0_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT")
        self.assertFalse(constraint is None)
        distributions_vtk = constraint.GetAttributes().GetArray("DISTRIBUTION")
        self.assertFalse(distributions_vtk is None)
        distributions = vtk_to_numpy(distributions_vtk)
        self.assertTrue(alltrue(distributions == vtkbone.vtkboneConstraint.FACE_Y0_DISTRIBUTION))
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.DistributeConstraintToNodes(
                geometry, constraint)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (4,5,8,9)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1/4))


    def test_distribute_force_constraint_y1(self):
        geometry = test_geometries.generate_two_element_geometry()
        elements = array((1,))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_Y1_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT")
        self.assertFalse(constraint is None)
        distributions_vtk = constraint.GetAttributes().GetArray("DISTRIBUTION")
        self.assertFalse(distributions_vtk is None)
        distributions = vtk_to_numpy(distributions_vtk)
        self.assertTrue(alltrue(distributions == vtkbone.vtkboneConstraint.FACE_Y1_DISTRIBUTION))
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.DistributeConstraintToNodes(
                geometry, constraint)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (6,7,10,11)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1/4))


    def test_distribute_force_constraint_z0(self):
        geometry = test_geometries.generate_two_element_geometry()
        elements = array((1,))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_Z0_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT")
        self.assertFalse(constraint is None)
        distributions_vtk = constraint.GetAttributes().GetArray("DISTRIBUTION")
        self.assertFalse(distributions_vtk is None)
        distributions = vtk_to_numpy(distributions_vtk)
        self.assertTrue(alltrue(distributions == vtkbone.vtkboneConstraint.FACE_Z0_DISTRIBUTION))
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.DistributeConstraintToNodes(
                geometry, constraint)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (4,5,6,7)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1/4))


    def test_distribute_force_constraint_z1(self):
        geometry = test_geometries.generate_two_element_geometry()
        elements = array((1,))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_Z1_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT")
        self.assertFalse(constraint is None)
        distributions_vtk = constraint.GetAttributes().GetArray("DISTRIBUTION")
        self.assertFalse(distributions_vtk is None)
        distributions = vtk_to_numpy(distributions_vtk)
        self.assertTrue(alltrue(distributions == vtkbone.vtkboneConstraint.FACE_Z1_DISTRIBUTION))
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.DistributeConstraintToNodes(
                geometry, constraint)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (8,9,10,11)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1/4))


    def test_distribute_force_constraint_body(self):
        geometry = test_geometries.generate_two_element_geometry()
        elements = array((1,))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.BODY_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT")
        self.assertFalse(constraint is None)
        distributions_vtk = constraint.GetAttributes().GetArray("DISTRIBUTION")
        self.assertFalse(distributions_vtk is None)
        distributions = vtk_to_numpy(distributions_vtk)
        self.assertTrue(alltrue(distributions == vtkbone.vtkboneConstraint.BODY_DISTRIBUTION))
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.DistributeConstraintToNodes(
                geometry, constraint)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (4,5,6,7,8,9,10,11)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1/8))


    def test_distribute_force_constraint_two_constraints_different_directions(self):
        geometry = test_geometries.generate_two_element_geometry()
        elements = array((1,))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint1 = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_Z1_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT 1")
        constraint2 = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_X0_DISTRIBUTION, 0, 0.1, "TEST FORCE CONSTRAINT 2")
        constraints = vtkbone.vtkboneConstraintCollection()
        constraints.AddItem(constraint1)
        constraints.AddItem(constraint2)
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.DistributeForceConstraintsToNodes(
                geometry, constraints)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (4,6,8,8,9,10,10,11)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        expected_senses = array((0,0,0,2,2,0,2,2))
        self.assertTrue(alltrue(senses == expected_senses))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        self.assertTrue(alltrue(values == 0.1/4))


    def test_distribute_force_constraint_two_constraints_same_directions(self):
        geometry = test_geometries.generate_two_element_geometry()
        elements = array((1,))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint1 = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_Z1_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT 1")
        constraint2 = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_X0_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT 2")
        constraints = vtkbone.vtkboneConstraintCollection()
        constraints.AddItem(constraint1)
        constraints.AddItem(constraint2)
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.DistributeForceConstraintsToNodes(
                geometry, constraints)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        self.assertTrue(alltrue(ids == (4,6,8,9,10,11)))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        expected_values = 0.1*array((1/4,1/4,1/2,1/4,1/2,1/4))
        self.assertTrue(alltrue(values == expected_values))


    def test_distribute_force_constraint_quasi_donut(self):
        geometry = test_geometries.generate_quasi_donut_geometry()
        elements = array((0,1,2,5))
        elements_vtk = numpy_to_vtk(elements, deep=1, array_type=vtk.VTK_ID_TYPE)
        constraint = vtkbone.vtkboneConstraintUtilities.CreateAppliedLoad(
           elements_vtk, vtkbone.vtkboneConstraint.FACE_Z0_DISTRIBUTION, 2, 0.1, "TEST FORCE CONSTRAINT")
        self.assertFalse(constraint is None)
        nodes_constraint = vtkbone.vtkboneConstraintUtilities.DistributeConstraintToNodes(
                geometry, constraint)
        self.assertEqual(nodes_constraint.GetConstraintType(), vtkbone.vtkboneConstraint.FORCE)
        self.assertEqual(nodes_constraint.GetConstraintAppliedTo(), vtkbone.vtkboneConstraint.NODES)
        ids_vtk = nodes_constraint.GetIndices()
        self.assertFalse(ids_vtk is None)
        ids = vtk_to_numpy(ids_vtk)
        expected_ids = array((0,1,2,3,6,7,8,9,12,13))
        self.assertTrue(alltrue(ids == expected_ids))
        senses_vtk = nodes_constraint.GetAttributes().GetArray("SENSE")
        self.assertFalse(senses_vtk is None)
        senses = vtk_to_numpy(senses_vtk)
        self.assertTrue(alltrue(senses == 2))
        values_vtk = nodes_constraint.GetAttributes().GetArray("VALUE")
        self.assertFalse(values_vtk is None)
        values = vtk_to_numpy(values_vtk)
        expected_values = 0.1*array((1/4,1/2,1/2,1/4,1/2,3/4,1/2,1/4,1/4,1/4))
        self.assertTrue(alltrue(values == expected_values))


if __name__ == '__main__':
    unittest.main()
