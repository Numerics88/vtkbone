#include "vtkboneMaximumPrincipalStrainIsotropicMaterial.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneMaximumPrincipalStrainIsotropicMaterial);

//----------------------------------------------------------------------------
vtkboneMaximumPrincipalStrainIsotropicMaterial::vtkboneMaximumPrincipalStrainIsotropicMaterial()
  :
  MaximumTensilePrincipalStrain (0.01),
  MaximumCompressivePrincipalStrain (0.01)
  {
  }

//----------------------------------------------------------------------------
vtkboneMaximumPrincipalStrainIsotropicMaterial::~vtkboneMaximumPrincipalStrainIsotropicMaterial()
  {
  }

//----------------------------------------------------------------------------
void vtkboneMaximumPrincipalStrainIsotropicMaterial::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "MaximumTensilePrincipalStrain: " << this->MaximumTensilePrincipalStrain << "\n";
  os << indent << "MaximumCompressivePrincipalStrain: " << this->MaximumCompressivePrincipalStrain << "\n";
}

//----------------------------------------------------------------------------
void vtkboneMaximumPrincipalStrainIsotropicMaterial::SetYieldStrains(double eYT, double eYC)
  {
  this->MaximumTensilePrincipalStrain = eYT;
  this->MaximumCompressivePrincipalStrain = eYC;
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneMaximumPrincipalStrainIsotropicMaterial::Copy()
  {
  vtkboneMaximumPrincipalStrainIsotropicMaterial* new_mat = vtkboneMaximumPrincipalStrainIsotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulus(this->YoungsModulus);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  new_mat->SetYieldStrains(this->MaximumTensilePrincipalStrain,
                           this->MaximumCompressivePrincipalStrain);
  return new_mat;
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneMaximumPrincipalStrainIsotropicMaterial::ScaledCopy(double factor)
  {
  vtkboneMaximumPrincipalStrainIsotropicMaterial* new_mat = vtkboneMaximumPrincipalStrainIsotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulus(factor*this->YoungsModulus);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  new_mat->SetYieldStrains(this->MaximumTensilePrincipalStrain,
                           this->MaximumCompressivePrincipalStrain);
  return new_mat;
  }
