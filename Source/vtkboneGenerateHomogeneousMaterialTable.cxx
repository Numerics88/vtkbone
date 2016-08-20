#include "vtkboneGenerateHomogeneousMaterialTable.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkboneMaterialTable.h"
#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkboneGenerateHomogeneousMaterialTable);

//----------------------------------------------------------------------------
vtkboneGenerateHomogeneousMaterialTable::vtkboneGenerateHomogeneousMaterialTable()
  :
  Material (NULL),
  FirstIndex (1),
  LastIndex (127),
  MaterialIdList (NULL)
  {
  this->SetNumberOfInputPorts(0);
  this->SetMaterial(vtkboneLinearIsotropicMaterial::New());
  }

//----------------------------------------------------------------------------
vtkboneGenerateHomogeneousMaterialTable::~vtkboneGenerateHomogeneousMaterialTable()
  {
  this->SetMaterial(NULL);
  this->SetMaterialIdList(NULL);
  }

//----------------------------------------------------------------------------
void vtkboneGenerateHomogeneousMaterialTable::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Material: \n";
  this->Material->PrintSelf(os, indent.GetNextIndent());
  os << indent << "FirstIndex: " << this->FirstIndex << "\n";
  os << indent << "LastIndex: " << this->LastIndex << "\n";
  os << indent << "MaterialIdList:\n";
  this->MaterialIdList->PrintSelf(os, indent.GetNextIndent());
  }

//----------------------------------------------------------------------------
int vtkboneGenerateHomogeneousMaterialTable::RequestData
  (
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector
  )
  {
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkboneMaterialTable *output = vtkboneMaterialTable::SafeDownCast(
                            outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
    {
    vtkErrorMacro("No output object.");
    return 0;
    }

  output->RemoveAll();

  if (this->MaterialIdList)
    {
    for (vtkIdType i=0; i<this->MaterialIdList->GetNumberOfTuples(); i++)
      {
      int matId = int(this->MaterialIdList->GetTuple1(i));
      if (matId > 0)
        {
        output->AddMaterial(matId, this->Material);
        }
      }
    }
  else
    {
    for (int index=this->FirstIndex; index<=this->LastIndex; index++)
      {
      output->AddMaterial(index, this->Material);
      }
    }

  return 1;
  }
