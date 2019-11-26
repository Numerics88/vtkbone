/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneFiniteElementModel - data model for finite element meshes
// .SECTION Description
//
// vtkboneFiniteElementModel represents a finite element model.
// vtkboneFiniteElementModel is a subclass of vtkUnstructuredGrid.  The
// underlying vtkUnstructuredGrid is the complete set of nodes and elements
// of the finite element model and thus specifies the geometry.
//
// Named subsets of nodes and elements may be defined.
//
// Optionally contains constraints (such as boundary conditions and applied
// loads), stored as vtkboneConstraintCollection.
//
// Optionally contains a material property table, stored as vtkboneMaterialTable.
//
// This object is 0-indexed on all arrays.  Where output is required to be
// 1-indexed, translation is performed in the appropriate writer object
// (e.g. vtkboneN88ModelWriter).
//
// The topology is as specified by VTK.  For convenience, the topologies
// of the commonly used VTK_VOXEL and VTK_HEXAHEDRON are shown below.
//
// @verbatim
//   VTK_VOXEL
//           6---------7
//          /|        /|
//         / |       / |
//        /  |      /  |
//       4---------5   |
//       |   |     |   |
//       |   2-----|---3
//       |  /      |  /
//       |/        | /     z  y
//       0---------1/      | /
//                         |/
//                         .--->x
//
//   VTK_HEXAHEDRON
//           7---------6
//          /|        /|
//         / |       / |
//        /  |      /  |
//       4---------5   |
//       |   |     |   |
//       |   3-----|---2
//       |  /      |  /
//       |/        | /     z  y
//       0---------1/      | /
//                         |/
//                         .--->x
// @endverbatim
//
// .SECTION See Also
// vtkboneConstraint vtkboneSolverParameters vtkboneMaterialTable
// vtkboneFiniteElementModelGenerator

#ifndef __vtkboneFiniteElementModel_h
#define __vtkboneFiniteElementModel_h

#include "vtkUnstructuredGrid.h"
#include "vtkboneWin32Header.h"
#include "vtkSmartPointer.h"

// Forward declarations
class vtkIdTypeArray;
class vtkCharArray;
class vtkDataArrayCollection;
class vtkboneConstraint;
class vtkboneConstraintCollection;
class vtkboneMaterialTable;

class VTKBONE_EXPORT vtkboneFiniteElementModel : public vtkUnstructuredGrid
{
public:
  static vtkboneFiniteElementModel* New();
  vtkTypeMacro(vtkboneFiniteElementModel, vtkUnstructuredGrid);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Add a node set.
  // Note that the node set must have a name or an error will occur.
  // Any existing node set with the same name will be replaced.
  virtual void AddNodeSet (vtkIdTypeArray* nodeSet);

  // Description:
  // Add an element set.
  // Note that the element set must have a name or an error will occur.
  // Any existing element set with the same name will be replaced.
  virtual void AddElementSet (vtkIdTypeArray* elementSet);

  // Description:
  // Returns a pointer to the named node set.
  // Returns NULL if no such node set exists.
  virtual vtkIdTypeArray* GetNodeSet (const char* nodeSetName);

  // Description:
  // Returns a pointer to the named element set.
  // Returns NULL if no such element set exists.
  virtual vtkIdTypeArray* GetElementSet (const char* elementSetName);

  // Description:
  // Remove a node set.
  // Returns VTK_ERROR if the named node set does not exist.
  virtual int RemoveNodeSet (const char* nodeSetName);

  // Description:
  // Remove an element set.
  // Returns VTK_ERROR if the named element set does not exist.
  virtual int RemoveElementSet (const char* elementSetName);

  // Description:
  // Returns the cell Ids of all elements that contain at least one point
  // identified by the specified node set.
  // The first version returns VTK_ERROR on error.
  // The second version returns NULL if no node set with the specified name
  // exists.
  // You are responsible for deleting the returned object.
  virtual int GetAssociatedElementsFromNodeSet (const char *nodeSetName,
                                             vtkIdTypeArray* ids);
  virtual vtkIdTypeArray* GetAssociatedElementsFromNodeSet (const char *nodeSetName);

  // Description:
  // Return a vtkUnstructuredGrid corresponding to the specified NodeSet.
  // The result is a dataset of vertices on the selected nodes.
  // This is useful for rendering the NodeSet.
  // Note: The Points list of the returned object is NOT the same as the
  // vtkboneFiniteElementModel from which it is derived.  However, you can
  // still trace back the original node (point) ids from the PointData
  // attribute PedigreeIds array, providing the vtkboneFiniteElementModel has
  // such an array (by default it does).
  // WARNING: Duplicate points in the node set will be merged.
  // The first version returns VTK_ERROR on error.
  // The second version returns NULL if no node set with the specified name exists.
  // You are responsible for deleting the returned object.
  virtual int DataSetFromNodeSet (const vtkIdTypeArray *nodeSet,
                                  vtkUnstructuredGrid* data);
  virtual vtkUnstructuredGrid* DataSetFromNodeSet (const vtkIdTypeArray *nodeSet);
  virtual int DataSetFromNodeSet (const char *nodeSetName,
                                  vtkUnstructuredGrid* data);
  virtual vtkUnstructuredGrid* DataSetFromNodeSet (const char *nodeSetName);

