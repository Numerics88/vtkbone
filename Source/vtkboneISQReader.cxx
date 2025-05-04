#include "vtkboneISQReader.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkShortArray.h"
#include "vtkSignedCharArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkSmartPointer.h"
#include "AimIO/AimIO.h"
#include "AimIO/IsqIO.h"
#include <cassert>


vtkStandardNewMacro (vtkboneISQReader);

//-----------------------------------------------------------------------
vtkboneISQReader::vtkboneISQReader()
{
  this->FileName = NULL;
  this->DataOnCells = -1;  // -1 indictates user has not set it yet.
  this->Error = 0;
  this->Dimension[0] = 1;
  this->Dimension[1] = 1;
  this->Dimension[2] = 1;
  this->ElementSize[0] = 0.01;
  this->ElementSize[1] = 0.01;
  this->ElementSize[2] = 0.01;
  this->Position[0] = 0;
  this->Position[1] = 0;
  this->Position[2] = 0;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------
vtkboneISQReader::~vtkboneISQReader()
{
  this->SetFileName(NULL);
}

//-----------------------------------------------------------------------
void vtkboneISQReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "Error: " << this->Error << "\n";
  os << indent << "Dimension: " << this->Dimension[0] << ","
     << this->Dimension[1] << ","
     << this->Dimension[2] << "\n";
  os << indent << "ElementSize: " << this->ElementSize[0] << ","
     << this->ElementSize[1] << ","
     << this->ElementSize[2] << "\n";
  os << indent << "Position: " << this->Position[0] << ","
     << this->Position[1] << ","
     << this->Position[2] << "\n";
}

//----------------------------------------------------------------------------
int vtkboneISQReader::RequestInformation (
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

  AimIO::IsqFile reader;

  reader.filename = this->FileName;
  try
  {
    reader.ReadIsqImageInfo();
  }
  catch (const std::exception& e)
  {
    vtkErrorMacro(<< "Error reading " << this->FileName);
    return 0;
  }

  this->ElementSize[0] = reader.slice_increment_um;
  this->ElementSize[1] = reader.slice_increment_um;
  this->ElementSize[2] = reader.slice_thickness_um;
  this->Dimension[0] = reader.dimensions_p[0];
  this->Dimension[1] = reader.dimensions_p[1];
  this->Dimension[2] = reader.dimensions_p[2];

  if (DataOnCells == -1)
  {
    vtkWarningMacro(<< "DataOnCells in vtkboneISQReader not set.  Defaulting "
                       "to data on points.  This default may change in future.");
    DataOnCells = 0;
  }

  // The origin is shifted by 1/2 a voxel if the data is placed on Points.
  // This is so that rendering will be in consistent frames.

  double spacing[3];
  spacing[0] = reader.slice_increment_um;
  spacing[1] = reader.slice_increment_um;
  spacing[2] = reader.slice_thickness_um;

  int extent[6];
  double origin[3];
  if (DataOnCells == 0)  // on Points
  {
    extent[0] = 0;   extent[1] = reader.dimensions_p[0] - 1;
    extent[2] = 0;   extent[3] = reader.dimensions_p[1] - 1;
    extent[4] = 0;   extent[5] = reader.dimensions_p[2] - 1;

    origin[0] = (0.5) * reader.slice_increment_um;
    origin[1] = (0.5) * reader.slice_increment_um;
    origin[2] = (0.5) * reader.slice_thickness_um;
  }
  else // on Cells
  {
    extent[0] = 0;   extent[1] = reader.dimensions_p[0];
    extent[2] = 0;   extent[3] = reader.dimensions_p[1];
    extent[4] = 0;   extent[5] = reader.dimensions_p[2];

    origin[0] = reader.slice_increment_um;
    origin[1] = reader.slice_increment_um;
    origin[2] = reader.slice_thickness_um;
  }

  int scalarType;
  switch (reader.buffer_type)
  {
    case AimIO::IsqFile::ISQFILE_TYPE_SHORT:
      scalarType = VTK_SHORT;
      break;
    default:
      vtkErrorMacro(<< "Unknown ISQ data type.");
  }

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
int vtkboneISQReader::RequestData (vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector* outputVector)
{
  this->Error = 1;
  this->UpdateProgress(0.1);

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkImageData* out = vtkImageData::GetData (outputVector);

  AimIO::IsqFile reader;
  reader.filename = this->FileName;
  try
  {
    reader.ReadIsqImageInfo();
  }
  catch (const std::exception& e)
  {
    vtkErrorMacro(<< "Error reading " << this->FileName);
    return 0;
  }

  this->UpdateProgress(.70);

  vtkIdType N = long_product (reader.dimensions_p);

  int extent[6];
  if (DataOnCells == 0)  // on Points
  {
    extent[0] = 0;   extent[1] = reader.dimensions_p[0] - 1;
    extent[2] = 0;   extent[3] = reader.dimensions_p[1] - 1;
    extent[4] = 0;   extent[5] = reader.dimensions_p[2] - 1;
  }
  else // on Cells
  {
    extent[0] = 0;   extent[1] = reader.dimensions_p[0];
    extent[2] = 0;   extent[3] = reader.dimensions_p[1];
    extent[4] = 0;   extent[5] = reader.dimensions_p[2];
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
      vtkErrorMacro(<< "No information about ISQ data type.");
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
        reader.ReadIsqImageData((short*)(sarray->WriteVoidPointer(0,N)), N);
      }
      catch (const std::exception& e)
      {
        vtkErrorMacro(<< "Error reading " << this->FileName);
        return 0;
      }
      dataArray = sarray;
      break;
    }

    default:
      vtkErrorMacro(<< "Unknown ISQ data type.");
      return 0;
  }
  this->UpdateProgress(.90);

  dataArray->SetName("ISQData");
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
