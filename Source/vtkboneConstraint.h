/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneConstraint
    @brief   a constraint for a finite element mesh

  Stores a constrain on a vtkboneFiniteElementModel.  A constraint is
  a generalization of a boundary condition and an applied load, and
  encompases both those cases.  Constraints consist of values
  assigned to a set of nodes or elements, together with some characteristics
  of those values.

  Contains a vtkIdTypeArray 'Indices' that specifies the list of node or
  element IDs.

  This class contains a vtkDataSetAttributes object, accessed with the
  method GetAttributes, that can store scalars and vectors.

  Constraints on nodes, which may be either Displacement or Force
  constraints, require two scalar arrays.  The first is the dimension that is
  displaced (0=x, 1=y, 2=z), and must be named SENSE, and the second is
  gives the displacement or force and must be named VALUE.
  Note that of course you can have multiple entries for the same node,
  but with different SENSE values.

  Constraints on elements, which are typically Force constraints, require
  an additional scalar array to specify the Distribution of the force
  across the nodes of the element; this array must be named
  DISTRIBUTION.

  Note that, although Force constraints may be assigned to either nodes or
  to elements, in most cases you should assign force constraints to
  elements, as this will typically give the expected results.


    @par Example:
  Here is an example of creating a boundary condition:

    @par Example:
  @code
    // Create Ids array
    vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
    ids->SetNumberOfValues(2);
    ids->SetValue(0, 3);   // Node ID 3
    ids->SetValue(1, 5);   // Node ID 5
    // Create SENSE array
    vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
    senses->SetName("SENSE");
    senses->SetNumberOfValues(2);
    senses->SetValue(0, 0);   // X axis
    senses->SetValue(1, 2);   // Z axis
    // Create VALUE array
    vtkSmartPointer<vtkDoubleArray> values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    values->SetNumberOfValues(2);
    values->SetValue(0, 0.1);   // Displacement of 0.1 mm
    values->SetValue(1, 0.1);   // Displacement of 0.1 mm
    // Create Constraint
    vtkSmartPointer<vtkboneConstraint> constraint = vtkSmartPointer<vtkboneConstraint>::New();
    constraint->SetName("a_custom_boundary_condition");
    constraint->SetIndices(ids);
    constraint->SetConstraintType(vtkboneConstraint::DISPLACEMENT);
    constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
    constraint->GetAttributes()->AddArray(senses);
    constraint->GetAttributes()->AddArray(values);
  @endcode

    @par Example:
  Here is an example of creating an applied load:

    @par Example:
  @code
    // Create Ids array
    vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
    ids->SetNumberOfValues(2);
    ids->SetValue(0, 0);   // Element ID 0
    ids->SetValue(1, 3);   // Element ID 3
    // Create SENSE array
    vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
    senses->SetName("SENSE");
    senses->SetNumberOfValues(2);
    senses->SetValue(0, 2);   // Z axis
    senses->SetValue(1, 2);   // Z axis
    // Create VALUE array
    vtkSmartPointer<vtkDoubleArray> values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    values->SetNumberOfValues(2);
    values->SetValue(0, 69.0);   // Force of 69 MPa per element
    values->SetValue(1, 69.0);   // Force of 69 MPa per element
    vtkSmartPointer<vtkIntArray> distributions = vtkSmartPointer<vtkIntArray>::New();
    distributions->SetName("DISTRIBUTION");
    distributions->SetNumberOfValues(2);
    // FACE_Z1_DISTRIBUTION is the top Z face.
    distributions->SetValue(0, vtkboneConstraint::FACE_Z1_DISTRIBUTION);
    distributions->SetValue(1, vtkboneConstraint::FACE_Z1_DISTRIBUTION);
    // Create Constraint
    vtkSmartPointer<vtkboneConstraint> constraint = vtkSmartPointer<vtkboneConstraint>::New();
    constraint->SetName("a_custom_applied_load");
    constraint->SetIndices(ids);
    constraint->SetConstraintType(vtkboneConstraint::FORCE);
    constraint->SetConstraintAppliedTo(vtkboneConstraint::ELEMENTS);
    constraint->GetAttributes()->AddArray(distributions);
    constraint->GetAttributes()->AddArray(senses);
    constraint->GetAttributes()->AddArray(values);
  @endcode

    @sa
  vtkboneFiniteElementModel vtkboneConstraintCollection
*/

#ifndef __vtkboneConstraint_h
#define __vtkboneConstraint_h

