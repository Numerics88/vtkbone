#include "vtkboneLinearAnisotropicMaterial.h"
#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneLinearAnisotropicMaterial);

//----------------------------------------------------------------------------
vtkboneLinearAnisotropicMaterial::vtkboneLinearAnisotropicMaterial()
  {
  for (unsigned int i=0; i<36; ++i)
    {
    this->StiffnessMatrix[i] = 0;
    }
  // Default is an isotropic stiffness matrix.
  double E = 6829.0;
  double nu = 0.3;
  double c = E / ((1+nu)*(1-2*nu));
  StiffnessMatrix[2*6+2] = StiffnessMatrix[6+1] = StiffnessMatrix[0] = c*(1-nu);
  StiffnessMatrix[2*6+1] = StiffnessMatrix[2*6]
    = StiffnessMatrix[6+2] = StiffnessMatrix[6]
    = StiffnessMatrix[2] = StiffnessMatrix[1] = c*nu;
  StiffnessMatrix[5*6+5] = StiffnessMatrix[4*6+4] = StiffnessMatrix[3*6+3] = 0.5*c*(1-2*nu);
  }

//----------------------------------------------------------------------------
vtkboneLinearAnisotropicMaterial::~vtkboneLinearAnisotropicMaterial()
  {
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterial::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "StiffnessMatrix:\n";
  for (unsigned int i=0; i<6; ++i)
    for (unsigned int j=0; j<6; ++j)
      {
      os << indent << this->StiffnessMatrix[6*i+j];
      if (j == 5)
        { os << "\n"; }
      else
        { os << "\t"; }
      }
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterial::SetStiffnessMatrix (const double* k)
  {
  memcpy (this->StiffnessMatrix, k, sizeof(double)*6*6);
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterial::SetStiffnessMatrix (vtkDataArray* k)
  {
  if (k == NULL)
    {
    vtkErrorMacro("No input object.");
    return;
    }
  if (k->GetNumberOfComponents() == 1)
    {
    for (vtkIdType i=0; i<36; ++i)
      {
      this->StiffnessMatrix[i] = k->GetTuple1(i);
      }
    }
  else if (k->GetNumberOfComponents() == 6)
    {
    for (vtkIdType i=0; i<6; ++i)
      for (vtkIdType j=0; j<6; ++j)
        {
        this->StiffnessMatrix[6*i+j] = k->GetComponent(i,j);
        }
    }
  else
    {
    vtkErrorMacro("Input array must have 1 or 6 components.");
    return;
    }
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterial::GetStiffnessMatrix (double* k)
  {
  memcpy (k, this->StiffnessMatrix, sizeof(double)*6*6);
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterial::GetStiffnessMatrix (vtkDataArray* k)
  {
  vtkSmartPointer<vtkDoubleArray> K = vtkSmartPointer<vtkDoubleArray>::New();
  K->SetNumberOfComponents(6);
  K->SetNumberOfTuples(6);
  this->GetStiffnessMatrix((double*)(K->WriteVoidPointer(0,0)));
  k->SetNumberOfComponents(6);
  k->SetNumberOfTuples(6);
  for (vtkIdType i=0; i<6; ++i)
    for (vtkIdType j=0; j<6; ++j)
      {
      k->SetComponent(i,j, K->GetComponent(i,j));
      }
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearAnisotropicMaterial::Copy()
  {
  vtkboneLinearAnisotropicMaterial* new_mat = vtkboneLinearAnisotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetStiffnessMatrix(this->StiffnessMatrix);
  return new_mat;
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearAnisotropicMaterial::ScaledCopy (double factor)
  {
  vtkboneLinearAnisotropicMaterial* new_mat = vtkboneLinearAnisotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetStiffnessMatrix(this->StiffnessMatrix);
  for (unsigned int i=0; i<36; ++i)
    {
    new_mat->GetStiffnessMatrix()[i] *= factor;
    }
  return new_mat;
  }