  // Description:
  // Return a vtkUnstructuredGrid corresponding to the specified ElementSet.
  // The result is a subset of the cells of the model.
  // This is useful for rendering the ElementSet.
  // Note: The Points list of the returned object is NOT the same as the
  // vtkboneFiniteElementModel from which it is derived.  However, you can
  // still trace back the original node (point) ids from the PointData
  // attribute PedigreeIds array, providing the vtkboneFiniteElementModel has
  // such an array (by default it does).
  // WARNING: Duplicate points or cells in the element set will be merged.
  // The first version returns VTK_ERROR on error.
  // The second version returns NULL if no element set with the specified name exists.
  // You are responsible for deleting the returned object.
  virtual int DataSetFromElementSet(const vtkIdTypeArray *elementSet,
                                   vtkUnstructuredGrid* data);
  virtual vtkUnstructuredGrid* DataSetFromElementSet(const vtkIdTypeArray *elementSet);
  virtual int DataSetFromElementSet(const char *elementSetName,
                                   vtkUnstructuredGrid* data);
  virtual vtkUnstructuredGrid* DataSetFromElementSet(const char *elementSetName);

  // Description:
  // Set/get the constraints.
  virtual void SetConstraints(vtkboneConstraintCollection *);
  vtkGetObjectMacro(Constraints, vtkboneConstraintCollection);

  // Description:
  // Set/get the material table.
  virtual void SetMaterialTable(vtkboneMaterialTable *);
  vtkGetObjectMacro(MaterialTable, vtkboneMaterialTable);

  // Description:
  // Set/get the convergence set. This is optional.
  virtual void SetConvergenceSet(vtkboneConstraint *);
  vtkGetObjectMacro(ConvergenceSet, vtkboneConstraint);

  // Description:
  // Specifies the type of elements in the model.
  enum ElementType_t {
    N88_UNKNOWN,
    N88_TETRAHEDRON,
    N88_HEXAHEDRON,
    N88_MIXED,
    NUMBER_OF_ElementType};
  // Description:
  // Return a string describing the value of ElementType_t
  static const char* GetElementTypeAsString (int arg);

  // Description:
  // Determine the correct element type.
  // This call can be computationally expensive, so avoid calling it
  // several times if possible.
  virtual int GetElementType();

  // Description:
  // A convenience function that returns all cell points in one array.
  // This is usually only useful if all cells are of the same type.
  // The second version returns NULL if no element set with the specified name exists.
  // You are responsible for deleting the returned object.  
  virtual void GetAllCellPoints (vtkIdTypeArray* allCellPoints);
  virtual vtkIdTypeArray* GetAllCellPoints ();

  // Description:
  // Generates a displacement Constraint along the specified axis sense with the
  // specified amount of displacement.
  // The variations of this method allow the nodes to be specified as
  // a vtkIdTypeArray list of node Ids, as a single node Id, or as a named
  // existing node set.
  // The Constraint is added to this object's constraint collection with
  // the specified name.  If an existing constraint has the same name, then
  // the new constraints are added to it.
  virtual int ApplyBoundaryCondition(
      vtkIdTypeArray* nodeIds,
      vtkDataArray* senses,
      vtkDataArray* displacements,
      const char* arg_constraintName);
  virtual int ApplyBoundaryCondition(
      vtkIdTypeArray* nodeIds,
      int sense,
      double displacement,
      const char* arg_constraintName);
  virtual int ApplyBoundaryCondition(
      vtkIdType nodeId,
      int sense,
      double displacement,
      const char* arg_constraintName);
  virtual int ApplyBoundaryCondition(
      const char* nodeSetName,
      int sense,
      double displacement,
      const char* arg_constraintName);

  // Description:
  // Generates a fixed Constraint, with displacement 0 applied to all senses
  // of the specified nodes.
  // This is a convenience function, as the same result can be obtained
  // by calling ApplyBoundaryCondition for each axis, with
  // displacement zero.
  // The variations of this method allow the nodes to be specified as
  // a vtkIdTypeArray list of node Ids, as a single node Id, or as a named
  // existing node set.
  // The Constraint is added to this object's constraint collection with
  // the specified name.  If an existing constraint has the same name, then
  // the new constraints are added to it.
  virtual int FixNodes(vtkIdTypeArray* ids, const char* arg_constraintName);
  virtual int FixNodes(vtkIdType id, const char* arg_constraintName);
  virtual int FixNodes(const char* selectionName, const char* arg_constraintName);