#include "vtkObject.h"
#include "vtkIdTypeArray.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkIdTypeArray;
class vtkDataSetAttributes;
class vtkDataArray;

class VTKBONE_EXPORT vtkboneConstraint : public vtkObject
{
public:
  static vtkboneConstraint* New();
  vtkTypeMacro(vtkboneConstraint, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void Initialize();

  enum ConstraintAppliedTo_t {
    NODES,
    ELEMENTS,
    NUMBER_OF_ConstraintAppliedTo
  };

  enum ConstraintType_t {
    FORCE,
    DISPLACEMENT,
    NUMBER_OF_ConstraintType
  };

  enum ConstraintSense_t {
    SENSE_X = 0,
    SENSE_Y = 1,
    SENSE_Z = 2
  };

  enum Distribution_t {
    FACE_X0_DISTRIBUTION,
    FACE_X1_DISTRIBUTION,
    FACE_Y0_DISTRIBUTION,
    FACE_Y1_DISTRIBUTION,
    FACE_Z0_DISTRIBUTION,
    FACE_Z1_DISTRIBUTION,
    BODY_DISTRIBUTION,
    NUMBER_OF_Distribution
  };

  /*! Return a string describing the value of ConstraintAppliedTo */
  static const char* GetConstraintAppliedToAsString(int ConstraintAppliedTo);

  /*! Return a string describing the value of ConstraintType */
  static const char* GetConstraintTypeAsString(int ConstraintType);

  /*! Return a string describing the value of Distribution */
  static const char* GetDistributionAsString(int Distribution);

  //@{
  /*! Set/get the name of the constraint. */
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);
  //@}

  //@{
  /*! Set/get the node set or element set. */
  vtkSetObjectMacro(Indices, vtkIdTypeArray);
  vtkGetObjectMacro(Indices, vtkIdTypeArray);
  //@}

  /*! Datasets are composite objects and need to check each part for MTime */
  vtkMTimeType GetMTime() override;

  //@{
  /*! Set/Get whether constraints are applied to nodes or to elements. */
  vtkSetClampMacro(ConstraintAppliedTo,
                   int, 0, NUMBER_OF_ConstraintAppliedTo);
  vtkGetMacro(ConstraintAppliedTo, int);
  //@}

  //@{
  /*! Set/Get the type of constraint. */
  vtkSetClampMacro(ConstraintType,
                   int, 0, NUMBER_OF_ConstraintType);
  vtkGetMacro(ConstraintType, int);
  //@}

  //@{
  /*! Return number of constraint values. */
  virtual vtkIdType GetNumberOfValues()
      {return this->GetIndices()->GetNumberOfTuples();}
  //@}

  //@{
  /*! These methods remove values from the data array. They shift data and
      resize array, so the data array is still valid after this operation.
      Note, this operation is fairly slow.  Also removes corresponding
      entries from associated fields (e.g. Scalars and/or Vectors). */
  virtual void RemoveValue(vtkIdType id);
  virtual void RemoveLastValue();
  //@}

  /*! Return a pointer to the data attributes. */
  vtkDataSetAttributes *GetAttributes() {return this->Attributes;};

  /*! This method checks to see if the attributes match the geometry.  Many
      filters will crash if the number of tuples in an array is less than
      the number of points. This method returns 1 if there is a mismatch,
      and 0 if everything is ok.  It prints an error if an array is too
      short, and a warning if an array is too long. */
  virtual int CheckAttributes();

  /*! Merges the entries of another vtkConstraint into this one.
      ConstraintAppliedTo and ConstraintType must match. */
  virtual int Merge(vtkboneConstraint* other);

  //@{
  /*! Shallow and Deep copy. WARNING: Using ShallowCopy will result in both
      this and the original object trying to manage the underlying data.  I
      recommend that you either regard both as constants, or that you
      delete the original object after this call. */
  virtual void ShallowCopy(vtkboneConstraint *constraint);
  virtual void DeepCopy(vtkboneConstraint *constraint);
  //@}

protected:
  vtkboneConstraint();
  ~vtkboneConstraint();

  char* Name;
  vtkIdTypeArray* Indices;    // node or element set
  vtkDataSetAttributes* Attributes;   // Scalars, vectors, etc. associated w/ each point

  int ConstraintAppliedTo;
  int ConstraintType;

private:
  vtkboneConstraint(const vtkboneConstraint&);  // Not implemented.
  void operator=(const vtkboneConstraint&);  // Not implemented.
};

#endif

