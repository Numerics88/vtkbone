/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneConstraintUtilities - Useful methods for creating and
//   manipulating constraints.
//
// .SECTION See Also
// vtkboneConstraint vtkboneConstraintCollection
// vtkboneFiniteElementModel
//

#ifndef __vtkboneConstraintUtilities_h
#define __vtkboneConstraintUtilities_h

#include "vtkObject.h"
#include "vtkboneWin32Header.h"

// forward declarations
class vtkboneConstraint;
class vtkboneConstraintCollection;
class vtkUnstructuredGrid;
class vtkboneFiniteElementModel;
class vtkIdTypeArray;
class vtkDataArray;

class VTKBONE_EXPORT vtkboneConstraintUtilities : public vtkObject
{
public:
  static vtkboneConstraintUtilities* New();
  vtkTypeMacro(vtkboneConstraintUtilities, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Generates a displacement constraint along the specified senses with the
  // specified amount of displacement.
  // The user is responsible for deleting the returned object.
  static vtkboneConstraint* CreateBoundaryCondition(
      vtkIdTypeArray* nodeIds,
      vtkDataArray* senses,
      vtkDataArray* displacements,
      const char* name);
  static vtkboneConstraint* CreateBoundaryCondition(
      vtkIdTypeArray* nodeIds,
      int sense,
      double displacement,
      const char* name);
  static vtkboneConstraint* CreateBoundaryCondition(
      vtkIdType nodeId,
      int sense,
      double displacement,
      const char* name);

  // Description:
  // Generates a fixed constraint, with displacement 0 applied to all senses
  // of the specified nodes.
  // The same result can be obtained
  // by calling CreateBoundaryCondition for each sense, and with
  // displacement zero.
  // The user is responsible for deleting the returned object.
  static vtkboneConstraint* CreateFixedNodes(vtkIdTypeArray* ids, const char* name);
  static vtkboneConstraint* CreateFixedNodes(vtkIdType id, const char* name);

  // Description:
  // Generates a force constraint along the specified senses with the
  // specified amount of force.
  // The distribution of the force on the elements must be specified with
  // the parameter distribution; which must take a value of the enum
  // Distribution_t defined in vtkboneConstraint.
  // The user is responsible for deleting the returned object.
  static vtkboneConstraint* CreateAppliedLoad(
      vtkIdTypeArray* elementIds,
      vtkDataArray* distributions,
      vtkDataArray* senses,
      vtkDataArray* forces,
      const char* name);
  static vtkboneConstraint* CreateAppliedLoad(
      vtkIdTypeArray* elementIds,
      int distribution,
      vtkDataArray* senses,
      vtkDataArray* forces,
      const char* name);
  static vtkboneConstraint* CreateAppliedLoad(
      vtkIdTypeArray* elementIds,
      int distribution,
      int sense,
      double force,
      const char* name);
  static vtkboneConstraint* CreateAppliedLoad(
      vtkIdType elementId,
      int distribution,
      int sense,
      double force,
      const char* name);

  // Description:
  // Gathers all the displacement constraints.
  // In the case of duplicates, one is arbitrarily chosen.
  // The resulting vtkboneConstraint is sorted by index, then by sense.
  // Any input force constraints are ignored.
  // The user is responsible for deleting the returned object.
  static vtkboneConstraint* GatherDisplacementConstraints(
      vtkboneConstraint* arg_constraint);
  static vtkboneConstraint* GatherDisplacementConstraints(
      vtkboneConstraintCollection* arg_constraints);
  static vtkboneConstraint* GatherDisplacementConstraints(
      vtkboneFiniteElementModel* model);

  // Description:
  // Same as GatherDisplacementConstraints, but only takes entries
  // with VALUE of absolute value less than or equal to tol.
  // The user is responsible for deleting the returned object.
  static vtkboneConstraint* GatherZeroValuedDisplacementConstraints(
      vtkboneConstraint* arg_constraint,
      double tol);
  static vtkboneConstraint* GatherZeroValuedDisplacementConstraints(
      vtkboneConstraintCollection* arg_constraints,
      double tol);
  static vtkboneConstraint* GatherZeroValuedDisplacementConstraints(
      vtkboneFiniteElementModel* model,
      double tol);

  // Description:
  // Same as GatherDisplacementConstraints, but only takes entries
  // with VALUE of absolute value greater than tol.
  // The user is responsible for deleting the returned object.
  static vtkboneConstraint* GatherNonzeroDisplacementConstraints(
      vtkboneConstraint* arg_constraint,
      double tol);
  static vtkboneConstraint* GatherNonzeroDisplacementConstraints(
      vtkboneConstraintCollection* arg_constraints,
      double tol);
  static vtkboneConstraint* GatherNonzeroDisplacementConstraints(
      vtkboneFiniteElementModel* model,
      double tol);

  // Description:
  // Distributes the constraint values on elements among the nodes of the
  // elements, according to the DISTRIBUTION field of the constraint.
  // If newConstraintName is not set, "CONSTRAINTS DISTRIBUTED TO NODES"
  // will be used.
  // The resulting vtkboneConstraint is sorted by index, then by sense.
  // This method will also accept constraints applied to nodes, in which case,
  // the are simply sorted, and summed where multiple entries refer to the
  // same node and sense.
  // The user is responsible for deleting the returned object.
  static vtkboneConstraint* DistributeConstraintToNodes(
      vtkUnstructuredGrid* geometry,
      vtkboneConstraint* arg_constraint);

  // Description:
  // Distributes the force constraint values on elements among the nodes of the
  // elements, according to the DISTRIBUTION field of the constraint.
  // If newConstraintName is not set, "FORCE CONSTRAINTS DISTRIBUTED TO NODES"
  // will be used.
  // The resulting vtkboneConstraint is sorted by index, then by sense.
  // This method will also accept force constraints applied to nodes, in which case,
  // the are simply sorted, and summed where multiple entries refer to the
  // same node and sense.
  // Any input displacement constraints are ignored.
  // The user is responsible for deleting the returned object.
  static vtkboneConstraint* DistributeForceConstraintsToNodes(
      vtkUnstructuredGrid* geometry,
      vtkboneConstraintCollection* arg_constraints);
  static vtkboneConstraint* DistributeForceConstraintsToNodes(
      vtkboneFiniteElementModel* model);

protected:
  vtkboneConstraintUtilities();
  ~vtkboneConstraintUtilities();

private:
  vtkboneConstraintUtilities(const vtkboneConstraintUtilities&);  // Not implemented.
  void operator=(const vtkboneConstraintUtilities&);  // Not implemented.
};

#endif

