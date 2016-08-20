#include "vtkboneMohrCoulombIsotropicMaterial.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneMohrCoulombIsotropicMaterial);

//----------------------------------------------------------------------------
vtkboneMohrCoulombIsotropicMaterial::vtkboneMohrCoulombIsotropicMaterial()
  :
  C (10000.0),
  Phi (0.0)
  {
  }

//----------------------------------------------------------------------------
vtkboneMohrCoulombIsotropicMaterial::~vtkboneMohrCoulombIsotropicMaterial()
  {
  }

//----------------------------------------------------------------------------
void vtkboneMohrCoulombIsotropicMaterial::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "C: " << this->C << "\n";
  os << indent << "Phi: " << this->Phi << "\n";
  }

//----------------------------------------------------------------------------
void vtkboneMohrCoulombIsotropicMaterial::SetYieldStrengths(double YT, double YC)
  {
  double t = sqrt (YT/YC);
  this->C = YT/(2.0*t);
  this->Phi = vtkMath::Pi()/2.0 - 2.0 * atan(t);
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneMohrCoulombIsotropicMaterial::Copy()
  {
  vtkboneMohrCoulombIsotropicMaterial* new_mat = vtkboneMohrCoulombIsotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulus(this->YoungsModulus);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  new_mat->SetC(this->C);
  new_mat->SetPhi(this->Phi);
  return new_mat;
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneMohrCoulombIsotropicMaterial::ScaledCopy(double factor)
  {
  vtkboneMohrCoulombIsotropicMaterial* new_mat = vtkboneMohrCoulombIsotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulus(factor*this->YoungsModulus);
  new_mat->SetPoissonsRatio(this->PoissonsRatio);
  new_mat->SetC(factor*this->C);
  new_mat->SetPhi(this->Phi);
  return new_mat;
  }
