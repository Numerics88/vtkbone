#include "vtkboneImageConnectivityMap.h"
#include "vtkObjectFactory.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkImageData.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkCharArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkSignedCharArray.h"
#include "vtkShortArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkIntArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkIdTypeArray.h"
#include "vtkLongLongArray.h"
#include "vtkUnsignedLongLongArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkIdList.h"
#include "vtkSmartPointer.h"
#include <limits>

vtkStandardNewMacro(vtkboneImageConnectivityMap);

//-----------------------------------------------------------------------
vtkboneImageConnectivityMap::vtkboneImageConnectivityMap()
{
  this->NumberOfRegions = 0;
}

//----------------------------------------------------------------------------
vtkboneImageConnectivityMap::~vtkboneImageConnectivityMap()
{
}

//----------------------------------------------------------------------------
void vtkboneImageConnectivityMap::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


//----------------------------------------------------------------------------
int vtkboneImageConnectivityMap::RequestInformation(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // This is the part that differs from vtkSimpleImageToImageFilter:
  // we always want vtkUnsignedIntArray for the scalars.
  int scalarType = VTK_UNSIGNED_INT;
  int numComp = 1;
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  // if the output is image data
  if (vtkImageData::SafeDownCast
	  (outInfo->Get(vtkDataObject::DATA_OBJECT())))
	{
	vtkDataObject::SetPointDataActiveScalarInfo(outInfo,
												scalarType, numComp);
	}
  return 1;
}


//----------------------------------------------------------------------------
int vtkboneImageConnectivityMap::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the data object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkImageData *input = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  int inExt[6];
  input->GetExtent(inExt);
  // if the input extent is empty then exit
  if (inExt[1] < inExt[0] ||
      inExt[3] < inExt[2] ||
      inExt[5] < inExt[4])
    {
    return 1;
    }

  // Set the extent of the output and allocate memory.
  output->SetExtent(
    outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));
  if (input->GetCellData()->GetScalars())
    {
    vtkSmartPointer<vtkUnsignedIntArray> newData = vtkSmartPointer<vtkUnsignedIntArray>::New();
    newData->SetName("ComponentLabel");
    int dims[3];
    input->GetDimensions(dims);
    // Convert from Point dims to Cell dims
    --dims[0];
    --dims[1];
    --dims[2];
    newData->SetNumberOfTuples(vtkIdType(dims[0])*vtkIdType(dims[1])*vtkIdType(dims[2]));
    output->GetCellData()->SetScalars(newData);
    }
  else
    {
    output->AllocateScalars(VTK_UNSIGNED_INT,1);
    output->GetPointData()->GetScalars()->SetName("ComponentLabel");
    }

  this->SimpleExecute(input, output);

  return 1;
}

//----------------------------------------------------------------------------
void vtkboneImageConnectivityMap::SimpleExecute
  (vtkImageData* input_image,
   vtkImageData* output_image)
{
  int dims[3];
  input_image->GetDimensions(dims);
  vtkUnsignedIntArray* cmap = NULL;
  // If there are scalars on the Cells, take those; otherwise take the scalars
  // on the Points.
  vtkDataArray* in_data = input_image->GetCellData()->GetScalars();
  if (in_data == NULL)
    {
    in_data = input_image->GetPointData()->GetScalars();
    cmap = vtkUnsignedIntArray::SafeDownCast(output_image->GetPointData()->GetScalars());
    }
  else
    {
    cmap = vtkUnsignedIntArray::SafeDownCast(output_image->GetCellData()->GetScalars());
    // Convert from Point dims to Cell dims
    --dims[0];
    --dims[1];
    --dims[2];
    }
  if ( in_data == NULL || cmap == NULL)
    {
    vtkErrorMacro(<< "Internal error in vtkboneImageConnectivityMap");
    return;
    }

  int returnVal;
  switch (in_data->GetDataType())
    {
    case VTK_CHAR:
      returnVal = this->GenerateConnectivityMap<vtkCharArray> (in_data, dims, cmap);
      break;
    case VTK_UNSIGNED_CHAR:
      returnVal = this->GenerateConnectivityMap<vtkUnsignedCharArray> (in_data, dims, cmap);
      break;
    case VTK_SIGNED_CHAR:
      returnVal = this->GenerateConnectivityMap<vtkSignedCharArray> (in_data, dims, cmap);
      break;
    case VTK_SHORT:
      returnVal = this->GenerateConnectivityMap<vtkShortArray> (in_data, dims, cmap);
      break;
    case VTK_UNSIGNED_SHORT:
      returnVal = this->GenerateConnectivityMap<vtkUnsignedShortArray> (in_data, dims, cmap);
      break;
    case VTK_INT:
      returnVal = this->GenerateConnectivityMap<vtkIntArray> (in_data, dims, cmap);
      break;
    case VTK_UNSIGNED_INT:
      returnVal = this->GenerateConnectivityMap<vtkUnsignedIntArray> (in_data, dims, cmap);
      break;
    case VTK_ID_TYPE:
      returnVal = this->GenerateConnectivityMap<vtkIdTypeArray> (in_data, dims, cmap);
      break;
    case VTK_LONG_LONG:
      returnVal = this->GenerateConnectivityMap<vtkLongLongArray> (in_data, dims, cmap);
      break;
    case VTK_UNSIGNED_LONG_LONG:
      returnVal = this->GenerateConnectivityMap<vtkUnsignedLongLongArray> (in_data, dims, cmap);
      break;
    case VTK_FLOAT:
      returnVal = this->GenerateConnectivityMap<vtkFloatArray> (in_data, dims, cmap);
      break;
    case VTK_DOUBLE:
      returnVal = this->GenerateConnectivityMap<vtkDoubleArray> (in_data, dims, cmap);
      break;
    default:
      vtkErrorMacro(<< "Unsupported data type for vtkboneImageConnectivityMap input");
      return;
    }
  if (returnVal != VTK_OK) return;
}

