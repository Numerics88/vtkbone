#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneLinearIsotropicMaterial);

//----------------------------------------------------------------------------
vtkboneLinearIsotropicMaterial::vtkboneLinearIsotropicMaterial()
  :
  YoungsModulus (6829.0),
  PoissonsRatio (0.3)
{
}

//----------------------------------------------------------------------------
vtkboneLinearIsotropicMaterial::~vtkboneLinearIsotropicMaterial()
{
}

//----------------------------------------------------------------------------
void vtkboneLinearIsotropicMaterial::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "YoungsModulus: " << this->YoungsModulus << "\n";
  os << indent << "PoissonsRatio: " << this->PoissonsRatio << "\n";
}

//----------------------------------------------------------------------------
double vtkboneLinearIsotropicMaterial::GetShearModulus()
{
  return this->YoungsModulus / (2*(1.0 + this->PoissonsRatio));
}

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearIsotropicMaterial::Copy()
{
  vtkboneLinearIsotropicMaterial* new_mat = vtkboneLinearIsotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulus(this->YoungsModulus);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  return new_mat;
}

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearIsotropicMaterial::ScaledCopy(double factor)
{
  vtkboneLinearIsotropicMaterial* new_mat = vtkboneLinearIsotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulus(factor*this->YoungsModulus);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  return new_mat;
}
