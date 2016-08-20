#include "vtkboneLinearAnisotropicMaterialArray.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneLinearAnisotropicMaterialArray);

//----------------------------------------------------------------------------
vtkboneLinearAnisotropicMaterialArray::vtkboneLinearAnisotropicMaterialArray()
  {
  this->StressStrainMatrixUpperTriangular = vtkFloatArray::New();
  }

//----------------------------------------------------------------------------
vtkboneLinearAnisotropicMaterialArray::~vtkboneLinearAnisotropicMaterialArray()
  {
  if (this->StressStrainMatrixUpperTriangular) {this->StressStrainMatrixUpperTriangular->Delete();}
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterialArray::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "StressStrainMatrixUpperTriangular:\n";
  this->StressStrainMatrixUpperTriangular->PrintSelf(os,indent.GetNextIndent());
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterialArray::Resize(vtkIdType size)
  {
  this->StressStrainMatrixUpperTriangular->SetNumberOfComponents(21);
  this->StressStrainMatrixUpperTriangular->SetNumberOfTuples(size);
  }

//----------------------------------------------------------------------------
vtkIdType vtkboneLinearAnisotropicMaterialArray::GetSize()
  {
  vtkIdType size = this->StressStrainMatrixUpperTriangular->GetNumberOfTuples();
  return size;
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterialArray::SetStressStrainMatrixUpperTriangular(vtkFloatArray* K)
  {
  if (this->StressStrainMatrixUpperTriangular) {this->StressStrainMatrixUpperTriangular->Delete();}
  this->StressStrainMatrixUpperTriangular = K;
  this->StressStrainMatrixUpperTriangular->Register(this);
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterialArray::SetItem
 (vtkIdType k,
  vtkboneLinearAnisotropicMaterial* material)
  {
  vtkIdType n = 0;
  for (vtkIdType i=0; i<6; ++i)
    for (vtkIdType j=0; j<=i; ++j)
      {
      this->StressStrainMatrixUpperTriangular->SetComponent(
        k, n, material->GetStressStrainMatrix()[i*6+j]);
      ++n;
      }
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterialArray::SetScaledItem
 (vtkIdType k,
  vtkboneLinearAnisotropicMaterial* material,
  double factor)
  {
  vtkIdType n = 0;
  for (vtkIdType i=0; i<6; ++i)
    for (vtkIdType j=0; j<=i; ++j)
      {
      this->StressStrainMatrixUpperTriangular->SetComponent(
        k, n, factor * material->GetStressStrainMatrix()[i*6+j]);
      ++n;
      }
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearAnisotropicMaterialArray::Copy()
  {
  vtkboneLinearAnisotropicMaterialArray* new_mat = vtkboneLinearAnisotropicMaterialArray::New();
  new_mat->SetName(this->Name);
  new_mat->SetStressStrainMatrixUpperTriangular(this->StressStrainMatrixUpperTriangular);
  return new_mat;
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearAnisotropicMaterialArray::ScaledCopy(double factor)
  {
  vtkboneLinearAnisotropicMaterialArray* new_mat = vtkboneLinearAnisotropicMaterialArray::New();
  new_mat->SetName(this->Name);
  vtkIdType size = this->GetSize();
  vtkSmartPointer<vtkFloatArray> K = vtkSmartPointer<vtkFloatArray>::New();
  K->SetNumberOfComponents(21);
  K->SetNumberOfTuples(size);
  for (vtkIdType i=0; i<size; ++i)
    {
    for (vtkIdType j=0; j<21; ++j)
      {
      K->SetComponent(i, j, factor * this->StressStrainMatrixUpperTriangular->GetComponent(i,j));
      }
    }
  new_mat->SetStressStrainMatrixUpperTriangular(K);
  return new_mat;
  }
