#include "vtkboneConstraint.h"
#include "vtkObjectFactory.h"
#include "vtkDataSetAttributes.h"
#include "vtkDataObject.h"
#include "vtkIdList.h"
#include "vtkSmartPointer.h"
#include "vtkboneMacros.h"
#include <algorithm>

vtkStandardNewMacro (vtkboneConstraint);

const char* const ConstraintAppliedTo_s[] = {
    "NODES",
    "ELEMENTS"};
vtkboneGetAsStringMacro (vtkboneConstraint, ConstraintAppliedTo);

const char* const ConstraintType_s[] = {
    "FORCE",
    "DISPLACEMENT"};
vtkboneGetAsStringMacro (vtkboneConstraint, ConstraintType);

const char* const Distribution_s[] = {
    "FACE_X0_DISTRIBUTION",
    "FACE_X1_DISTRIBUTION",
    "FACE_Y0_DISTRIBUTION",
    "FACE_Y1_DISTRIBUTION",
    "FACE_Z0_DISTRIBUTION",
    "FACE_Z1_DISTRIBUTION",
    "BODY_DISTRIBUTION"};
vtkboneGetAsStringMacro (vtkboneConstraint, Distribution);


//----------------------------------------------------------------------------
vtkboneConstraint::vtkboneConstraint()
:
  Name                  (NULL),
  Indices               (NULL),
  ConstraintAppliedTo   (NODES),
  ConstraintType        (DISPLACEMENT)
{
  this->Attributes = vtkDataSetAttributes::New();
}

//----------------------------------------------------------------------------
vtkboneConstraint::~vtkboneConstraint()
{
  this->SetName (NULL);
  this->SetIndices(NULL);
  this->Attributes->Delete();
}

//----------------------------------------------------------------------------
void vtkboneConstraint::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Constraint applied to: " << GetConstraintAppliedToAsString (this->ConstraintAppliedTo) << "\n";
  os << indent << "Constraint type: " << GetConstraintTypeAsString (this->ConstraintType) << "\n";
  os << indent << "Indices:";
  if (this->Indices)
    {
    os << "\n";
    this->Indices->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << " NONE\n";
    }
  os << indent << "Attributes:\n";
  this->Attributes->PrintSelf(os,indent.GetNextIndent());
}

