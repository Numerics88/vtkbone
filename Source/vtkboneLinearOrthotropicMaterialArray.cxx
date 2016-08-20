#include "vtkboneLinearOrthotropicMaterialArray.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneLinearOrthotropicMaterialArray);

//----------------------------------------------------------------------------
vtkboneLinearOrthotropicMaterialArray::vtkboneLinearOrthotropicMaterialArray()
  {
  this->YoungsModulus = vtkFloatArray::New();
  this->PoissonsRatio = vtkFloatArray::New();
  this->ShearModulus = vtkFloatArray::New();
  }

//----------------------------------------------------------------------------
vtkboneLinearOrthotropicMaterialArray::~vtkboneLinearOrthotropicMaterialArray()
  {
  if (this->YoungsModulus) {this->YoungsModulus->Delete();}
  if (this->PoissonsRatio) {this->PoissonsRatio->Delete();}
  if (this->ShearModulus) {this->ShearModulus->Delete();}
  }

//----------------------------------------------------------------------------
void vtkboneLinearOrthotropicMaterialArray::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "YoungsModulus:\n";
  this->YoungsModulus->PrintSelf(os,indent.GetNextIndent());
  os << indent << "PoissonsRatio:\n";
  this->PoissonsRatio->PrintSelf(os,indent.GetNextIndent());
  os << indent << "ShearModulus:\n";
  this->ShearModulus->PrintSelf(os,indent.GetNextIndent());
  }

//----------------------------------------------------------------------------
void vtkboneLinearOrthotropicMaterialArray::Resize(vtkIdType size)
  {
  this->YoungsModulus->SetNumberOfComponents(3);
  this->YoungsModulus->SetNumberOfTuples(size);
  this->PoissonsRatio->SetNumberOfComponents(3);
  this->PoissonsRatio->SetNumberOfTuples(size);
  this->ShearModulus->SetNumberOfComponents(3);
  this->ShearModulus->SetNumberOfTuples(size);
  }

//----------------------------------------------------------------------------
vtkIdType vtkboneLinearOrthotropicMaterialArray::GetSize()
  {
  vtkIdType size = this->YoungsModulus->GetNumberOfTuples();
  if (size != this->PoissonsRatio->GetNumberOfTuples() ||
      size != this->ShearModulus->GetNumberOfTuples())
    {
    vtkErrorMacro(<< "Inconsistent array sizes in vtkboneLinearOrthotropicMaterialArray.");
    return 0;
    }
  return size;
  }

//----------------------------------------------------------------------------
void vtkboneLinearOrthotropicMaterialArray::SetYoungsModulus(vtkFloatArray* E)
  {
  if (this->YoungsModulus) {this->YoungsModulus->Delete();}
  this->YoungsModulus = E;
  this->YoungsModulus->Register(this);
  }

//----------------------------------------------------------------------------
void vtkboneLinearOrthotropicMaterialArray::SetPoissonsRatio (vtkFloatArray* nu)
  {
  if (this->PoissonsRatio) {this->PoissonsRatio->Delete();}
  this->PoissonsRatio = nu;
  this->PoissonsRatio->Register(this);
  }

//----------------------------------------------------------------------------
void vtkboneLinearOrthotropicMaterialArray::SetShearModulus(vtkFloatArray* G)
  {
  if (this->ShearModulus) {this->ShearModulus->Delete();}
  this->ShearModulus = G;
  this->ShearModulus->Register(this);
  }

//----------------------------------------------------------------------------
void vtkboneLinearOrthotropicMaterialArray::SetItem
 (vtkIdType k,
  vtkboneLinearOrthotropicMaterial* material)
  {
  this->YoungsModulus->SetComponent(k, 0, material->GetYoungsModulusX());
  this->YoungsModulus->SetComponent(k, 1, material->GetYoungsModulusY());
  this->YoungsModulus->SetComponent(k, 2, material->GetYoungsModulusZ());
  this->PoissonsRatio->SetComponent(k, 0, material->GetPoissonsRatioYZ());
  this->PoissonsRatio->SetComponent(k, 1, material->GetPoissonsRatioZX());
  this->PoissonsRatio->SetComponent(k, 2, material->GetPoissonsRatioXY());
  this->ShearModulus->SetComponent (k, 0, material->GetShearModulusYZ());
  this->ShearModulus->SetComponent (k, 1, material->GetShearModulusZX());
  this->ShearModulus->SetComponent (k, 2, material->GetShearModulusXY());
  }

//----------------------------------------------------------------------------
void vtkboneLinearOrthotropicMaterialArray::SetScaledItem
 (vtkIdType k,
  vtkboneLinearOrthotropicMaterial* material,
  double factor)
  {
  this->YoungsModulus->SetComponent(k, 0, factor * material->GetYoungsModulusX());
  this->YoungsModulus->SetComponent(k, 1, factor * material->GetYoungsModulusY());
  this->YoungsModulus->SetComponent(k, 2, factor * material->GetYoungsModulusZ());
  this->PoissonsRatio->SetComponent(k, 0, material->GetPoissonsRatioYZ());
  this->PoissonsRatio->SetComponent(k, 1, material->GetPoissonsRatioZX());
  this->PoissonsRatio->SetComponent(k, 2, material->GetPoissonsRatioXY());
  this->ShearModulus->SetComponent (k, 0, factor * material->GetShearModulusYZ());
  this->ShearModulus->SetComponent (k, 1, factor * material->GetShearModulusZX());
  this->ShearModulus->SetComponent (k, 2, factor * material->GetShearModulusXY());
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearOrthotropicMaterialArray::Copy()
  {
  vtkboneLinearOrthotropicMaterialArray* new_mat = vtkboneLinearOrthotropicMaterialArray::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulus(this->YoungsModulus);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  new_mat->SetShearModulus(this->ShearModulus);
  return new_mat;
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearOrthotropicMaterialArray::ScaledCopy(double factor)
  {
  vtkboneLinearOrthotropicMaterialArray* new_mat = vtkboneLinearOrthotropicMaterialArray::New();
  new_mat->SetName(this->Name);
  vtkIdType size = this->GetSize();
  vtkSmartPointer<vtkFloatArray> E = vtkSmartPointer<vtkFloatArray>::New();
  E->SetNumberOfComponents(3);
  E->SetNumberOfTuples(size);
  for (vtkIdType i=0; i<size; ++i)
    {
    E->SetComponent(i, 0, factor * this->YoungsModulus->GetComponent(i,0));
    E->SetComponent(i, 1, factor * this->YoungsModulus->GetComponent(i,1));
    E->SetComponent(i, 2, factor * this->YoungsModulus->GetComponent(i,2));
    }
  new_mat->SetYoungsModulus(E);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  vtkSmartPointer<vtkFloatArray> G = vtkSmartPointer<vtkFloatArray>::New();
  G->SetNumberOfComponents(3);
  G->SetNumberOfTuples(size);
  for (vtkIdType i=0; i<size; ++i)
    {
    G->SetComponent(i, 0, factor * this->ShearModulus->GetComponent(i,0));
    G->SetComponent(i, 1, factor * this->ShearModulus->GetComponent(i,1));
    G->SetComponent(i, 2, factor * this->ShearModulus->GetComponent(i,2));
    }
  new_mat->SetShearModulus(G);
  return new_mat;
  }
