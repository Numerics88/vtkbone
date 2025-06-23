/*=========================================================================

                                vtkbone

  VTK classes for building and analyzing Numerics88 finite element models.

  Copyright (c) 2010-2025, Numerics88 Solutions.
  All rights reserved.

=========================================================================*/

#include "AbaqusInputReaderHelper.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkboneLinearOrthotropicMaterial.h"
#include "vtkboneMaterialTable.h"
#include "vtkboneConstraint.h"
#include "vtkboneConstraintCollection.h"
#include "vtkAlgorithm.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkIntArray.h"
#include "vtkCharArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkSmartPointer.h"
#include "n88util/text.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <assert.h>

using boost::lexical_cast;

//----------------------------------------------------------------------------
AbaqusInputReaderHelper::AbaqusInputReaderHelper
(
  std::istream&            arg_stream,
  long                     arg_streamSize,
  vtkAlgorithm*            arg_boss,
  vtkboneFiniteElementModel*            arg_model
)
:
  CommandStyleFileReader (arg_stream),
  debugHandler      (NULL),
  warningHandler    (NULL),
  messageObject     (NULL),
  streamSize        (arg_streamSize),
  boss              (arg_boss),
  model             (arg_model),
  abortExecute      (0)
{
  assert(model);  // Don't allow NULL pointer.
  this->model->Initialize();  // Clear any pre-existing data.

  // Register root level command handlers
  RegisterCommandHandler ("HEADING",
      reinterpret_cast<CommandHandler_t>(
      &AbaqusInputReaderHelper::Read_HEADING));
  RegisterCommandHandler ("NODE",
      reinterpret_cast<CommandHandler_t>(
      &AbaqusInputReaderHelper::Read_NODE));
  RegisterCommandHandler ("ELEMENT",
       reinterpret_cast<CommandHandler_t>(
       &AbaqusInputReaderHelper::Read_ELEMENT));
  RegisterCommandHandler ("NSET",
     reinterpret_cast<CommandHandler_t>(
     &AbaqusInputReaderHelper::Read_NSET));
  RegisterCommandHandler ("ELSET",
     reinterpret_cast<CommandHandler_t>(
     &AbaqusInputReaderHelper::Read_ELSET));
  RegisterCommandHandler ("MATERIAL",
       reinterpret_cast<CommandHandler_t>(
       &AbaqusInputReaderHelper::Read_MATERIAL));
  RegisterCommandHandler ("ELASTIC",
       reinterpret_cast<CommandHandler_t>(
       &AbaqusInputReaderHelper::Read_ELASTIC));
  RegisterCommandHandler ("SOLID SECTION",
       reinterpret_cast<CommandHandler_t>(
       &AbaqusInputReaderHelper::Read_SOLID_SECTION));
  RegisterCommandHandler ("STEP",
       reinterpret_cast<CommandHandler_t>(
       &AbaqusInputReaderHelper::Read_STEP));
}

//------------------------------------------------------------------------------
void AbaqusInputReaderHelper::RegisterMessageObject
(
  vtkObject* obj,
  messageHandler_t debugCall,
  messageHandler_t warningCall
)
{
  this->messageObject = obj;
  this->debugHandler = debugCall;
  this->warningHandler = warningCall;
}

//------------------------------------------------------------------------------
void AbaqusInputReaderHelper::DebugMessage (const std::string& msg)
{
  if (this->debugHandler && this->messageObject)
  {
    (*this->debugHandler) (this->messageObject, msg);
  }
  else
  {
    CommandStyleFileReader::DebugMessage (msg);
  }
}