//----------------------------------------------------------------------------
template <typename TArray>
int vtkboneImageConnectivityMap::GenerateConnectivityMap
(
  vtkDataArray* in_data_arg,
  int* dims,
  vtkUnsignedIntArray* cmap
)
{
  TArray* in_data = TArray::SafeDownCast(in_data_arg);
  if (in_data == NULL)
    {
    vtkErrorMacro(<< "Internal error in vtkboneImageConnectivityMap");
    return VTK_ERROR;
    }

  vtkIdType numPts = vtkIdType(dims[0]) * vtkIdType(dims[1]) * vtkIdType(dims[2]);
  cmap->SetNumberOfTuples (numPts);
  for (vtkIdType i=0; i<numPts; ++i)
    { cmap->SetValue(i,0); }

  vtkIdType index = 0;
  this->NumberOfRegions = 0;
  for (int k=0; k<dims[2]; ++k)
    for (int j=0; j<dims[1]; ++j)
      for (int i=0; i<dims[0]; ++i)
        {
        if ((in_data->GetValue(index) != 0) &&
            (cmap->GetValue(index) == 0))
          {
          // This is a voxel that is not empty, but not yet assigned to
          // any component.
          ++this->NumberOfRegions;
          if (this->NumberOfRegions == std::numeric_limits<unsigned int>::max())
            {
            vtkErrorMacro (<< "Number of components exceeds integer representation");
            return VTK_ERROR;
            }
          int returnVal = this->MarkComponent<TArray> (in_data, dims, cmap, i, j, k, this->NumberOfRegions);
          if (returnVal != VTK_OK) return VTK_ERROR;
          }
        ++index;
        }

  return VTK_OK;
}

//----------------------------------------------------------------------------
template <typename TArray>
int vtkboneImageConnectivityMap::MarkComponent
(
  TArray* in_data,
  int* dims,
  vtkUnsignedIntArray* cmap,
  int seed_i,
  int seed_j,
  int seed_k,
  unsigned int label
)
{
  vtkIdType index = (seed_k*dims[1] + seed_j)*dims[0]+ seed_i;
  cmap->SetValue(index, label);

  vtkIdType numPts = vtkIdType(dims[0]) * vtkIdType(dims[1]) * vtkIdType(dims[2]);
  vtkSmartPointer<vtkIdList> wave1 = vtkSmartPointer<vtkIdList>::New();
  wave1->Allocate(numPts/4+1,numPts);
  vtkSmartPointer<vtkIdList> wave2 = vtkSmartPointer<vtkIdList>::New();
  wave2->Allocate(numPts/4+1,numPts);
  vtkIdList* newWave = wave1;
  vtkIdList* oldWave = wave2;

  oldWave->InsertNextId (index);
  vtkIdType dims01 = dims[0]*dims[1];
  do
    {
    newWave->SetNumberOfIds(0);
    for (vtkIdType n=0; n<oldWave->GetNumberOfIds(); ++n)
      {
      vtkIdType id = oldWave->GetId(n);
      // Reconstruct i,j,k
      int i = id % dims[0];
      int k = id/dims[0]; // Integer division
      int j = k % dims[1];
      k /= dims[1];
      if (i > 0)
        {
        int candidateId = id - 1;
        if (in_data->GetValue(candidateId) != 0 && cmap->GetValue(candidateId) == 0)
          {
          cmap->SetValue(candidateId, label);
          newWave->InsertNextId(candidateId);
          }
        }
      if (i < dims[0] - 1)
        {
        int candidateId = id + 1;
        if (in_data->GetValue(candidateId) != 0 && cmap->GetValue(candidateId) == 0)
          {
          cmap->SetValue(candidateId, label);
          newWave->InsertNextId(candidateId);
          }
        }
      if (j > 0)
        {
        int candidateId = id - dims[0];
        if (in_data->GetValue(candidateId) != 0 && cmap->GetValue(candidateId) == 0)
          {
          cmap->SetValue(candidateId, label);
          newWave->InsertNextId(candidateId);
          }
        }
      if (j < dims[1] - 1)
        {
        int candidateId = id + dims[0];
        if (in_data->GetValue(candidateId) != 0 && cmap->GetValue(candidateId) == 0)
          {
          cmap->SetValue(candidateId, label);
          newWave->InsertNextId(candidateId);
          }
        }
      if (k > 0)
        {
        int candidateId = id - dims01;
        if (in_data->GetValue(candidateId) != 0 && cmap->GetValue(candidateId) == 0)
          {
          cmap->SetValue(candidateId, label);
          newWave->InsertNextId(candidateId);
          }
        }
      if (k < dims[2] - 1)
        {
        int candidateId = id + dims01;
        if (in_data->GetValue(candidateId) != 0 && cmap->GetValue(candidateId) == 0)
          {
          cmap->SetValue(candidateId, label);
          newWave->InsertNextId(candidateId);
          }
        }
      }
    std::swap (newWave, oldWave);
    } while (oldWave->GetNumberOfIds() > 0);

  return VTK_OK;
}
