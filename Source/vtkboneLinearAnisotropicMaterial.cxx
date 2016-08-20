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
    this->StressStrainMatrix[i] = 0;
    }
  // Default is an isotropic stiffness matrix.
  double E = 6829.0;
  double nu = 0.3;
  double c = E / ((1+nu)*(1-2*nu));
  StressStrainMatrix[2*6+2] = StressStrainMatrix[6+1] = StressStrainMatrix[0] = c*(1-nu);
  StressStrainMatrix[2*6+1] = StressStrainMatrix[2*6]
    = StressStrainMatrix[6+2] = StressStrainMatrix[6]
    = StressStrainMatrix[2] = StressStrainMatrix[1] = c*nu;
  StressStrainMatrix[5*6+5] = StressStrainMatrix[4*6+4] = StressStrainMatrix[3*6+3] = 0.5*c*(1-2*nu);
  }

//----------------------------------------------------------------------------
vtkboneLinearAnisotropicMaterial::~vtkboneLinearAnisotropicMaterial()
  {
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterial::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "StressStrainMatrix:\n";
  for (unsigned int i=0; i<6; ++i)
    for (unsigned int j=0; j<6; ++j)
      {
      os << indent << this->StressStrainMatrix[6*i+j];
      if (j == 5)
        { os << "\n"; }
      else
        { os << "\t"; }
      }
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterial::SetStressStrainMatrix (const double* k)
  {
  memcpy (this->StressStrainMatrix, k, sizeof(double)*6*6);
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterial::SetStressStrainMatrix (vtkDataArray* k)
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
      this->StressStrainMatrix[i] = k->GetTuple1(i);
      }
    }
  else if (k->GetNumberOfComponents() == 6)
    {
    for (vtkIdType i=0; i<6; ++i)
      for (vtkIdType j=0; j<6; ++j)
        {
        this->StressStrainMatrix[6*i+j] = k->GetComponent(i,j);
        }
    }
  else
    {
    vtkErrorMacro("Input array must have 1 or 6 components.");
    return;
    }
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterial::GetStressStrainMatrix (double* k)
  {
  memcpy (k, this->StressStrainMatrix, sizeof(double)*6*6);
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterial::GetStressStrainMatrix (vtkDataArray* k)
  {
  vtkSmartPointer<vtkDoubleArray> K = vtkSmartPointer<vtkDoubleArray>::New();
  K->SetNumberOfComponents(6);
  K->SetNumberOfTuples(6);
  this->GetStressStrainMatrix((double*)(K->WriteVoidPointer(0,0)));
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
  new_mat->SetStressStrainMatrix(this->StressStrainMatrix);
  return new_mat;
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearAnisotropicMaterial::ScaledCopy (double factor)
  {
  vtkboneLinearAnisotropicMaterial* new_mat = vtkboneLinearAnisotropicMaterial::New();
  new_mat->SetName(this->Name);
  new_mat->SetStressStrainMatrix(this->StressStrainMatrix);
  for (unsigned int i=0; i<36; ++i)
    {
    new_mat->GetStressStrainMatrix()[i] *= factor;
    }
  return new_mat;
  }
