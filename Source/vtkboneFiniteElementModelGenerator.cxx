#include "vtkboneFiniteElementModelGenerator.h"
#include "vtkboneNodeSetsByGeometry.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkInformationStringKey.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkboneMaterialTable.h"
#include "vtkCellData.h"
#include "vtkboneSelectionUtilities.h"
#include "vtkSmartPointer.h"
#include "vtkboneMacros.h"

vtkStandardNewMacro(vtkboneFiniteElementModelGenerator);

//----------------------------------------------------------------------------
vtkboneFiniteElementModelGenerator::vtkboneFiniteElementModelGenerator()
{
  this->AddPedigreeIdArrays = 1;
  this->SetNumberOfInputPorts(2);
}

//----------------------------------------------------------------------------
vtkboneFiniteElementModelGenerator::~vtkboneFiniteElementModelGenerator()
{
}

//----------------------------------------------------------------------------
void vtkboneFiniteElementModelGenerator::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "AddPedigreeIdArrays: " << this->AddPedigreeIdArrays << "\n";
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModelGenerator::FillInputPortInformation(int port, vtkInformation *info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
  }
  if (port == 1)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkboneMaterialTable");
  }
  else
  {
    return 0;
  }
  return 1;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModelGenerator::RequestData
(
  vtkInformation *,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector
)
{
  vtkInformation* inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation* inInfo1 = inputVector[1]->GetInformationObject(0);
  vtkUnstructuredGrid *geometry = vtkUnstructuredGrid::SafeDownCast(
                                 inInfo0->Get(vtkDataObject::DATA_OBJECT()));
  vtkboneMaterialTable* materials = vtkboneMaterialTable::SafeDownCast(
                                 inInfo1->Get(vtkDataObject::DATA_OBJECT()));
  if (!geometry || !materials)
  {
    vtkErrorMacro("Missing input object.");
    return 0;
  }

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkboneFiniteElementModel *output = vtkboneFiniteElementModel::SafeDownCast(
                            outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
  {
    vtkErrorMacro("No output object.");
    return 0;
  }

  if (geometry->GetNumberOfCells() == 0)
  {
    vtkWarningMacro("Zero elements on input to vtkboneFiniteElementModelGenerator");
    return 0;
  }

  // Copy the input to the output.
  output->ShallowCopy(geometry);
  output->BuildLinks();

  // Attach Material Table to model
  output->SetMaterialTable(materials);

  if (this->AddPedigreeIdArrays)
  {
    vtkboneSelectionUtilities::AddPointPedigreeIdsArray(output);
    vtkboneSelectionUtilities::AddCellPedigreeIdsArray(output);
  }

  return 1;
}
