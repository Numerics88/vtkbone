#include "vtkboneGenerateHommingaMaterialTable.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkboneMaterialTable.h"
#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkboneLinearOrthotropicMaterial.h"
#include "vtkboneLinearAnisotropicMaterial.h"
#include "vtkboneLinearIsotropicMaterialArray.h"
#include "vtkboneLinearOrthotropicMaterialArray.h"
#include "vtkboneLinearAnisotropicMaterialArray.h"
#include "vtkSmartPointer.h"
#include <boost/format.hpp>

vtkStandardNewMacro(vtkboneGenerateHommingaMaterialTable);

//----------------------------------------------------------------------------
vtkboneGenerateHommingaMaterialTable::vtkboneGenerateHommingaMaterialTable()
  :
  FirstIndex (1),
  LastIndex (127),
  Exponent (1.7),
  FullScaleMaterial (NULL)
  {
  this->SetNumberOfInputPorts(0);
  this->SetFullScaleMaterial(vtkboneLinearIsotropicMaterial::New());
  }

//----------------------------------------------------------------------------
vtkboneGenerateHommingaMaterialTable::~vtkboneGenerateHommingaMaterialTable()
  {
  this->SetFullScaleMaterial(NULL);
  }

//----------------------------------------------------------------------------
void vtkboneGenerateHommingaMaterialTable::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FirstIndex: " << this->FirstIndex << "\n";
  os << indent << "LastIndex: " << this->LastIndex << "\n";
  os << indent << "Exponent " << this->Exponent << "\n";
  os << indent << "FullScaleMaterial: \n";
  this->FullScaleMaterial->PrintSelf(os, indent.GetNextIndent());
  }

//----------------------------------------------------------------------------
int vtkboneGenerateHommingaMaterialTable::RequestData
  (
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector
  )
  {
  using boost::format;
  
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkboneMaterialTable *output = vtkboneMaterialTable::SafeDownCast(
                            outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
    {
    vtkErrorMacro("No output object.");
    return 0;
    }

  vtkSmartPointer<vtkboneMaterial> ma;

  if (vtkboneLinearAnisotropicMaterial* aniso =
      vtkboneLinearAnisotropicMaterial::SafeDownCast(this->FullScaleMaterial))
    {
    vtkboneLinearAnisotropicMaterialArray* ma_aniso = vtkboneLinearAnisotropicMaterialArray::New();
    ma = vtkSmartPointer<vtkboneMaterial>::Take(ma_aniso);
    int N = this->LastIndex - this->FirstIndex + 1;
    ma_aniso->Resize (N);
    for (int k=0; k<N; ++k)
      {
      double x = (double)(k+1)/(double)(N);
      double factor = pow(x, this->Exponent);
      ma_aniso->SetScaledItem (k, aniso, factor);
      }    
    }

  else if (vtkboneLinearOrthotropicMaterial* ortho =
           vtkboneLinearOrthotropicMaterial::SafeDownCast(this->FullScaleMaterial))
    {
    vtkboneLinearOrthotropicMaterialArray* ma_ortho = vtkboneLinearOrthotropicMaterialArray::New();
    ma = vtkSmartPointer<vtkboneMaterial>::Take(ma_ortho);
    int N = this->LastIndex - this->FirstIndex + 1;
    ma_ortho->Resize (N);
    for (int k=0; k<N; ++k)
      {
      double x = (double)(k+1)/(double)(N);
      double factor = pow(x, this->Exponent);
      ma_ortho->SetScaledItem (k, ortho, factor);
      }
    }

  else if (vtkboneLinearIsotropicMaterial* iso =
           vtkboneLinearIsotropicMaterial::SafeDownCast(this->FullScaleMaterial))
    {
    vtkboneLinearIsotropicMaterialArray* ma_iso = vtkboneLinearIsotropicMaterialArray::New();
    ma = vtkSmartPointer<vtkboneMaterial>::Take(ma_iso);
    int N = this->LastIndex - this->FirstIndex + 1;
    ma_iso->Resize (N);
    for (int k=0; k<N; ++k)
      {
      double x = (double)(k+1)/(double)(N);
      double factor = pow(x, this->Exponent);
      ma_iso->SetScaledItem (k, iso, factor);
      }
    }

  else
    {
    vtkErrorMacro ("Full scale material not set or incompatible with Homminga material table.");
    return 0;
    }

  std::string name = this->FullScaleMaterial->GetName();
  name += "_homminga";
  ma->SetName (name.c_str());

  output->RemoveAll();
  output->AddMaterial (this->FirstIndex, ma);

  return 1;
  }
