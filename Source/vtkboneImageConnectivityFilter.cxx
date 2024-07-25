#include "vtkboneImageConnectivityFilter.h"
#include "vtkObjectFactory.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkboneImageConnectivityMap.h"
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
#include "vtkIdList.h"
#include "vtkSmartPointer.h"
#include <limits>

vtkStandardNewMacro(vtkboneImageConnectivityFilter);

template <typename T> T sqr(const T x) {return x*x;}

//-----------------------------------------------------------------------
const char *vtkboneImageConnectivityFilter::GetExtractionModeAsString()
{
  if ( this->ExtractionMode == EXTRACT_SEEDED_REGIONS )
  {
    return "ExtractSeededRegions";
  }
  else if ( this->ExtractionMode == EXTRACT_SPECIFIED_REGIONS )
  {
    return "ExtractSpecifiedRegions";
  }
  else if ( this->ExtractionMode == EXTRACT_ALL_REGIONS )
  {
    return "ExtractAllRegions";
  }
  else if ( this->ExtractionMode == EXTRACT_CLOSEST_POINT_REGION )
  {
    return "ExtractClosestPointRegion";
  }
  else if ( this->ExtractionMode == EXTRACT_REGIONS_OF_SPECIFIED_SIZE )
  {
    return "ExtractRegionsOfSpecifiedSize";
  }
  else if ( this->ExtractionMode == EXTRACT_LARGEST_REGION )
  {
    return "ExtractLargestRegion";
  }
  else
  {
    return "Invalid ExtractionMode";
  }
}

//-----------------------------------------------------------------------
vtkboneImageConnectivityFilter::vtkboneImageConnectivityFilter()
:
  ExtractionMode (EXTRACT_LARGEST_REGION),
  MinimumRegionSize (1),
  NumberOfExtractedRegions (0)
{
  this->Seeds = vtkIdList::New();
  this->SpecifiedRegionIds = vtkIdList::New();
  this->ClosestPoint[0] = 0;
  this->ClosestPoint[1] = 0;
  this->ClosestPoint[2] = 0;
}

//----------------------------------------------------------------------------
vtkboneImageConnectivityFilter::~vtkboneImageConnectivityFilter()
{
  if (Seeds) Seeds->Delete();
  if (SpecifiedRegionIds) SpecifiedRegionIds->Delete();
}

//----------------------------------------------------------------------------
void vtkboneImageConnectivityFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Extraction Mode: ";
  os << this->GetExtractionModeAsString() << "\n";

  os << indent << "Closest Point: (" << this->ClosestPoint[0] << ", "
     << this->ClosestPoint[1] << ", " << this->ClosestPoint[2] << ")\n";

  os << indent << "MinimumRegionSize: " << this->MinimumRegionSize << "\n";
}

//----------------------------------------------------------------------------
void vtkboneImageConnectivityFilter::InitializeSeedList()
{
  this->Modified();
  this->Seeds->Reset();
}

//----------------------------------------------------------------------------
void vtkboneImageConnectivityFilter::AddSeed(vtkIdType id)
{
  this->Modified();
  this->Seeds->InsertNextId(id);
}

//----------------------------------------------------------------------------
void vtkboneImageConnectivityFilter::DeleteSeed(vtkIdType id)
{
  this->Modified();
  this->Seeds->DeleteId(id);
}

//----------------------------------------------------------------------------
void vtkboneImageConnectivityFilter::InitializeSpecifiedRegionList()
{
  this->Modified();
  this->SpecifiedRegionIds->Reset();
}

//----------------------------------------------------------------------------
void vtkboneImageConnectivityFilter::AddSpecifiedRegion(int id)
{
  this->Modified();
  this->SpecifiedRegionIds->InsertNextId(id);
}

//----------------------------------------------------------------------------
void vtkboneImageConnectivityFilter::DeleteSpecifiedRegion(int id)
{
  this->Modified();
  this->SpecifiedRegionIds->DeleteId(id);
}