//------------------------------------------------------------------------------
void AbaqusInputReaderHelper::WarningMessage (const std::string& msg)
{
  if (this->warningHandler && this->messageObject)
  {
    (*warningHandler) (messageObject, msg);
  }
  else
  {
    CommandStyleFileReader::WarningMessage (msg);
  }
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::GetLine()
{
  // Call GetLine of the base class;
  int returnVal = CommandStyleFileReader::GetLine();
  // Update progress and check for abort
  if (returnVal)
  {
    if (this->lineCount % progessInterval == 0)
    {
      if (this->streamSize && this->boss)
      {
        this->boss->UpdateProgress (static_cast<double>(this->stream.tellg())/this->streamSize);
        if (this->boss->GetAbortExecute())
        {
          // Set an error message so no other error gets set.
          frSetErrorMsgMacro( "Abort Execute called");
          this->abortExecute = 1;
          return 0;
        }
      }
    }
  }
  return returnVal;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::IsCommand ()
{
  if (this->line.size() < 2) { return 0; }
  if (this->line[0] != '*') { return 0; }
  if (this->line[1] == '*') { return 0; }
  // Now parse for command name and optionally parameters
  std::vector<std::string> tokens;
  n88util::split_trim (this->line.substr(1), tokens);
  if (tokens.size() == 0 || tokens[0].size() == 0)
    { return 0; }
  this->commandName = tokens[0];
  boost::to_upper (this->commandName);
  this->parameters.clear();
  for (size_t i=1; i<tokens.size(); ++i)
  {
    size_t found = tokens[i].find_first_of('=');
    std::string key;
    std::string value;
    if (found == std::string::npos)
    {
      key = tokens[i];
    }
    else
    {
      key = tokens[i].substr (0,found);
      value = tokens[i].substr (found+1);
      boost::algorithm::trim (value);
    }
    boost::algorithm::trim (key);
    boost::to_upper (key);
    this->parameters[key] = value;
  }
  return 1;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::IsCommentLine()
{
  if (this->line.size() < 2) { return 0; }
  if (this->line[0] != '*') { return 0; }
  if (this->line[1] != '*') { return 0; }
  return 1;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_HEADING()
{
  std::ostringstream comments;

  while (this->GetLine())
  {

    // On next command finish reading heading
    if (this->IsCommand())
    {
      this->repeatLastCommand = 1;
      break;
    }

    if (this->IsCommentLine())
    {
      if (line.size() == 2)
      {
        comments << "\n";
        continue;
      }
      if (line[2] == ' ')
      {
        comments << this->line.substr(3) << "\n";
        continue;
      }
      comments << this->line.substr(2) << "\n";
      continue;
    }

    comments << this->line << "\n";

  }

  this->model->SetLog(comments.str().c_str());
  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_NODE()
{
  if (this->model->GetPoints())
  {
    frSetWarningMsgMacro( "Unexpected duplicate NODE section line "
                        << this->lineCount << ". Ignoring.");
    return FSDF_OK;
  }

  vtkIdType n = 0;
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  frSetDebugMsgMacro( "Reading Node Data.");

  while (this->GetLine())
  {

    if (this->line.size() == 0 || this->IsCommentLine())
      { continue; }

    // On next command finish reading nodes
    if (this->IsCommand())
    {
      this->repeatLastCommand = 1;
      break;
    }

    // Try to read line as node data
    vtkIdType n_previous = n;
    long nTmp;  // required because vtkIdType may or may not be type long
    double x[3];
    if (sscanf(this->line.c_str(), "%ld, %lf, %lf, %lf", &nTmp, &x[0], &x[1], &x[2]) != 4)
    {
      frSetErrorMsgMacro( "Parse error: line " << this->lineCount);
      return FSDF_ERROR;
    }
    n = nTmp;
    if (n != n_previous+1)
    {
      frSetErrorMsgMacro( "Non-consecutive node numbering: line "
                                     << this->lineCount);
      return FSDF_ERROR;
    }
    points->InsertNextPoint(x);
  }  // while (GetLine())

  // Check if error occurred
  if (this->GetErrorStatus())
    { return FSDF_ERROR; }

  frSetDebugMsgMacro( "End of command NODE at line " << this->lineCount
      << ". Read " << n << " node points");
  this->model->SetPoints(points);
  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_ELEMENT()
{
  // Note: In principle, there can be multiple sets of elements, which
  //       belong to different parts.  We do not support this (yet).
  if (this->model->GetCells())
  {
    frSetWarningMsgMacro(
        "Unexpected duplicate ELEMENT section line "
          << this->lineCount << ". Ignoring.");
    return FSDF_OK;
  }

  frSetDebugMsgMacro( "Reading ELEMENT Data.");

  if (this->parameters.count("TYPE") == 0)
  {
    frSetWarningMsgMacro( "Unable to identify TYPE in ELEMENT command, line "
      << this->lineCount << ". Ignoring section.");
    return FSDF_OK;
  }
  std::string type = this->parameters["TYPE"];
  frSetDebugMsgMacro("Identified TYPE in ELEMENT command as " << type);
  if (type != "C3D8")
  {
    frSetWarningMsgMacro( "Unable to handle ELEMENT TYPE " << type
      << ". Ignoring section.");
    return FSDF_OK;
  }

  // Note that it doesn't matter whether ELSET is declared here or elsewhere
  if (this->parameters.count("ELSET"))
  {
    this->allElementsSet = this->parameters["ELSET"];
    frSetDebugMsgMacro("Identified ELSET in ELEMENT command as " << this->allElementsSet);
  }

  vtkIdType n = 0;
  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  vtkIdType numNodes = this->model->GetPoints()->GetNumberOfPoints();

  while (this->GetLine())
  {

    if (this->line.size() == 0 || this->IsCommentLine())
      { continue; }

    // On next command finish reading elements
    if (this->IsCommand())
    {
      this->repeatLastCommand = 1;
      break;
    }

    // Try to read line as element data
    vtkIdType x[8];
    vtkIdType n_previous = n;
    long nTmp;  // required because vtkIdType may or may not be type long
    long tmp[8];  // required because vtkIdType may or may not be long
                       // depending on the platform.
    if (sscanf(this->line.c_str(), "%ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld", &nTmp,
            &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6], &tmp[7] ) != 9)
    {
      frSetErrorMsgMacro( "Parse error: line " << this->lineCount);
      return FSDF_ERROR;
    }
    n = nTmp;
    if (n != n_previous+1)
    {
      frSetErrorMsgMacro( "Non-consecutive element numbering: line "
                                     << this->lineCount);
      return FSDF_ERROR;
    }
    // NOTE that topology of type C3D8 is the same as VTK_HEXAHEDRON
    for (int i=0; i<8; i++)
    {
      x[i] = tmp[i] - 1;
      if ((x[i] < 0) || (x[i] >= numNodes))
      {
        frSetErrorMsgMacro( "Invalid node number: line " << this->lineCount);
        return FSDF_ERROR;
      }
    }
    cells->InsertNextCell(8, x);
  }  // while (GetLine())

  // Check if error occurred
  if (this->GetErrorStatus())
    { return FSDF_ERROR; }

  frSetDebugMsgMacro( "End of command ELEMENT at line " << this->lineCount
      << ". Read " << n << " elements");
  this->model->SetCells (VTK_HEXAHEDRON, cells);

  // Need to attach scalars - for now set all to zero.
  vtkSmartPointer<vtkIntArray> scalars = vtkSmartPointer<vtkIntArray>::New();
  scalars->SetNumberOfTuples (n);
  for (vtkIdType i=0; i<n; ++i)
    { scalars->SetValue(i,0); }
  this->model->GetCellData()->SetScalars (scalars);

  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_NSET()
{
  frSetDebugMsgMacro( "Reading NSET.");

  std::string name;
  if (this->parameters.count("NSET") == 0)
  {
    frSetWarningMsgMacro( "Unable to identify NSET in NSET command, line "
        << this->lineCount << ". Ignoring section.");
    return FSDF_OK;
  }
  name = this->parameters["NSET"];
  frSetDebugMsgMacro("Identified NSET in NSET command as " << name);

  vtkSmartPointer<vtkIdTypeArray> nodeList =
          vtkSmartPointer<vtkIdTypeArray>::New();
  nodeList->SetName (name.c_str());
  vtkIdType numNodes = this->model->GetPoints()->GetNumberOfPoints();

  while (this->GetLine())
  {

    if (this->line.size() == 0 || this->IsCommentLine())
      { continue; }

    // On next command finish reading nodes
    if (this->IsCommand())
    {
      this->repeatLastCommand = 1;
      break;
    }

    std::vector<std::string> tokens;
    n88util::split_arguments(this->line, tokens);
    if (this->parameters.count("GENERATE"))
    {
      // Parse for a triple of numbers
      if (tokens.size() != 3)
      {
        frSetWarningMsgMacro( "NSET with GENERATE requires exactly 3 values: line "<< this->lineCount);
        break;
      }
      try
      {
        // Convert to 0-indexed.
        vtkIdType startId = lexical_cast<vtkIdType>(tokens[0]) - 1;
        vtkIdType stopId = lexical_cast<vtkIdType>(tokens[1]) - 1;
        vtkIdType step = lexical_cast<vtkIdType>(tokens[2]);
        for (vtkIdType nodeId=startId; nodeId<=stopId; nodeId += step)
        {
          if ((nodeId < 0) || (nodeId >= numNodes))
          {
            frSetErrorMsgMacro( "Invalid node number: line "<< this->lineCount);
            return FSDF_ERROR;
          }
          nodeList->InsertNextValue (nodeId);
        }
      }
      catch (boost::bad_lexical_cast&)
      {
        frSetErrorMsgMacro("Parse error, unable to interpret as number: line " << this->lineCount);
        return FSDF_ERROR;
      }
    }
    else
    {
      // Parse for a sequence of numbers separated by commas
      for (size_t i=0; i<tokens.size(); ++i)
      {
        try
        {
          vtkIdType nodeId = lexical_cast<vtkIdType>(tokens[i]) - 1;  // Convert to 0-indexed.
          if ((nodeId < 0) || (nodeId >= numNodes))
          {
            frSetErrorMsgMacro( "Invalid node number: line "<< this->lineCount);
            return FSDF_ERROR;
          }
          nodeList->InsertNextValue (nodeId);
        }
        catch (boost::bad_lexical_cast&)
        {
          frSetErrorMsgMacro("Parse error, unable to interpret as number: line " << this->lineCount);
          return FSDF_ERROR;
        }
      }
    }

  }  // while (this->GetLine())

  // Check if error occurred
  if (this->GetErrorStatus())
    { return FSDF_ERROR; }

  frSetDebugMsgMacro( "Read " << nodeList->GetNumberOfTuples() << " node Ids");
  this->model->AddNodeSet (nodeList);
  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_ELSET()
{
  frSetDebugMsgMacro( "Reading ELSET.");

  std::string name;
  if (this->parameters.count("ELSET") == 0)
  {
    frSetWarningMsgMacro( "Unable to identify ELSET in ELSET command, line "
        << this->lineCount << ". Ignoring section.");
    return FSDF_OK;
  }
  name = this->parameters["ELSET"];
  frSetDebugMsgMacro("Identified ELSET in ELSET command as " << name);

  vtkSmartPointer<vtkIdTypeArray> elementList =
          vtkSmartPointer<vtkIdTypeArray>::New();
  elementList->SetName (name.c_str());
  vtkIdType numElements = this->model->GetCells()->GetNumberOfCells();

  while (this->GetLine())
  {

    if (this->line.size() == 0 || this->IsCommentLine())
      { continue; }

    // On next command finish reading elements
    if (this->IsCommand())
    {
      this->repeatLastCommand = 1;
      break;
    }

    std::vector<std::string> tokens;
    n88util::split_arguments(this->line, tokens);
    if (this->parameters.count("GENERATE"))
    {
      // Parse for a triple of numbers
      if (tokens.size() != 3)
      {
        frSetWarningMsgMacro( "ELSET with GENERATE requires exactly 3 values: line "<< this->lineCount);
        break;
      }
      try
      {
        // Convert to 0-indexed.
        vtkIdType startId = lexical_cast<vtkIdType>(tokens[0]) - 1;
        vtkIdType stopId = lexical_cast<vtkIdType>(tokens[1]) - 1;
        vtkIdType step = lexical_cast<vtkIdType>(tokens[2]);
        for (vtkIdType elementId=startId; elementId<=stopId; elementId += step)
        {
          if ((elementId < 0) || (elementId >= numElements))
          {
            frSetErrorMsgMacro( "Invalid element number: line "<< this->lineCount);
            return FSDF_ERROR;
          }
          elementList->InsertNextValue (elementId);
        }
      }
      catch (boost::bad_lexical_cast&)
      {
        frSetErrorMsgMacro("Parse error, unable to interpret as number: line " << this->lineCount);
        return FSDF_ERROR;
      }
    }
    else
    {
      // Parse for a sequence of numbers separated by commas
      std::vector<std::string> tokens;
      n88util::split_arguments(this->line, tokens);
      for (size_t i=0; i<tokens.size(); ++i)
      {
        try
        {
          vtkIdType elementId = lexical_cast<vtkIdType>(tokens[i]) - 1;  // Convert to 0-indexed.
          if ((elementId < 0) || (elementId >= numElements))
          {
            frSetErrorMsgMacro( "Invalid element number: line "<< this->lineCount);
            return FSDF_ERROR;
          }
          elementList->InsertNextValue (elementId);
        }
        catch (boost::bad_lexical_cast&)
        {
          frSetErrorMsgMacro("Parse error, unable to interpret as number: line " << this->lineCount);
          return FSDF_ERROR;
        }
      }
    }

  }  // while (this->GetLine())

  // Check if error occurred
  if (this->GetErrorStatus())
    { return FSDF_ERROR; }

  frSetDebugMsgMacro( "Read " << elementList->GetNumberOfTuples() << " element Ids");
  this->model->AddElementSet (elementList);
  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_MATERIAL()
{
  if (!this->currentMaterial.empty())
  {
    frSetWarningMsgMacro( "Previous MATERIAL definition not complete; line " << this->lineCount);
    return FSDF_OK;
  }
  if (this->parameters.count("NAME") == 0)
  {
    frSetWarningMsgMacro( "Unable to identify NAME in MATERIAL command, line "
        << this->lineCount << ". Ignoring material.");
    return FSDF_OK;
  }
  this->currentMaterial = this->parameters["NAME"];
  frSetDebugMsgMacro("Identified NAME in MATERIAL command as " << this->currentMaterial);
  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_ELASTIC()
{
  if (this->currentMaterial.empty())
  {
    frSetWarningMsgMacro( "ELASTIC command with no current material; line " << this->line);
    return FSDF_OK;
  }

  std::string type;
  if (this->parameters.count("TYPE"))
  {
    type = this->parameters["TYPE"];
    frSetDebugMsgMacro("Identified TYPE in ELASTIC command as " << type);
  }
  else
  {
    type = "ISOTROPIC";
    frSetDebugMsgMacro("Selecting default TYPE in ELASTIC command as " << type);
  }
  if (type != "ISOTROPIC" && type != "ORTHOTROPIC")
  {
    frSetWarningMsgMacro( "Unable to handle ELASTIC TYPE " << type
      << ". Ignoring section.");
    return FSDF_OK;
  }

  while (this->GetLine())
  {

    if (this->line.size() == 0 || this->IsCommentLine())
      { continue; }

    if (this->IsCommand())
    {
      frSetWarningMsgMacro( "Unexpected new command with ELASTIC definition; line " << this->lineCount);
      this->currentMaterial.clear();
      this->repeatLastCommand = 1;
      return FSDF_OK;
    }

    if (type == "ISOTROPIC")
    {
      // Try parsing as a pair of numbers
      std::vector<std::string> tokens;
      n88util::split_trim(this->line, tokens);
      if (tokens.size() != 2)
      {
        frSetWarningMsgMacro( "Unexpected 2 values for ISOTROPIC definition; line " << this->lineCount);
        this->currentMaterial.clear();
        return FSDF_OK;
      }
      try
      {
        vtkSmartPointer<vtkboneLinearIsotropicMaterial> material =
                         vtkSmartPointer<vtkboneLinearIsotropicMaterial>::New();
        material->SetName(this->currentMaterial.c_str());
        material->SetYoungsModulus(lexical_cast<double>(tokens[0]));
        material->SetPoissonsRatio(lexical_cast<double>(tokens[1]));
        if (this->model->GetMaterialTable()->GetMaterial(this->currentMaterial.c_str()))
        {
          frSetWarningMsgMacro("Duplicate material. Discarding new definition; line " << this->lineCount);
          this->currentMaterial.clear();
          return FSDF_OK;
        }
        this->model->GetMaterialTable()->AppendMaterial(material);
        this->currentMaterial.clear();
        return FSDF_OK;
      }
      catch (boost::bad_lexical_cast&)
      {
        frSetWarningMsgMacro("Parse error, unable to interpret as number: line " << this->lineCount);
        this->currentMaterial.clear();
        return FSDF_OK;
      }
    }
    else if (type == "ORTHOTROPIC")
    {
      // Try parsing as 8 numbers
      std::vector<std::string> tokens;
      n88util::split_trim(this->line, tokens);
      if (tokens.size() != 8)
      {
        frSetWarningMsgMacro( "Unexpected 8 values for ORTHOTROPIC definition; line " << this->lineCount);
        this->currentMaterial.clear();
        return FSDF_OK;
      }
      // Orthotropic continues to the next line.
      if (!this->GetLine())
      {
        frSetErrorMsgMacro( "Unexpected end of file; line " << this->lineCount);
        return FSDF_ERROR;
      }
      if (this->IsCommand())
      {
        frSetWarningMsgMacro( "Unexpected new command with ORTHOTROPIC definition; line " << this->lineCount);
        this->currentMaterial.clear();
        this->repeatLastCommand = 1;
        return FSDF_OK;
      }
      try
      {
        double D1111 = lexical_cast<double>(tokens[0]);
        double D1122 = lexical_cast<double>(tokens[1]);
        double D2222 = lexical_cast<double>(tokens[2]);
        double D1133 = lexical_cast<double>(tokens[3]);
        double D2233 = lexical_cast<double>(tokens[4]);
        double D3333 = lexical_cast<double>(tokens[5]);
        double D1212 = lexical_cast<double>(tokens[6]);
        double D1313 = lexical_cast<double>(tokens[7]);
        double D2323 = lexical_cast<double>(this->line);
        vtkSmartPointer<vtkboneLinearOrthotropicMaterial> material =
                         vtkSmartPointer<vtkboneLinearOrthotropicMaterial>::New();
        material->SetName(this->currentMaterial.c_str());
        material->SetYoungsModulusX(1.0/D1111);
        material->SetYoungsModulusY(1.0/D2222);
        material->SetYoungsModulusZ(1.0/D3333);
        material->SetPoissonsRatioYZ(-D2233/D2222);
        material->SetPoissonsRatioZX(-D1133/D3333);
        material->SetPoissonsRatioXY(-D1122/D1111);
        material->SetShearModulusYZ(1.0/D2323);
        material->SetShearModulusZX(1.0/D1313);
        material->SetShearModulusXY(1.0/D1212);
        if (this->model->GetMaterialTable()->GetMaterial(this->currentMaterial.c_str()))
        {
          frSetWarningMsgMacro("Duplicate material. Discarding new definition; line " << this->lineCount);
          this->currentMaterial.clear();
          return FSDF_OK;
        }
        this->model->GetMaterialTable()->AppendMaterial(material);
        this->currentMaterial.clear();
        return FSDF_OK;
      }
      catch (boost::bad_lexical_cast&)
      {
        frSetWarningMsgMacro("Parse error, unable to interpret as number: line " << this->lineCount);
        this->currentMaterial.clear();
        return FSDF_OK;
      }
    }

  }  // while (GetLine())

  this->currentMaterial.clear();

  // Check if error occurred
  if (this->GetErrorStatus())
  {
    return FSDF_ERROR;
  }

  frSetWarningMsgMacro( "Unable to identify values for ELASTIC; line " << this->lineCount);
  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_SOLID_SECTION()
{
  if (this->parameters.count("ELSET") == 0)
  {
    frSetWarningMsgMacro( "Unable to identify ELSET in SOLID SECTION command, line "
        << this->lineCount);
    return FSDF_OK;
  }
  std::string elset = this->parameters["ELSET"];
  frSetDebugMsgMacro("Identified ELSET in SOLID SECTION command as " << elset);
  if (this->parameters.count("MATERIAL") == 0)
  {
    frSetWarningMsgMacro( "Unable to identify MATERIAL in SOLID SECTION command, line "
        << this->lineCount);
    return FSDF_OK;
  }
  std::string materialName = this->parameters["MATERIAL"];
  frSetDebugMsgMacro("Identified MATERIAL in SOLID SECTION command as " << materialName);

  int index = this->model->GetMaterialTable()->GetIndex(materialName.c_str());
  if (index == 0)
  {
    frSetWarningMsgMacro( "Undefined material: " << materialName << " line "
        << this->lineCount);
    return FSDF_OK;
  }

  vtkIntArray* scalars = vtkIntArray::SafeDownCast(this->model->GetCellData()->GetScalars());
  if (scalars == NULL)
  {
    frSetErrorMsgMacro ("Internal error. No Cell scalars.");
    return FSDF_ERROR;
  }

  if (elset == this->allElementsSet)
  {
    // Set all elements to specified material
    for (vtkIdType i=0; i<scalars->GetNumberOfTuples(); ++i)
      { scalars->SetValue(i,index); }
  }
  else
  {
    vtkIdTypeArray* ids = this->model->GetElementSet(elset.c_str());
    if (ids == NULL)
    {
      frSetWarningMsgMacro( "No such element set: " << elset);
      return FSDF_OK;
    }
    for (vtkIdType i=0; i<ids->GetNumberOfTuples(); ++i)
    {
      vtkIdType id = ids->GetValue(i);
      if (id >= this->model->GetNumberOfCells())
      {
        frSetWarningMsgMacro( "Invalid ID in element set");
        return FSDF_OK;
      }
      scalars->SetValue(id,index);
    }
  }

  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_STEP()
{
  frSetDebugMsgMacro( "Reading STEP.");

  // STEP continues until END STEP, so create a new level of command handlers
  this->commandContextStack.push(CommandContext_t());
  RegisterCommandHandler ("STATIC",
       reinterpret_cast<CommandHandler_t>(
       &AbaqusInputReaderHelper::Read_STATIC));
  RegisterCommandHandler ("BOUNDARY",
       reinterpret_cast<CommandHandler_t>(
       &AbaqusInputReaderHelper::Read_BOUNDARY));
  RegisterCommandHandler ("CLOAD",
       reinterpret_cast<CommandHandler_t>(
       &AbaqusInputReaderHelper::Read_CLOAD));
  RegisterCommandHandler ("END STEP",
       reinterpret_cast<CommandHandler_t>(
       &AbaqusInputReaderHelper::Read_END_STEP));

  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_STATIC()
{
  // Not really anything to do; this is the only kind of model that we solve.
  frSetDebugMsgMacro( "Reading STATIC.");
  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_BOUNDARY()
{
  if (this->parameters.count("TYPE") == 0)
  {
    frSetWarningMsgMacro( "Unable to identify TYPE in BOUNDARY command, line "
        << this->lineCount << ". Ignoring section.");
    return FSDF_OK;
  }
  std::string type = this->parameters["TYPE"];
  frSetDebugMsgMacro("Identified TYPE in BOUNDARY command as " << type);
  if (type != "DISPLACEMENT")
  {
    frSetWarningMsgMacro( "Unhandled value for TYPE in BOUNDARY command: "
        << type << " ; line " << this->lineCount << ". Ignoring section.");
    return FSDF_OK;
  }

  std::string name;
  if (this->parameters.count("NAME"))
  {
    name = this->parameters["NAME"];
    frSetDebugMsgMacro("Identified NAME in BOUNDARY command as " << name);
  }
  else
  {
    // No name specified, need to come up with our own.
    int i = 1;
    do
    {
      name = (boost::format("boundary_%d") % i).str();
      ++i;
    } while (this->model->GetConstraints()->GetItem(name.c_str()) != NULL);
    frSetDebugMsgMacro("Assigning name of " << name);
  }

  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetName("SENSE");
  vtkSmartPointer<vtkFloatArray> values = vtkSmartPointer<vtkFloatArray>::New();
  values->SetName("VALUE");

  while (this->GetLine())
  {

    if (this->line.size() == 0 || this->IsCommentLine())
      { continue; }

    // On next command finish reading boundary
    if (this->IsCommand())
    {
      this->repeatLastCommand = 1;
      break;
    }

    // Parse for a sequence of numbers separated by commas
    std::vector<std::string> tokens;
    n88util::split_trim(this->line, tokens);
    if (tokens.size() != 4)
    {
      frSetWarningMsgMacro( "Expected 4 values for BOUNDARY data line; line "
        << this->lineCount);
      continue;
    }
    int firstDegreeOfFreedom;
    float magnitude;
    try
    {
      firstDegreeOfFreedom = lexical_cast<vtkIdType>(tokens[1]) - 1;
      magnitude = lexical_cast<float>(tokens[3]) ;
    }
    catch (boost::bad_lexical_cast&)
    {
      frSetWarningMsgMacro("Parse error, unable to interpret as number; line " << this->lineCount);
      continue;
    }
    int lastDegreeOfFreedom;
    try
    {
      lastDegreeOfFreedom = lexical_cast<vtkIdType>(tokens[2]) - 1;
    }
    catch (boost::bad_lexical_cast&)
    {
      // If not present, is the same as the first.
      lastDegreeOfFreedom = firstDegreeOfFreedom;
    }
    if (firstDegreeOfFreedom < 0 ||
        lastDegreeOfFreedom < 0 ||
        firstDegreeOfFreedom > 2 ||
        lastDegreeOfFreedom > 2)
    {
      frSetWarningMsgMacro("Degree of freedom out of range; line " << this->lineCount);
      continue;
    }
    try
    {
      vtkIdType id = lexical_cast<vtkIdType>(tokens[0]) - 1;
      for (int s=firstDegreeOfFreedom; s<=lastDegreeOfFreedom; ++s)
      {
        ids->InsertNextValue(id);
        senses->InsertNextValue(s);
        values->InsertNextValue(magnitude);
      }
    }
    catch (boost::bad_lexical_cast&)
    {
      // If not a number, try as a node set name.
      vtkIdTypeArray* nodeSet = this->model->GetNodeSet(tokens[0].c_str());
      if (nodeSet == NULL)
      {
        frSetWarningMsgMacro("Unable to interpret " << tokens[0]
            << " as number or node set name; line " << this->lineCount);
        continue;
      }
      for (vtkIdType i=0; i<nodeSet->GetNumberOfTuples(); ++i)
      {
        vtkIdType id = nodeSet->GetValue(i);
        for (int s=firstDegreeOfFreedom; s<=lastDegreeOfFreedom; ++s)
        {
          ids->InsertNextValue(id);
          senses->InsertNextValue(s);
          values->InsertNextValue(magnitude);
        }
      }
    }

  }  // while (GetLine())

  // Check if error occurred
  if (this->GetErrorStatus())
    { return FSDF_ERROR; }

  vtkSmartPointer<vtkboneConstraint> constraint = vtkSmartPointer<vtkboneConstraint>::New();
  constraint->SetName(name.c_str());
  constraint->SetIndices(ids);
  constraint->SetConstraintType(vtkboneConstraint::DISPLACEMENT);
  constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
  constraint->GetAttributes()->AddArray(senses);
  constraint->GetAttributes()->AddArray(values);
  model->GetConstraints()->AddItem(constraint);

  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_CLOAD()
{
  std::string name;
  if (this->parameters.count("NAME"))
  {
    name = this->parameters["NAME"];
    frSetDebugMsgMacro("Identified NAME in CLOAD command as " << name);
  }
  else
  {
    // No name specified, need to come up with our own.
    int i = 1;
    do
    {
      name = (boost::format("load_%d") % i).str();
      ++i;
    } while (this->model->GetConstraints()->GetItem(name.c_str()) != NULL);
    frSetDebugMsgMacro("Assigning name of " << name);
  }

  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetName("SENSE");
  vtkSmartPointer<vtkFloatArray> values = vtkSmartPointer<vtkFloatArray>::New();
  values->SetName("VALUE");

  while (this->GetLine())
  {

    if (this->line.size() == 0 || this->IsCommentLine())
      { continue; }

    // On next command finish reading boundary
    if (this->IsCommand())
    {
      this->repeatLastCommand = 1;
      break;
    }

    // Parse for a sequence of numbers separated by commas
    std::vector<std::string> tokens;
    n88util::split_trim(this->line, tokens);
    if (tokens.size() != 3)
    {
      frSetWarningMsgMacro( "Expected 3 values for CLOAD data line; line "
        << this->lineCount);
      continue;
    }
    int degreeOfFreedom;
    float magnitude;
    try
    {
      degreeOfFreedom = lexical_cast<vtkIdType>(tokens[1]) - 1;
      magnitude = lexical_cast<float>(tokens[2]);
    }
    catch (boost::bad_lexical_cast&)
    {
      frSetWarningMsgMacro("Parse error, unable to interpret as number; line " << this->lineCount);
      continue;
    }
    if (degreeOfFreedom < 0 ||
        degreeOfFreedom > 2)
    {
      frSetWarningMsgMacro("Degree of freedom out of range; line " << this->lineCount);
      continue;
    }
    try
    {
      vtkIdType id = lexical_cast<vtkIdType>(tokens[0]) - 1;
      ids->InsertNextValue(id);
      senses->InsertNextValue(degreeOfFreedom);
      values->InsertNextValue(magnitude);
    }
    catch (boost::bad_lexical_cast&)
    {
      // If not a number, try as a node set name.
      vtkIdTypeArray* nodeSet = this->model->GetNodeSet(tokens[0].c_str());
      if (nodeSet == NULL)
      {
        frSetWarningMsgMacro("Unable to interpret " << tokens[0]
            << " as number or node set name; line " << this->lineCount);
        continue;
      }
      for (vtkIdType i=0; i<nodeSet->GetNumberOfTuples(); ++i)
      {
        vtkIdType id = nodeSet->GetValue(i);
        ids->InsertNextValue(id);
        senses->InsertNextValue(degreeOfFreedom);
        values->InsertNextValue(magnitude);
      }
    }

  }  // while (GetLine())

  // Check if error occurred
  if (this->GetErrorStatus())
    { return FSDF_ERROR; }

  vtkSmartPointer<vtkboneConstraint> constraint = vtkSmartPointer<vtkboneConstraint>::New();
  constraint->SetName(name.c_str());
  constraint->SetIndices(ids);
  constraint->SetConstraintType(vtkboneConstraint::FORCE);
  constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
  constraint->GetAttributes()->AddArray(senses);
  constraint->GetAttributes()->AddArray(values);
  model->GetConstraints()->AddItem(constraint);

  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Read_END_STEP()
{
  frSetDebugMsgMacro( "Exiting STEP.");
  // Return to root level command handlers
  this->commandContextStack.pop();

  return FSDF_OK;
}

//------------------------------------------------------------------------------
int AbaqusInputReaderHelper::Finish()
{
  return FSDF_OK;
}
