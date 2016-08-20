#include "vtkboneDecimateImage.h"

#include "vtkImageData.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkObjectFactory.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "n88util/array.hpp"

vtkStandardNewMacro(vtkboneDecimateImage);


//----------------------------------------------------------------------------
int vtkboneDecimateImage::RequestInformation(
  vtkInformation *, vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  vtkImageData *input = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  int inExt[6], outExt[6];
  double inSpacing[3], outSpacing[3];
  double inOrigin[3], outOrigin[3];
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inExt);
  inInfo->Get(vtkDataObject::SPACING(), inSpacing);
  inInfo->Get(vtkDataObject::ORIGIN(), inOrigin);

  int inDims[3], outDims[3];
  inDims[0] = (inExt[1] - inExt[0] + 1);
  inDims[1] = (inExt[3] - inExt[2] + 1);
  inDims[2] = (inExt[5] - inExt[4] + 1);

  for (int k = 0; k < 3; k++)
    {
    outSpacing[k] = inSpacing[k] * 2;
    }

  if (input->GetCellData()->GetScalars())
    {
    for (int k = 0; k < 3; k++)
      {
      outOrigin[k] = inOrigin[k] + inExt[2*k]*inSpacing[k];
      }
    for (int k = 0; k < 3; k++)
      {
      //  Round up to even cells (points-1), then add one more point at boundary.
      outDims[k] = 1 + inDims[k] / 2;
      }
    }
  else
    {
    for (int k = 0; k < 3; k++)
      {
      outOrigin[k] = inOrigin[k] + (inExt[2*k] + 0.5) * inSpacing[k];
      }
    for (int k = 0; k < 3; k++)
      {
      outDims[k] = (inDims[k]+1) / 2;  // Round up to even points
      }
    }
  for (int k = 0; k < 3; k++)
    {
    outExt[2*k] = 0;
    outExt[2*k+1] = outDims[k] - 1;
    }

  // set the output information
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), outExt, 6);
  outInfo->Set(vtkDataObject::SPACING(), outSpacing, 3);
  outInfo->Set(vtkDataObject::ORIGIN(), outOrigin, 3);

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneDecimateImage::RequestUpdateExtent(
  vtkInformation *, vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  // always request the whole extent
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
              inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()),6);

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneDecimateImage::RequestData(
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
    int numComp = 1;
    int dataType = input->GetCellData()->GetScalars()->GetDataType();
    int dims[3];
    output->GetDimensions(dims);
    dims[0] -= 1;
    dims[1] -= 1;
    dims[2] -= 1;
    vtkIdType imageSize = vtkIdType(dims[0])*vtkIdType(dims[1])*vtkIdType(dims[2]);
    // if we currently have scalars then just adjust the size
    vtkDataArray *scalars = output->GetCellData()->GetScalars();
    if (scalars && scalars->GetDataType() == dataType
        && scalars->GetReferenceCount() == 1)
      {
      scalars->SetNumberOfComponents(numComp);
      scalars->SetNumberOfTuples(imageSize);
      // Since the execute method will be modifying the scalars
      // directly.
      scalars->Modified();
      }
    else
      {
      // allocate the new scalars
      scalars = vtkDataArray::CreateDataArray(dataType);
      scalars->SetNumberOfComponents(numComp);
      scalars->SetName("ImageScalars");
      // allocate enough memory
      scalars->SetNumberOfTuples(imageSize);
      output->GetCellData()->SetScalars(scalars);
      scalars->Delete();
      }
    }
  else
    {
    output->AllocateScalars(outInfo);
    }

  return this->SimpleExecute(input, output);
}

//----------------------------------------------------------------------------
void vtkboneDecimateImage::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

template <typename T>
void ImplementDataCopy (
  T* iptr,
  int idims[3],
  T* optr,
  int odims[3])
{
  n88::array<3,T> in(iptr, n88::tuplet<3,size_t>(idims[2],idims[1],idims[0]));
  n88::array<3,T> out(optr, n88::tuplet<3,size_t>(odims[2],odims[1],odims[0]));

  out.zero();

  // Output value is maximum in 2x2x2 input equivalent volume.
  for (int k=0; k<idims[2]; ++k)
    {
    int kk = k/2;
    for (int j=0; j<idims[1]; ++j)
      {
      int jj = j/2;
      for (int i=0; i<idims[0]; ++i)
        {
        int ii = i/2;
        out(kk,jj,ii) = std::max(out(kk,jj,ii),in(k,j,i));
        }
      }
    }
}

//----------------------------------------------------------------------------
int vtkboneDecimateImage::SimpleExecute(vtkImageData* input, vtkImageData* output)
  {
  vtkDataArray* idata = NULL;
  vtkDataArray* odata = NULL;
  int idims[3];
  input->GetDimensions(idims);
  int odims[3];
  output->GetDimensions(odims);

  if (input->GetCellData()->GetScalars())
    {
    idata = input->GetCellData()->GetScalars();
    odata = output->GetCellData()->GetScalars();
    for (int i=0; i<3; ++i) {--idims[i];}
    for (int i=0; i<3; ++i) {--odims[i];}
    }
  else
    {
    idata = input->GetPointData()->GetScalars();
    odata = output->GetPointData()->GetScalars();
    }

  void* iraw = idata->WriteVoidPointer(0,0);
  void* oraw = odata->WriteVoidPointer(0,0);

  n88_assert(idata->GetDataType() == odata->GetDataType());
  n88_assert(idata->GetNumberOfTuples() == idims[0]*idims[1]*idims[2]);
  n88_assert(odata->GetNumberOfTuples() == odims[0]*odims[1]*odims[2]);

  switch (idata->GetDataType())
    {
    case VTK_FLOAT:
      ImplementDataCopy (reinterpret_cast<float*>(iraw), idims,
                         reinterpret_cast<float*>(oraw), odims);
      break;
    case VTK_DOUBLE:
      ImplementDataCopy (reinterpret_cast<double*>(iraw), idims,
                         reinterpret_cast<double*>(oraw), odims);
      break;
    case VTK_CHAR:
    case VTK_SIGNED_CHAR:
      ImplementDataCopy (reinterpret_cast<char*>(iraw), idims,
                         reinterpret_cast<char*>(oraw), odims);
      break;
    case VTK_UNSIGNED_CHAR:
      ImplementDataCopy (reinterpret_cast<unsigned char*>(iraw), idims,
                         reinterpret_cast<unsigned char*>(oraw), odims);
      break;
    case VTK_SHORT:
      ImplementDataCopy (reinterpret_cast<short*>(iraw), idims,
                         reinterpret_cast<short*>(oraw), odims);
      break;
    case VTK_UNSIGNED_SHORT:
      ImplementDataCopy (reinterpret_cast<unsigned short*>(iraw), idims,
                         reinterpret_cast<unsigned short*>(oraw), odims);
      break;
    case VTK_INT:
      ImplementDataCopy (reinterpret_cast<int*>(iraw), idims,
                         reinterpret_cast<int*>(oraw), odims);
      break;
    case VTK_UNSIGNED_INT:
      ImplementDataCopy (reinterpret_cast<unsigned int*>(iraw), idims,
                         reinterpret_cast<unsigned int*>(oraw), odims);
      break;
    default:
      vtkErrorMacro("Unhandled data type in vtkboneDecimateImage.");
      return 0;    
      break;
    }  

  return 1;
  }
