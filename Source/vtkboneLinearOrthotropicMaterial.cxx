#include "vtkboneLinearOrthotropicMaterial.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneLinearOrthotropicMaterial);

//----------------------------------------------------------------------------
vtkboneLinearOrthotropicMaterial::vtkboneLinearOrthotropicMaterial()
  :
  YoungsModulusX (6829.0),
  YoungsModulusY (6829.0),
  YoungsModulusZ (6829.0),
  PoissonsRatioYZ (0.3),
  PoissonsRatioZX (0.3),
  PoissonsRatioXY (0.3),
  ShearModulusYZ (6829.0 / (2*(1.0 + 0.3))),
  ShearModulusZX (6829.0 / (2*(1.0 + 0.3))),
  ShearModulusXY (6829.0 / (2*(1.0 + 0.3)))
  {
  }

//----------------------------------------------------------------------------
vtkboneLinearOrthotropicMaterial::~vtkboneLinearOrthotropicMaterial()
  {
  }

//----------------------------------------------------------------------------
void vtkboneLinearOrthotropicMaterial::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "YoungsModulusX: " << this->YoungsModulusX << "\n";
  os << indent << "YoungsModulusY: " << this->YoungsModulusY << "\n";
  os << indent << "YoungsModulusZ: " << this->YoungsModulusZ << "\n";
  os << indent << "PoissonsRatioYZ: " << this->PoissonsRatioYZ << "\n";
  os << indent << "PoissonsRatioZX: " << this->PoissonsRatioZX << "\n";
  os << indent << "PoissonsRatioXY: " << this->PoissonsRatioXY << "\n";
  os << indent << "ShearModulusYZ: " << this->ShearModulusYZ << "\n";
  os << indent << "ShearModulusZX: " << this->ShearModulusZX << "\n";
  os << indent << "ShearModulusXY: " << this->ShearModulusXY << "\n";
  }

//----------------------------------------------------------------------------
double vtkboneLinearOrthotropicMaterial::GetPoissonsRatioZY()
  {
  return this->PoissonsRatioYZ * this->YoungsModulusZ / this->YoungsModulusY;
  }

//----------------------------------------------------------------------------
double vtkboneLinearOrthotropicMaterial::GetPoissonsRatioXZ()
  {
  return this->PoissonsRatioZX * this->YoungsModulusX / this->YoungsModulusZ;
  }

//----------------------------------------------------------------------------
double vtkboneLinearOrthotropicMaterial::GetPoissonsRatioYX()
  {
  return this->PoissonsRatioXY * this->YoungsModulusY / this->YoungsModulusX;
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearOrthotropicMaterial::Copy()
  {
  vtkboneLinearOrthotropicMaterial* new_mat = vtkboneLinearOrthotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulusX(this->YoungsModulusX);
  new_mat->SetYoungsModulusY(this->YoungsModulusY);
  new_mat->SetYoungsModulusZ(this->YoungsModulusZ);
  new_mat->SetPoissonsRatioYZ(this->PoissonsRatioYZ);
  new_mat->SetPoissonsRatioZX(this->PoissonsRatioZX);
  new_mat->SetPoissonsRatioXY(this->PoissonsRatioXY);
  new_mat->SetShearModulusYZ(this->ShearModulusYZ);
  new_mat->SetShearModulusZX(this->ShearModulusZX);
  new_mat->SetShearModulusXY(this->ShearModulusXY);
  return new_mat;
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearOrthotropicMaterial::ScaledCopy(double factor)
  {
  vtkboneLinearOrthotropicMaterial* new_mat = vtkboneLinearOrthotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetYoungsModulusX(factor*this->YoungsModulusX);
  new_mat->SetYoungsModulusY(factor*this->YoungsModulusY);
  new_mat->SetYoungsModulusZ(factor*this->YoungsModulusZ);
  new_mat->SetPoissonsRatioYZ(this->PoissonsRatioYZ);
  new_mat->SetPoissonsRatioZX(this->PoissonsRatioZX);
  new_mat->SetPoissonsRatioXY(this->PoissonsRatioXY);
  new_mat->SetShearModulusYZ(factor*this->ShearModulusYZ);
  new_mat->SetShearModulusZX(factor*this->ShearModulusZX);
  new_mat->SetShearModulusXY(factor*this->ShearModulusXY);
  return new_mat;
  }
