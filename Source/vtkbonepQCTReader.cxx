#include "vtkbonepQCTReader.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkShortArray.h"
#include "vtkCharArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkSmartPointer.h"
#include "pQCTIO/pQCTIO.h"
#include <cassert>


vtkStandardNewMacro (vtkbonepQCTReader);

//-----------------------------------------------------------------------
vtkbonepQCTReader::vtkbonepQCTReader()
{
  this->FileName = NULL;
  this->DataOnCells = -1;  // -1 indictates user has not set it yet.
  this->Error = 0;
  this->ElementSize[0] = 0.01;
  this->ElementSize[1] = 0.01;
  this->Position[0] = 0;
  this->Position[1] = 0;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------
vtkbonepQCTReader::~vtkbonepQCTReader()
{
  this->SetFileName(NULL);
}

//-----------------------------------------------------------------------
void vtkbonepQCTReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "Error: " << this->Error << "\n";
  os << indent << "ElementSize: " << this->ElementSize[0] << ","
     << this->ElementSize[1] << "\n";
  os << indent << "Position: " << this->Position[0] << ","
     << this->Position[1] << "\n";
}

//----------------------------------------------------------------------------
int vtkbonepQCTReader::RequestInformation (
  vtkInformation       * vtkNotUsed( request ),
  vtkInformationVector** vtkNotUsed( inputVector ),
  vtkInformationVector * outputVector)
{
  // Make sure FileName is set
  if (!this->FileName)
  {
    vtkErrorMacro("A Filename must be specified");
    return 0;
  }

  pQCTIO::pQCTFile reader;

  reader.filename = this->FileName;
  try
    {
    reader.ReadImageInfo();
    }
  catch (const std::exception& e)
    {
    vtkErrorMacro(<< "Error reading " << this->FileName);
    return 0;    
    }

  this->ElementSize[0] = reader.VoxelSize;
  this->ElementSize[1] = reader.VoxelSize;
  this->ElementSize[2] = 2;
  this->Dimension[0] = reader.PicMatrixX;
  this->Dimension[1] = reader.PicMatrixY;
  this->Dimension[2] = 0;
  this->Position[0] = 0;
  this->Position[1] = 0;
  this->Position[2] = 0;

  if (DataOnCells == -1)
    {
    vtkWarningMacro(<< "DataOnCells in vtkbonepQCTReader not set.  Defaulting "
                       "to data on points.  This default may change in future.");
    DataOnCells = 0;
    }

  // pQCT 'pos' data is in units of voxels, and converted to vtkImageData->Origin
  // which are in real world coordinates.  Therefore, voxels are converted to
  // world coordinates.
  // The origin is shifted by 1/2 a voxel if the data is placed on Points.
  // This is so that rendering will be in consistent frames.

  double spacing[3];
  spacing[0] = reader.VoxelSize;
  spacing[1] = reader.VoxelSize;
  spacing[2] = 2;

  int extent[6];
  double origin[3];
  if (DataOnCells == 0)  // on Points
    {
    extent[0] = 0;   extent[1] = reader.PicMatrixX - 1;
    extent[2] = 0;   extent[3] = reader.PicMatrixY - 1;
    extent[4] = 0;   extent[5] = 0;

    origin[0] = (0 + 0.5) * reader.VoxelSize;
    origin[1] = (0 + 0.5) * reader.VoxelSize;
    origin[2] = (0 + 0.5) * 2;
    }
  else // on Cells
    {
    extent[0] = 0;   extent[1] = reader.PicMatrixX;
    extent[2] = 0;   extent[3] = reader.PicMatrixY;
    extent[4] = 0;   extent[5] = 0;

    origin[0] = 0 * reader.VoxelSize;
    origin[1] = 0 * reader.VoxelSize;
    origin[2] = (0 + 0.5) * 2;
    }

  // pQCT always short type
  int scalarType = VTK_SHORT;

  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  if (DataOnCells)
    {
    vtkDataObject::SetActiveAttributeInfo(outInfo, vtkDataObject::FIELD_ASSOCIATION_CELLS,
      vtkDataSetAttributes::SCALARS, NULL, scalarType, 1, -1);
    }
  else
    {
    vtkDataObject::SetPointDataActiveScalarInfo (outInfo, scalarType, 1);
    }

  outInfo->Set (vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);
  outInfo->Set (vtkDataObject::SPACING(), spacing, 3);
  outInfo->Set (vtkDataObject::ORIGIN(), origin, 3);

  return 1;
}

//----------------------------------------------------------------------------
int vtkbonepQCTReader::RequestData (vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector* outputVector)
{
  this->Error = 1;
  this->UpdateProgress(0.1);

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkImageData* out = vtkImageData::GetData (outputVector);

  pQCTIO::pQCTFile reader;
  reader.filename = this->FileName;
  try
    {
    reader.ReadImageInfo();
    }
  catch (const std::exception& e)
    {
    vtkErrorMacro(<< "Error reading " << this->FileName);
    return 0;    
    }

  this->UpdateProgress(.70);

  vtkIdType N = reader.number_of_voxels;
  int extent[6];
  if (DataOnCells == 0)  // on Points
    {
    extent[0] = 0;   extent[1] = reader.PicMatrixX - 1;
    extent[2] = 0;   extent[3] = reader.PicMatrixY - 1;
    extent[4] = 0;   extent[5] = 0;
    }
  else // on Cells
    {
    extent[0] = 0;   extent[1] = reader.PicMatrixX;
    extent[2] = 0;   extent[3] = reader.PicMatrixY;
    extent[4] = 0;   extent[5] = 0;
    }
  out->SetExtent(extent);

  vtkSmartPointer<vtkDataArray> dataArray;

  vtkInformation *attrInfo = vtkDataObject::GetActiveFieldInformation(outInfo,
      vtkDataObject::FIELD_ASSOCIATION_CELLS, vtkDataSetAttributes::SCALARS);
  if (!attrInfo)
    {
    attrInfo = vtkDataObject::GetActiveFieldInformation(outInfo,
        vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
    if (!attrInfo)
      {
      return 0;
      vtkErrorMacro(<< "No information about pQCT data type.");
      }
    }      
  int scalarType = attrInfo->Get( vtkDataObject::FIELD_ARRAY_TYPE() );
  
  switch( scalarType )
  {
    case VTK_SHORT:
      {
      vtkSmartPointer<vtkShortArray> sarray = vtkSmartPointer<vtkShortArray>::New();
      sarray->SetNumberOfComponents(1);
      sarray->SetNumberOfValues(N);
      try
        {
        reader.ReadImageData((short*)(sarray->WriteVoidPointer(0,N)), N);
        }
      catch (const std::exception& e)
        {
        vtkErrorMacro(<< "Error reading " << this->FileName);
        return 0;    
        }
      dataArray = sarray;
      }
      break;

    default:
      vtkErrorMacro(<< "Unknown pQCT data type.");
      return 0;
  }
  this->UpdateProgress(.90);

  dataArray->SetName("pQCTData");
  if (DataOnCells == 0)  // on Points
    {
    out->GetPointData()->SetScalars( dataArray );
    }
  else  // on Cells
    {
    out->GetCellData()->SetScalars( dataArray );
    out->GetPointData()->SetScalars( NULL );
    }

  vtkDebugMacro(<< N << " points read.");

  this->Error = 0;

  return 1;
}
