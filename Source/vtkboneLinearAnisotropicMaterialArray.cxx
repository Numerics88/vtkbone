#include "vtkboneLinearAnisotropicMaterialArray.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneLinearAnisotropicMaterialArray);

//----------------------------------------------------------------------------
vtkboneLinearAnisotropicMaterialArray::vtkboneLinearAnisotropicMaterialArray()
  {
  this->StiffnessMatrixUpperTriangular = vtkFloatArray::New();
  }

//----------------------------------------------------------------------------
vtkboneLinearAnisotropicMaterialArray::~vtkboneLinearAnisotropicMaterialArray()
  {
  if (this->StiffnessMatrixUpperTriangular) {this->StiffnessMatrixUpperTriangular->Delete();}
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterialArray::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "StiffnessMatrixUpperTriangular:\n";
  this->StiffnessMatrixUpperTriangular->PrintSelf(os,indent.GetNextIndent());
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterialArray::Resize(vtkIdType size)
  {
  this->StiffnessMatrixUpperTriangular->SetNumberOfComponents(21);
  this->StiffnessMatrixUpperTriangular->SetNumberOfTuples(size);
  }

//----------------------------------------------------------------------------
vtkIdType vtkboneLinearAnisotropicMaterialArray::GetSize()
  {
  vtkIdType size = this->StiffnessMatrixUpperTriangular->GetNumberOfTuples();
  return size;
  }

//----------------------------------------------------------------------------
void vtkboneLinearAnisotropicMaterialArray::SetStiffnessMatrixUpperTriangular(vtkFloatArray* K)
  {
  if (this->StiffnessMatrixUpperTriangular) {this->StiffnessMatrixUpperTriangular->Delete();}
  this->StiffnessMatrixUpperTriangular = K;
  this->StiffnessMatrixUpperTriangular->Register(this);
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
      this->StiffnessMatrixUpperTriangular->SetComponent(
        k, n, material->GetStiffnessMatrix()[i*6+j]);
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
      this->StiffnessMatrixUpperTriangular->SetComponent(
        k, n, factor * material->GetStiffnessMatrix()[i*6+j]);
      ++n;
      }
  }

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneLinearAnisotropicMaterialArray::Copy()
  {
  vtkboneLinearAnisotropicMaterialArray* new_mat = vtkboneLinearAnisotropicMaterialArray::New();
  new_mat->SetName(this->Name);
  new_mat->SetStiffnessMatrixUpperTriangular(this->StiffnessMatrixUpperTriangular);
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
      K->SetComponent(i, j, factor * this->StiffnessMatrixUpperTriangular->GetComponent(i,j));
      }
    }
  new_mat->SetStiffnessMatrixUpperTriangular(K);
  return new_mat;
  }
