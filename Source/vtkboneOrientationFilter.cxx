#include "vtkboneOrientationFilter.h"
#include "vtkIdTypeArray.h"
#include "vtkPolyDataNormals.h"
#include "vtkSelection.h"
#include "vtkExtractSelection.h"
#include "vtkboneSelectionUtilities.h"
#include "vtkGeometryFilter.h"
#include "vtkInformation.h"
#include "vtkCellData.h"
#include "vtkMath.h"
#include "vtkSmartPointer.h"
#include "vtkInformationVector.h"
#include "vtkExecutive.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro (vtkboneOrientationFilter);

//----------------------------------------------------------------------------
vtkboneOrientationFilter::vtkboneOrientationFilter()
{
  this->NormalVector[0] = 0;
  this->NormalVector[1] = 0;
  this->NormalVector[2] = 1;
}

//----------------------------------------------------------------------------
vtkboneOrientationFilter::~vtkboneOrientationFilter()
{
}

//----------------------------------------------------------------------------
void vtkboneOrientationFilter::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "NormalVector: "
    << NormalVector[0] << ","
    << NormalVector[1] << ","
    << NormalVector[2] << "\n";
}

//----------------------------------------------------------------------------
int vtkboneOrientationFilter::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* input = vtkPolyData::SafeDownCast (inInfo->Get (vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast (outInfo->Get (vtkDataObject::DATA_OBJECT()));

  if (!input || !output)
    {
    vtkErrorMacro (<<"Wrong or not enough inputs/outputs");
    return 0;
    }

  vtkSmartPointer<vtkIdTypeArray> selectedIds = vtkSmartPointer<vtkIdTypeArray>::New();
  // Over-allocate - does no harm on 64 bit systems
  selectedIds->Allocate (input->GetNumberOfCells());

  // Ensure that we have normals.  If not, we need to generate them.
  if (input->GetCellData()->GetNormals() == NULL)
    {
    vtkSmartPointer<vtkPolyDataNormals> normalsGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalsGenerator->ConsistencyOff();
    normalsGenerator->AutoOrientNormalsOff();
    normalsGenerator->ComputePointNormalsOff();
    normalsGenerator->ComputeCellNormalsOn();
    normalsGenerator->SetInputData (input);
    normalsGenerator->Update();
    // Now we will actually replace the input with the result
    // It might seem evil to do this, however this can lead to possible
    // efficiencies in not having to re-calculate normals later.
    // Using temp might not be necessary, but it could possibly eliminate
    // the bug of having reference counts sub-objects of the input going
    // to zero during the copying process.
    input->GetCellData()->SetNormals (
               normalsGenerator->GetOutput()->GetCellData()->GetNormals());
    }
  vtkDataArray* normals = input->GetCellData()->GetNormals();

  // Check dot product of cell normals with specified normal vector
  for (vtkIdType i=0; i<input->GetNumberOfCells(); i++)
    {
    if (vtkMath::Dot (normals->GetTuple3(i), this->NormalVector) > 0)
      {
      selectedIds->InsertNextValue (i);
      }
    }

  // Extract all the marked Cells
  vtkSmartPointer<vtkSelection> selection = vtkSmartPointer<vtkSelection>::New();
  vtkboneSelectionUtilities::CellSelectionFromIds (selection, input, selectedIds);

  vtkSmartPointer<vtkExtractSelection> extractor = vtkSmartPointer<vtkExtractSelection>::New();
  extractor->SetInputData (0, input);
  extractor->SetInputData (1, selection);
  extractor->Update();

  // We need this because the output of vtkExtractSelection is vtkUnstructuredGrid,
  // however, we need to return a vtkPolyData.
  vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
  geometryFilter->SetInputData (extractor->GetOutput());
  geometryFilter->Update();

  output->ShallowCopy (geometryFilter->GetOutput());

  return 1;
}
