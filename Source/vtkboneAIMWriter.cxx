#include "vtkboneAIMWriter.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkMath.h"
#include "vtkImageData.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkSmartPointer.h"
#include "AimIO/AimIO.h"
#include <sstream>
#include <string>
#include <time.h>
#include <boost/format.hpp>

using boost::format;
using std::string;
using std::ostringstream;

//-----------------------------------------------------------------------
// Private Utility Stuff

inline int roundFloat (const float x)
{
  return floor (x+0.5f);
}

vtkStandardNewMacro (vtkboneAIMWriter);

//-----------------------------------------------------------------------
vtkboneAIMWriter::vtkboneAIMWriter()
{
  this->FileName = NULL;
  this->NewProcessingLog = 1; // Default is to add the base AIM log info
  this->ProcessingLog = NULL;
}

//----------------------------------------------------------------------------
vtkboneAIMWriter::~vtkboneAIMWriter()
{
  this->SetFileName(0);   // Frees memory
  this->SetProcessingLog(0);   // Frees memory
}

//----------------------------------------------------------------------------
void vtkboneAIMWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << "\n";
  os << indent << "NewProcessingLog: " << this->NewProcessingLog << "\n";
  os << indent << "ProcessingLog: "
     << (this->ProcessingLog ? this->ProcessingLog : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkboneAIMWriter::AppendToLog (const char* s)
{
  // Could be more efficient.  But this will never segfault or leak memory.
  this->SetProcessingLog ((string(ProcessingLog) + s).c_str());
}

//----------------------------------------------------------------------------
void vtkboneAIMWriter::WriteData()
{
  AimIO::AimFile writer;

  // // Do not call any deallocation functions on image!  It does not own its data!!!
  // D3AnyImage image;
  // D3InitAny(image,D1Tundef);
  // D3int          dim,off,pos;
  // D3float        el_size_mm;
  // int            status;
  // int            dimension[3];
  // double          origin[3], spacing[3], conversionError[3];
  // time_t        rawtime;
  // struct        tm *timeinfo;
  // int position[3];

  vtkImageData *temp_input= vtkImageData::SafeDownCast(this->GetInput());

  this->UpdateProgress(.01);

  // Now we are going to make sure that the image data is on the points.
  // The offset is strange: it will be 0,0,0 for data centered at pixel
  // centers with corner offset 0,0,0 (it should be 0.5,0.5,0.5).
  // Despite this strange offset, it all works out in the end.
  int     dimension[3];
  double  origin[3];
  double  spacing[3];
  vtkSmartPointer<vtkImageData> input = vtkSmartPointer<vtkImageData>::New();
  temp_input->GetSpacing(spacing);
  temp_input->GetOrigin(origin);
  temp_input->GetDimensions(dimension);
  if (temp_input->GetPointData()->GetScalars())
    {
    input->ShallowCopy(temp_input);
    input->SetOrigin(origin[0] - 0.5*spacing[0],
                     origin[1] - 0.5*spacing[1],
                     origin[2] - 0.5*spacing[2]);
    }
  else if (temp_input->GetCellData()->GetScalars())
    {
    input->SetSpacing(spacing);
    input->SetOrigin(origin);
    input->SetDimensions(dimension[0] - 1,
                         dimension[1] - 1,
                         dimension[2] - 1);
    input->GetPointData()->SetScalars(temp_input->GetCellData()->GetScalars());
    }
  else
    {
    vtkErrorMacro(<<"Could not get data from input.");
    return;
    }

  vtkDebugMacro(<<"Writing AIM file...");

  if (!this->FileName)
    {
      vtkErrorMacro(<<"An output filename must be specified.");
      return;
    }
  writer.filename = this->FileName;

  // Do this again - they may have changed.
  input->GetDimensions(dimension);
  input->GetSpacing(spacing);
  input->GetOrigin(origin);

  writer.dimensions[0] = dimension[0];
  writer.dimensions[1] = dimension[1];
  writer.dimensions[2] = dimension[2];

  writer.position[0] = roundFloat(origin[0]/spacing[0]);
  writer.position[1] = roundFloat(origin[1]/spacing[1]);
  writer.position[2] = roundFloat(origin[2]/spacing[2]);

  double conversionError[3];
  conversionError[0] = spacing[0]*writer.position[0] - origin[0];
  conversionError[1] = spacing[1]*writer.position[1] - origin[1];
  conversionError[2] = spacing[2]*writer.position[2] - origin[2];

  writer.element_size[0] = spacing[0];
  writer.element_size[1] = spacing[1];
  writer.element_size[2] = spacing[2];

  // if (fabs(conversionError[0]) > 0.000001 ||
  //     fabs(conversionError[1]) > 0.000001 ||
  //     fabs(conversionError[2]) > 0.000001) {
  //
  //       vtkWarningMacro(<<"\n  Possible error when calculating AIM position from image origin."
  //                         "\n  SPACING * POSITION - ORIGIN = ERROR"
  //                         "\n    X: "
  //                       << spacing[0] << " * "
  //                       << position[0] << " - "
  //                       << origin[0] << " = "
  //                       << conversionError[0] << " "
  //                       << (fabs(conversionError[0]) > 0.0000001 ? "<- error?" : "")
  //                       << "\n    Y: "
  //                       << spacing[1] << " * "
  //                       << position[1] << " - "
  //                       << origin[1] << " = "
  //                       << conversionError[1] << " "
  //                       << (fabs(conversionError[1]) > 0.0000001 ? "<- error?" : "")
  //                       << "\n    Z: "
  //                       << spacing[2] << " * "
  //                       << position[2] << " - "
  //                       << origin[2] << " = "
  //                       << conversionError[2] << " "
  //                       << (fabs(conversionError[2]) > 0.0000001 ? "<- error?" : "")
  //                         );
  // }

  time_t rawtime;
  time (&rawtime);
  struct tm *timeinfo = localtime(&rawtime);
  this->UpdateProgress(.05);

  // Note: This is a dummy log, and it is important that certain parameters be set so that other
  // Scanco software can properly read the data.  For example, 'mu_scaling' and 'patient name'.
  if (this->NewProcessingLog) {
    ostringstream newLog;
    newLog << "! \n";
    newLog << "! Processing Log \n";
    newLog << "! \n";
    newLog << "!-------------------------------------------------------------------------------\n";
    newLog << "Created by                    vtkboneAIMWriter\n";
    newLog << format("%-30s%-50s\n") % "Time" % asctime(timeinfo);
    newLog << "Original file                 DK0:[MICROCT.DATA.00000062.00001080]C0001021.ISQ\n";
    newLog << "!-------------------------------------------------------------------------------\n";
    newLog << "Site                                                1\n";
    newLog << "Scanner ID                                       5001\n";
    newLog << "Scanner type                                        9\n";
    newLog << "Position Slice 1 [um]                          120000\n";
    newLog << "No. samples                                      1536\n";
    newLog << "No. projections per 180                           750\n";
    newLog << "Scan Distance [um]                             777952\n";
    newLog << "Integration time [us]                          100000\n";
    newLog << "Reference line [um]                                 0\n";
    newLog << "Reconstruction-Alg.                                 3\n";
    newLog << "Energy [V]                                      77400\n";
    newLog << "Intensity [uA]                                   7000\n";
    newLog << "!-------------------------------------------------------------------------------\n";
    newLog << "Mu_Scaling                                       8192\n";
    newLog << "Calibration Data              77kVp, BH: 1200 mg HA/ccm, Scaling 8192           \n";
    newLog << "Calib. default unit type      2 (Density)                                       \n";
    newLog << "Density: unit                 mg HA/ccm                                         \n";
    // newLog << "Density: slope                         1.44113599e+03\n";
    // newLog << "Density: intercept                    -3.56445007e+02\n";
    // newLog << "HU: mu water                                  0.24640\n";
    newLog << "Density: slope                         8.19200000e+03\n";
    newLog << "Density: intercept                     0.00000000e+00\n";
    newLog << "HU: mu water                                  0.24640\n";
    //newLog << "!-------------------------------------------------------------------------------\n";
    //newLog << "Parameter (before) name       Linear Attenuation                                \n";
    //newLog << "Parameter units               [1/cm]                                            \n";
    //newLog << "Minimum value                                 0.00000\n";
    //newLog << "Maximum value                            20000.00000\n";
    //newLog << "Average value                                 0.38143\n";
    //newLog << "Standard deviation                            0.21346\n";
    //newLog << "Scaled by factor                                 8192\n";
    //newLog << "Minimum data value                         -852.00000\n";
    //newLog << "Maximum data value                        10717.00000\n";
    //newLog << "Average data value                         3124.65381\n";
    //newLog << "Standard data deviation                    1748.67932\n";
    //newLog << "!-------------------------------------------------------------------------------\n";
    //newLog << "Procedure:                    D3P_Std_FloatNormMax_M()                          \n";
    //newLog << "! \n";
    //newLog << "max                                         127.00000\n";
    //newLog << "! \n";
    //newLog << "Parameter name                Thickness                                         \n";
    //newLog << "Parameter units               [mm]                                              \n";
    //newLog << "!    Zeros not counted. DT type assumed\n";
    //newLog << "Minimum value                                 0.00000\n";
    //newLog << "Maximum value                               127.00000\n";
    //newLog << "Average value                                 0.23753\n";
    //newLog << "Standard deviation                            0.05602\n";
    //newLog << "Scaled by factor                             12.19512\n";
    //newLog << "Minimum data value                            0.00000\n";
    //newLog << "Maximum data value                            4.00000\n";
    //newLog << "Average data value                            2.89675\n";
    //newLog << "Standard data deviation                       0.68320\n";
    // newLog << "!-------------------------------------------------------------------------------\n";

    if (ProcessingLog)
      {
      newLog << ProcessingLog;
      }

    SetProcessingLog(newLog.str().c_str());

  }

  writer.processing_log = ProcessingLog;

  this->UpdateProgress(.1);

  writer.version = AimIO::AIMFILE_VERSION_20;;

  // Write the image
  vtkDataArray* data = input->GetPointData()->GetScalars();
  switch (data->GetDataType())
    {
    case VTK_CHAR:
      writer.WriteImageData ((char*)(data->WriteVoidPointer(0,0)));
      break;
    case VTK_SHORT:
      writer.WriteImageData ((short*)(data->WriteVoidPointer(0,0)));
      break;
    case VTK_FLOAT:
      writer.WriteImageData ((float*)(data->WriteVoidPointer(0,0)));
      break;
    default:
      vtkErrorMacro(<<"Input must be of type VTK_CHAR, VTK_SHORT or VTK_FLOAT.");
      return;
    }

  this->UpdateProgress(1.0);

  return;
}

//----------------------------------------------------------------------------
vtkImageData* vtkboneAIMWriter::GetInput()
{
  return vtkImageData::SafeDownCast(this->Superclass::GetInput());
}

vtkImageData* vtkboneAIMWriter::GetInput(int port)
{
  return vtkImageData::SafeDownCast(this->Superclass::GetInput(port));
}

int vtkboneAIMWriter::FillInputPortInformation(int,
                                           vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}
