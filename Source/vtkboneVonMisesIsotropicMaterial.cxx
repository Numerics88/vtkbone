#include "vtkboneVonMisesIsotropicMaterial.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneVonMisesIsotropicMaterial);

//----------------------------------------------------------------------------
vtkboneVonMisesIsotropicMaterial::vtkboneVonMisesIsotropicMaterial()
  :
  YieldStrength (10000.0)
{
}

//----------------------------------------------------------------------------
vtkboneVonMisesIsotropicMaterial::~vtkboneVonMisesIsotropicMaterial()
{
}

//----------------------------------------------------------------------------
void vtkboneVonMisesIsotropicMaterial::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "YieldStrength: " << this->YieldStrength << "\n";
}

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneVonMisesIsotropicMaterial::Copy()
{
  vtkboneVonMisesIsotropicMaterial* new_mat = vtkboneVonMisesIsotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulus(this->YoungsModulus);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  new_mat->SetYieldStrength(this->YieldStrength);
  return new_mat;
}

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneVonMisesIsotropicMaterial::ScaledCopy(double factor)
{
  vtkboneVonMisesIsotropicMaterial* new_mat = vtkboneVonMisesIsotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulus(factor*this->YoungsModulus);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  new_mat->SetYieldStrength(factor*this->YieldStrength);
  return new_mat;
}