  // Description:
  // Generates a force Constraint along the specified axis sense with the
  // specified amount of force.
  // IMPORTANT: For the variants of this method that take a scalar value for
  //            the force, this value is the total force, which will be evenly
  //            distributed input set.
  // The variations of this method allow the elements to be specified as
  // a vtkIdTypeArray list of node Ids, as a single node Id, or as a named
  // existing node set.
  // The distribution of the force on the elements must be specified with
  // the parameter distribution; which must take a value of the enum
  // Distribution_t defined in vtkboneConstraint.
  // The Constraint is added to this object's constraint collection with
  // the specified name.  If an existing constraint has the same name, then
  // the new constraints are added to it.
  virtual int ApplyLoad(
      vtkIdTypeArray* elementIds,
      vtkDataArray* distributions,
      vtkDataArray* senses,
      vtkDataArray* forces,
      const char* arg_constraintName);
  virtual int ApplyLoad(
      vtkIdTypeArray* elementIds,
      int distribution,
      vtkDataArray* senses,
      vtkDataArray* forces,
      const char* arg_constraintName);
  virtual int ApplyLoad(
      vtkIdTypeArray* elementIds,
      int distribution,
      int sense,
      double total_force,
      const char* arg_constraintName);
  virtual int ApplyLoad(
      vtkIdType elementId,
      int distribution,
      int sense,
      double force,
      const char* arg_constraintName);
  virtual int ApplyLoad(
      const char* elementSetName,
      int distribution,
      int sense,
      double total_force,
      const char* arg_constraintName);

  // Description:
  // Given a constraint, generate the complementary convergence set
  // from it. For example, for a constraint on displacements, a
  // convergence set of force will be generated.
  virtual int ConvergenceSetFromConstraint(vtkboneConstraint* constraint);
  virtual int ConvergenceSetFromConstraint(const char* constraintName);

  // Description:
  // Return a Dataset consisting of the nodes or cells associated with
  // the constraint.
  // If the constraint is a cell constraint, you will get that subset of cells.
  // If the constraint is a node constraint, you will get a dataset of
  // vertices on the constrained nodes.
  // The first version returns VTK_ERROR on error.
  // The second version returns NULL if no constraint with the specified name exists.
  // You are responsible for deleting the returned object.
  virtual int DataSetFromConstraint(vtkboneConstraint* arg_constraint,
                                   vtkUnstructuredGrid* data);
  virtual vtkUnstructuredGrid* DataSetFromConstraint(vtkboneConstraint* arg_constraint);
  virtual int DataSetFromConstraint(const char *arg_constraintName,
                                   vtkUnstructuredGrid* data);
  virtual vtkUnstructuredGrid* DataSetFromConstraint(const char *arg_constraintName);

  // Description:
  // Set/get the name of the model.
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);

  // Description:
  // Set/get the node sets.
  virtual void SetNodeSets (vtkDataArrayCollection*);
  vtkGetObjectMacro(NodeSets, vtkDataArrayCollection);

  // Description:
  // Set/get the element sets.
  virtual void SetElementSets (vtkDataArrayCollection*);
  vtkGetObjectMacro(ElementSets, vtkDataArrayCollection);

  // Description:
  // Set/get the gauss point data.
  //
  // Each set of gauss point data should be stored as a named vtkFloatArray.
  // The number of components of the array should be set to the number of
  // values per Gauss point (e.g. 1 for scalar data, 6 for stress/strain data).
  // The number of tuples is then the product of the number of elements
  // (i.e. Cells) times the number of gauss points per element.
  virtual void SetGaussPointData(vtkDataArrayCollection *);
  vtkGetObjectMacro(GaussPointData, vtkDataArrayCollection);

  // Description:
  // Set/Get the History.
  // The history should be a list of one-liners, each line starting with
  // the data and followed by the user and/or program and/or object that
  // is creating or modifying the model.
  // If more information is required, add it to the Log field.
  vtkSetStringMacro(History);
  vtkGetStringMacro(History);

  // Description:
  // Append to the History.
  // The date will automatically be added.
  void AppendHistory(const char* s);

  // Description:
  // Detailed information on how the model was created and/or modified.
  vtkSetStringMacro(Log);
  vtkGetStringMacro(Log);

  // Description:
  // Append to the Log.
  // The date will automatically be added.
  void AppendLog(const char* s);

  // Description:
  // Shallow and Deep copy.
  virtual void ShallowCopy(vtkDataObject *src);
  virtual void DeepCopy(vtkDataObject *src);

protected:
  vtkboneFiniteElementModel();
  ~vtkboneFiniteElementModel();

  vtkDataArrayCollection* NodeSets;
  vtkDataArrayCollection* ElementSets;
  vtkDataArrayCollection* GaussPointData;

  vtkboneConstraintCollection *Constraints;
  vtkboneMaterialTable *MaterialTable;
  vtkboneConstraint* ConvergenceSet;

  char* Name;
  char* History;
  char* Log;

private:
  vtkboneFiniteElementModel(const vtkboneFiniteElementModel&);  // Not implemented.
  void operator=(const vtkboneFiniteElementModel&);  // Not implemented.
};

#endif