//----------------------------------------------------------------------------
int vtkboneImageConnectivityFilter::RequestData(
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
  // This is the part that differs from vtkSimpleImageToImageFilter
  if (input->GetCellData()->GetScalars())
  {
    vtkSmartPointer<vtkDataArray> newData = vtkSmartPointer<vtkDataArray>::Take(
            vtkDataArray::CreateDataArray(input->GetCellData()->GetScalars()->GetDataType()));
    newData->SetName(input->GetCellData()->GetScalars()->GetName());
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
    output->AllocateScalars(outInfo);
  }

  this->SimpleExecute(input, output);

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneImageConnectivityFilter::CalculateRegionSizes
(
  vtkUnsignedIntArray* cmap_data,
  unsigned int numRegions,
  vtkIdTypeArray* regionSizes
)
{
  regionSizes->SetNumberOfTuples (numRegions+1);
  for (int r=0; r<numRegions+1; ++r)
    { regionSizes->SetValue(r,0); }
  vtkIdType numPts = cmap_data->GetNumberOfTuples();
  for (vtkIdType i=0; i<numPts; ++i)
  {
    unsigned int r = cmap_data->GetValue(i);
    regionSizes->SetValue (r, regionSizes->GetValue(r) + 1);
  }
  return VTK_OK;
}

//----------------------------------------------------------------------------
void vtkboneImageConnectivityFilter::SimpleExecute
  (vtkImageData* input_image,
   vtkImageData* output_image)
{
  vtkDataArray* out_data = NULL;
  // If there are scalars on the Cells, take those; otherwise take the scalars
  // on the Points.
  vtkDataArray* in_data = input_image->GetCellData()->GetScalars();
  if (in_data == NULL)
  {
    in_data = input_image->GetPointData()->GetScalars();
    out_data = output_image->GetPointData()->GetScalars();
  }
  else
  {
    out_data = output_image->GetCellData()->GetScalars();
  }

  if (in_data == NULL || out_data == NULL)
  {
    vtkErrorMacro(<< "Internal error in vtkboneImageConnectivityFilter");
    return;
  }
  vtkIdType numPts = in_data->GetNumberOfTuples();
  if (out_data->GetNumberOfTuples() != numPts)
  {
    vtkErrorMacro(<< "Internal error in vtkboneImageConnectivityFilter");
    return;
  }
  if (numPts == 0) return;

  // Zero the output.
  for (vtkIdType i=0; i<numPts; ++i)
  {
    out_data->SetTuple1 (i, 0);
  }

  vtkSmartPointer<vtkboneImageConnectivityMap> connectivityMapper =
                        vtkSmartPointer<vtkboneImageConnectivityMap>::New();
  connectivityMapper->SetInputData (input_image);
  connectivityMapper->Update();
  vtkImageData* cmap = connectivityMapper->GetOutput();
  vtkUnsignedIntArray* cmap_data = vtkUnsignedIntArray::SafeDownCast(cmap->GetCellData()->GetScalars());
  if (cmap_data == NULL)
  {
    cmap_data = vtkUnsignedIntArray::SafeDownCast(cmap->GetPointData()->GetScalars());
  }
  if (cmap_data == NULL)
  {
    vtkErrorMacro (<< "Internal error in vtkboneImageConnectivityFilter");
    return;
  }
  if (cmap_data->GetNumberOfTuples() != numPts)
  {
    vtkErrorMacro(<< "Internal error in vtkboneImageConnectivityFilter");
    return;
  }
  unsigned int numRegions = connectivityMapper->GetNumberOfRegions();

  // Return an empty result if no regions
  if (numRegions == 0) return;

  if (this->ExtractionMode == EXTRACT_LARGEST_REGION)
  {
    vtkSmartPointer<vtkIdTypeArray> regionSizes = vtkSmartPointer<vtkIdTypeArray>::New();
    this->CalculateRegionSizes(cmap_data, numRegions, regionSizes);
    unsigned int largestRegion = 0;
    vtkIdType largestSize = 0;
    for (int r=1; r<numRegions+1; ++r)
    {
      if (regionSizes->GetValue(r) > largestSize)
      {
        largestSize = regionSizes->GetValue(r);
        largestRegion = r;
      }
    }
    if (largestRegion == 0)
    {
      vtkErrorMacro (<< "Internal error in vtkboneImageConnectivityFilter");
      return;
    }
    this->SpecifiedRegionIds->SetNumberOfIds(1);
    this->SpecifiedRegionIds->SetId(0,largestRegion);
  }

  if (this->ExtractionMode == EXTRACT_ALL_REGIONS)
  {
    this->SpecifiedRegionIds->SetNumberOfIds(numRegions);
    for (int r=0; r<numRegions; ++r)
    {
      this->SpecifiedRegionIds->SetId(r,r+1);
    }
  }

  if (this->ExtractionMode == EXTRACT_REGIONS_OF_SPECIFIED_SIZE)
  {
    vtkSmartPointer<vtkIdTypeArray> regionSizes = vtkSmartPointer<vtkIdTypeArray>::New();
    this->CalculateRegionSizes(cmap_data, numRegions, regionSizes);
    this->SpecifiedRegionIds->SetNumberOfIds(0);
    for (int r=1; r<numRegions+1; ++r)
    {
      if (regionSizes->GetValue(r) >= this->MinimumRegionSize)
      {
        this->SpecifiedRegionIds->InsertNextId(r);
      }
    }
  }

  if (this->ExtractionMode == EXTRACT_CLOSEST_POINT_REGION)
  {
    int dims[3];
    double origin[3];
    double spacing[3];
    input_image->GetDimensions(dims);
    input_image->GetOrigin(origin);
    input_image->GetSpacing(spacing);
    if (input_image->GetCellData()->GetScalars())
    {
      --dims[0];
      --dims[1];
      --dims[2];
      origin[0] += 0.5*spacing[0];
      origin[1] += 0.5*spacing[1];
      origin[2] += 0.5*spacing[2];
    }
    double x[3];
    vtkIdType index = 0;
    x[2] = origin[2] - this->ClosestPoint[2];
    double closestDistance2 = sqr(dims[0]*spacing[0]) + sqr(dims[1]*spacing[1]) + sqr(dims[2]*spacing[2]);
    unsigned int closestRegion = 0;
    for (int k=0; k<dims[2]; ++k)
    {
      x[1] = origin[1] - this->ClosestPoint[1];
      for (int j=0; j<dims[1]; ++j)
      {
        x[0] = origin[0] - this->ClosestPoint[0];
        for (int i=0; i<dims[0]; ++i)
        {
          if (cmap_data->GetValue(index) != 0)
          {
            double dist2 = sqr(x[0]) + sqr(x[1]) + sqr(x[2]);
            if (dist2 < closestDistance2)
            {
              closestDistance2 = dist2;
              closestRegion = cmap_data->GetValue(index);
            }
          }
          x[0] += spacing[0];
          ++index;
        }
        x[1] += spacing[1];
      }
      x[2] += spacing[2];
    }
    this->SpecifiedRegionIds->SetNumberOfIds(1);
    this->SpecifiedRegionIds->SetId(0,closestRegion);
  }

  if (this->ExtractionMode == EXTRACT_SEEDED_REGIONS)
  {
    this->SpecifiedRegionIds->SetNumberOfIds(0);
    for (int s=0; s< this->Seeds->GetNumberOfIds(); ++s)
    {
      vtkIdType id = this->Seeds->GetId(s);
      if (id > 0 && id < numPts)
      {
        unsigned int region = cmap_data->GetValue(id);
        if (region != 0)
        {
          this->SpecifiedRegionIds->InsertUniqueId(region);
        }
      }
    }
  }

  this->NumberOfExtractedRegions = this->SpecifiedRegionIds->GetNumberOfIds();

  // Now copy input to output for all the specified regions
  for (vtkIdType i=0; i<numPts; ++i)
  {
    for (unsigned int r=0; r<this->NumberOfExtractedRegions; ++r)
    {
      unsigned int regionId = cmap_data->GetValue(i);
      if (regionId == this->SpecifiedRegionIds->GetId(r))
      {
        out_data->SetTuple (i, i, in_data);
        break;
      }
    }
  }

}
