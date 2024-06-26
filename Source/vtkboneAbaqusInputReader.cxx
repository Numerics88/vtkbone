#include "vtkboneAbaqusInputReader.h"
#include "AbaqusInputReaderHelper.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkbone_version.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include <fstream>
#include <sys/stat.h>
#include <assert.h>

vtkStandardNewMacro(vtkboneAbaqusInputReader);

//----------------------------------------------------------------------------
vtkboneAbaqusInputReader::vtkboneAbaqusInputReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkboneAbaqusInputReader::~vtkboneAbaqusInputReader()
{
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
void vtkboneAbaqusInputReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if( this->FileName )
    os << indent << "File Name : " << this->FileName << endl;
}

//------------------------------------------------------------------------------
int vtkboneAbaqusInputReader::RequestData(
  vtkInformation *,
  vtkInformationVector **,
  vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkboneFiniteElementModel *output = vtkboneFiniteElementModel::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
    {
    vtkErrorMacro("No output object.");
    return 0;
    }

  // Make sure we have a file to read.
  if (!this->FileName)
    {
    vtkErrorMacro("FileName not set.");
    return 0;
    }

  // Open the input file.
  std::ifstream fin(this->FileName);
  if (!fin)
    {
    vtkErrorMacro("Error opening file " << this->FileName);
    return 0;
    }

  // Get the file size; this is used for progress updates.
  struct stat stat_m;
  int result = stat(this->FileName, &stat_m);
  vtkIdType fileSize = 0;
  if (result == 0)
    {
    fileSize = stat_m.st_size;
    vtkDebugMacro("File size is " << fileSize << " bytes.");
    }
  else
    {
    vtkWarningMacro("Can't determine file size of " << this->FileName);
    }

  vtkDebugMacro("Reading Abaqus file " << this->FileName);

  AbaqusInputReaderHelper reader (fin, fileSize, this, output);
  reader.RegisterMessageObject(this, &DebugMessage, & WarningMessage);
  reader.SetDebug(this->Debug);
  int returnVal = reader.Read();
  if (!reader.GetAbortStatus() && reader.GetErrorStatus() != FSDF_OK)
    {
    vtkErrorMacro("Error in Abaqus Input Deck Reader file:" << this->FileName
                  << "\n" << reader.GetErrorMsg());
    return VTK_ERROR;
    }

  std::ostringstream history;
  history << "Model read from Abaqus input file \"" << this->FileName << "\" using vtkbone version " << VTKBONE_VERSION;
  output->AppendHistory(history.str().c_str());

  return VTK_OK;
}

//------------------------------------------------------------------------------
// Copied and modified from vtkDebugWithObjectMacro in vtkSetGet.h
void vtkboneAbaqusInputReader::DebugMessage (vtkObject* self, const std::string& msg)
{
  if (self->GetDebug() && vtkObject::GetGlobalWarningDisplay())
    {
    vtkOStreamWrapper::EndlType endl;
    vtkOStreamWrapper::UseEndl(endl);
    vtkOStrStreamWrapper vtkmsg;
    vtkOutputWindowDisplayDebugText(msg.c_str());
    vtkmsg.rdbuf()->freeze(0);
    }
}

//------------------------------------------------------------------------------
// Copied and modified from vtkWarningWithObjectMacro in vtkSetGet.h
void vtkboneAbaqusInputReader::WarningMessage (vtkObject* self, const std::string& msg)
{
  if (vtkObject::GetGlobalWarningDisplay())
    {
    vtkOStreamWrapper::EndlType endl;
    vtkOStreamWrapper::UseEndl(endl);
    vtkOStrStreamWrapper vtkmsg;
    if ( self->HasObserver("WarningEvent") )
      {
      // The const_cast is required because the argument to InvokeEvent
      // is not declared const, although presumably (and hopefully) it
      // is not modified!
      self->InvokeEvent("WarningEvent", const_cast<char*>(msg.c_str()));
      }
    else
      {
      vtkOutputWindowDisplayWarningText(msg.c_str());
      }
    vtkmsg.rdbuf()->freeze(0);
    }
}