//----------------------------------------------------------------------------
void vtkboneConstraint::Initialize ()
{
  this->SetName (NULL);
  this->SetIndices(NULL);
  this->Attributes->Initialize();
  this->SetName(NULL);
  this->ConstraintAppliedTo = NODES;
  this->ConstraintType = DISPLACEMENT;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkboneConstraint::GetMTime()
{
  vtkMTimeType mtime = this->Attributes->GetMTime();
  if (this->Indices)
    {
    vtkMTimeType mtime1 = this->Indices->GetMTime();
    mtime = std::max(mtime, mtime1);
    }
  return mtime;
}

//----------------------------------------------------------------------------
void vtkboneConstraint::RemoveValue (vtkIdType id)
{
  if (this->Indices)
    {
    this->Indices->RemoveTuple(id);
    }

  // Also remove last element from any attributes;
  vtkDataSetAttributes* attr = this->GetAttributes();
  for (int i=0; i < attr->GetNumberOfArrays(); i++)
    {
    attr->GetArray(i)->RemoveTuple(id);
    }
}

//----------------------------------------------------------------------------
void vtkboneConstraint::RemoveLastValue()
{
  if (this->Indices)
    {
    this->Indices->RemoveLastTuple();
    }

  // Also remove last element from any attributes;
  vtkDataSetAttributes* attr = this->GetAttributes();
  for (int i=0; i < attr->GetNumberOfArrays(); i++)
    {
    attr->GetArray(i)->RemoveLastTuple();
    }
}

//----------------------------------------------------------------------------
int vtkboneConstraint::CheckAttributes()
{
  int numPts;
  int numArrays, idx;
  vtkAbstractArray *array;
  int numTuples;
  const char* name;

  numArrays = this->GetAttributes()->GetNumberOfArrays();
  if (numArrays > 0)
    {
    // This call can be expensive.
    numPts = this->GetNumberOfValues();
    for (idx = 0; idx < numArrays; ++idx)
      {
      array = this->GetAttributes()->GetAbstractArray(idx);
      numTuples = array->GetNumberOfTuples();
      name = array->GetName();
      if (name == NULL)
        {
        name = "";
        }
      if (numTuples < numPts)
        {
        vtkErrorMacro("Array " << name << " with "
                      << array->GetNumberOfComponents()
                      << " components, only has " << numTuples << " tuples but there are "
                      << numPts << " points");
        return 1;
        }
      if (numTuples > numPts)
        {
        vtkWarningMacro("Array " << name << " with "
                        << array->GetNumberOfComponents()
                        << " components, has " << numTuples << " tuples but there are only "
                        << numPts << " points");
        }
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkboneConstraint::Merge (vtkboneConstraint* other)
  {
  if ((other->GetConstraintType() != this->ConstraintType) ||
      (other->GetConstraintAppliedTo() != this->ConstraintAppliedTo))
    {
    vtkErrorMacro(<<"Inconsistent constraint types.");
    return 0;
    }

  vtkIdType N_this = this->GetNumberOfValues();
  vtkIdType N_other = other->GetNumberOfValues();

  // First copy Ids
  this->Indices->Resize(N_this + N_other);
  for (vtkIdType i=0; i<N_other; i++)
    {
    this->Indices->InsertNextValue(other->GetIndices()->GetValue(i));
    }

  // Now copy over any matching arrays
  vtkDataSetAttributes* attr_other = other->GetAttributes();
  for (int array_index=0;
       array_index < this->Attributes->GetNumberOfArrays();
       array_index++)
    {
    vtkDataArray* this_array = this->Attributes->GetArray(array_index);
    const char* name = this_array->GetName();
    vtkDataArray* other_array = other->GetAttributes()->GetArray(name);
    if (!other_array)
      {
      vtkErrorMacro(<<"Constraint is missing array " << name);
      return 0;
      }
    if (other_array->GetNumberOfTuples() != N_other)
      {
      vtkErrorMacro(<<"Constraint has inconsistent array length for " << name);
      return 0;
      }
    this_array->Resize(N_this + N_other);
    for (vtkIdType i=0; i<N_other; i++)
      {
      this_array->InsertTuple(N_this + i, i, other_array);
      }
    }

  return 1;
  }

//----------------------------------------------------------------------------
void vtkboneConstraint::ShallowCopy (vtkboneConstraint *meshConstraint)
{
  if (meshConstraint != NULL)
    {

    if (this->Indices)
      {
      this->Indices->UnRegister(this);
      }
    this->Indices = meshConstraint->Indices;
    if (this->Indices)
      {
      this->Indices->Register(this);
      }

    if (this->Attributes)
      {
      this->Attributes->UnRegister(this);
      }
    this->Attributes = meshConstraint->Attributes;
    if (this->Attributes)
      {
      this->Attributes->Register(this);
      }

    this->ConstraintAppliedTo = meshConstraint->ConstraintAppliedTo;
    this->ConstraintType = meshConstraint->ConstraintType;

    }
}

//----------------------------------------------------------------------------
void vtkboneConstraint::DeepCopy (vtkboneConstraint *meshConstraint)
{
  this->Initialize();

  if (meshConstraint != NULL)
    {

    if (this->Indices)
      {
      this->Indices->UnRegister(this);
      }
    this->Indices = NULL;
    if (meshConstraint->GetIndices())
      {
      this->Indices = vtkIdTypeArray::New();
      this->Indices->DeepCopy(meshConstraint->GetIndices());
      this->Indices->Register(this);
      this->Indices->Delete();
      }

    if (this->Attributes)
      {
      this->Attributes->UnRegister(this);
      }
    this->Attributes = NULL;
    if (meshConstraint->GetAttributes())
      {
      this->Attributes = vtkDataSetAttributes::New();
      this->Attributes->DeepCopy(meshConstraint->GetAttributes());
      this->Attributes->Register(this);
      this->Attributes->Delete();
      }

    this->ConstraintAppliedTo = meshConstraint->ConstraintAppliedTo;
    this->ConstraintType = meshConstraint->ConstraintType;
    }

}
