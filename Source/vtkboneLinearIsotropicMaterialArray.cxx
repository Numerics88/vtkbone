#include "vtkboneLinearIsotropicMaterialArray.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneLinearIsotropicMaterialArray);

//----------------------------------------------------------------------------
vtkboneLinearIsotropicMaterialArray::vtkboneLinearIsotropicMaterialArray()
{
  this->YoungsModulus = vtkFloatArray::New();
  this->PoissonsRatio = vtkFloatArray::New();
}

//----------------------------------------------------------------------------
vtkboneLinearIsotropicMaterialArray::~vtkboneLinearIsotropicMaterialArray()
{
  if (this->YoungsModulus) {this->YoungsModulus->Delete();}
  if (this->PoissonsRatio) {this->PoissonsRatio->Delete();}
}

//----------------------------------------------------------------------------
void vtkboneLinearIsotropicMaterialArray::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "YoungsModulus:\n";
  this->YoungsModulus->PrintSelf(os,indent.GetNextIndent());
  os << indent << "PoissonsRatio:\n";
  this->PoissonsRatio->PrintSelf(os,indent.GetNextIndent());
}

//----------------------------------------------------------------------------
void vtkboneLinearIsotropicMaterialArray::Resize(vtkIdType size)
{
  this->YoungsModulus->SetNumberOfTuples(size);
  this->PoissonsRatio->SetNumberOfTuples(size);
}

//----------------------------------------------------------------------------
vtkIdType vtkboneLinearIsotropicMaterialArray::GetSize()
{
  vtkIdType size = this->YoungsModulus->GetNumberOfTuples();
  if (size != this->PoissonsRatio->GetNumberOfTuples())
  {
    vtkErrorMacro(<< "Inconsistent array sizes in vtkboneLinearIsotropicMaterialArray.");
    return 0;
  }
  return size;
}

//----------------------------------------------------------------------------
void vtkboneLinearIsotropicMaterialArray::SetYoungsModulus(vtkFloatArray* E)
{
  if (this->YoungsModulus) {this->YoungsModulus->Delete();}
  this->YoungsModulus = E;
  this->YoungsModulus->Register(this);
}

//----------------------------------------------------------------------------
void vtkboneLinearIsotropicMaterialArray::SetPoissonsRatio (vtkFloatArray* nu)
{
  if (this->PoissonsRatio) {this->PoissonsRatio->Delete();}
  this->PoissonsRatio = nu;
  this->PoissonsRatio->Register(this);
}

//----------------------------------------------------------------------------
void vtkboneLinearIsotropicMaterialArray::SetItem
 (vtkIdType k,
  vtkboneLinearIsotropicMaterial* material)
{
  this->YoungsModulus->SetComponent (k, 0, material->GetYoungsModulus());
  this->PoissonsRatio->SetComponent (k, 0, material->GetPoissonsRatio());
}

//----------------------------------------------------------------------------
void vtkboneLinearIsotropicMaterialArray::SetScaledItem
 (vtkIdType k,
  vtkboneLinearIsotropicMaterial* material,
  double factor)
{
  this->YoungsModulus->SetComponent (k, 0, factor * material->GetYoungsModulus());
  this->PoissonsRatio->SetComponent (k, 0, material->GetPoissonsRatio());
}

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearIsotropicMaterialArray::Copy()
{
  vtkboneLinearIsotropicMaterialArray* new_mat = vtkboneLinearIsotropicMaterialArray::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulus(this->YoungsModulus);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  return new_mat;
}

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearIsotropicMaterialArray::ScaledCopy(double factor)
{
  vtkboneLinearIsotropicMaterialArray* new_mat = vtkboneLinearIsotropicMaterialArray::New();
  new_mat->SetName(this->Name);
  vtkIdType size = this->GetSize();
  vtkSmartPointer<vtkFloatArray> E = vtkSmartPointer<vtkFloatArray>::New();
  E->SetNumberOfTuples(size);
  for (vtkIdType i=0; i<size; ++i)
  {
    E->SetComponent(i, 0, factor * this->YoungsModulus->GetComponent(i,0));
  }
  new_mat->SetYoungsModulus(E);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  return new_mat;
}
