/*=========================================================================

                                vtkbone

  VTK classes for building and analyzing Numerics88 finite element models.

  Copyright (c) 2010-2025, Numerics88 Solutions.
  All rights reserved.

=========================================================================*/

#include "CommandStyleFileReader.h"
#include <cstring>
#include <assert.h>

//----------------------------------------------------------------------------
CommandStyleFileReader::CommandStyleFileReader
(
  std::istream&   arg_stream
)
:
  stream          (arg_stream),
  lineCount       (0),
  repeatLastCommand (0),
  errorStatus     (FSDF_OK),
  debug           (0)
{
  // Create a root-level command handlers set
  this->commandContextStack.push(CommandContext_t());
}

//----------------------------------------------------------------------------
int CommandStyleFileReader::Read()
{
  // Repeatedly call FindCommand until we reach the end of file.
  int returnVal;
  while (this->stream.good())
  {
    returnVal = this->FindCommand();
    if (returnVal != FSDF_OK)
    {
      // Won't supersede a previously set error.
      frSetErrorMsgMacro("Unexpected error");
      return returnVal;
    }
  }
  returnVal = this->Finish();
  if (returnVal != FSDF_OK)
  {
    // Won't supersede a previously set error.
    frSetErrorMsgMacro("Unexpected error");
    return returnVal;
  }
  return returnVal;
}

//----------------------------------------------------------------------------
int CommandStyleFileReader::RegisterCommandHandler
(
  const char* commandName,
  CommandHandler_t handler
)
{
  if (this->commandContextStack.top().count(commandName) > 0)
  {
    frSetErrorMsgMacro("Attempt to register duplicate Command Handler: "
                          << commandName);
    return FSDF_ERROR;
  }
  this->commandContextStack.top()[commandName] = handler;
  frSetDebugMsgMacro("Registered Command Handler for " << commandName);
  return FSDF_OK;
}

//----------------------------------------------------------------------------
int CommandStyleFileReader::CallCommandHandler (CommandHandler_t handler)
{
  return (this->*handler)();
}

//----------------------------------------------------------------------------
int CommandStyleFileReader::GetLine()
{
  if (this->repeatLastCommand)
  {
    this->repeatLastCommand = 0;
    return 1;
  }
  // if (stream.getline(line,maxLineLength).good())
  if (getline(this->stream,this->line).good())
  {
    this->lineCount++;
    int len = this->line.size();
    // Remove trailing \r character if present.
    if (len > 0 && this->line[len-1] == '\r')
    {
      this->line.resize(len-1);
    }
    return 1;
  }
  else
  {
    if (this->stream.bad())
    {
      frSetErrorMsgMacro("File IO error: line " << this->lineCount+1);
      return 0;
    }
    else if (this->stream.eof())
    {
      // Normal end of file - nothing wrong here
      return 0;
    }
    else if (this->stream.fail())
    {
      frSetErrorMsgMacro("File error (excessively long line?): line "
                         << this->lineCount+1);
      return 0;
    }
    assert (0);      // Should be impossible to get here.
  }
  return 0;
}

//----------------------------------------------------------------------------
int CommandStyleFileReader::FindCommand()
{
  while (this->GetLine())
  {

    if (this->IsCommand())
    {
      frSetDebugMsgMacro("Found command: " << this->commandName
          << " at line " << this->lineCount);
      int returnVal = this->ProcessCommand();
      // Bail on error
      if (returnVal != FSDF_OK) {return returnVal;}
    }

  }  // while (this->GetLine())

  if (this->stream.eof())
  {
    // Normal end of file reached - all done.
    return FSDF_OK;
  }
  return FSDF_ERROR;  // Otherwise this is not a normal exit - report failure.
}

//----------------------------------------------------------------------------
int CommandStyleFileReader::ProcessCommand()
{
  if (this->commandContextStack.top().count(this->commandName))
  {
    CommandHandler_t handler = this->commandContextStack.top()[this->commandName];
    int returnVal = this->CallCommandHandler (handler);
    return returnVal;
  }
  else
  {
    // Unhandled command - just warn and continue.
    frSetWarningMsgMacro("Unhandled command in current context: " << this->commandName
          << " at line " << this->lineCount);
  }
  return FSDF_OK;
}

//----------------------------------------------------------------------------
void CommandStyleFileReader::SetError (const std::string& msg)
{
  // Don't overwrite previously set error.
  if (this->errorStatus == FSDF_OK)
  {
    this->errorStatus = FSDF_ERROR;
    this->errorMsg = msg;
  }
}

//----------------------------------------------------------------------------
void CommandStyleFileReader::DebugMessage (const std::string& msg)
{
  if (this->debug)
  {
    std::cerr << "DEBUG: " << msg << "\n";
  }
}

//----------------------------------------------------------------------------
void CommandStyleFileReader::WarningMessage (const std::string& msg)
{
  std::cerr << "WARNING: " << msg << "\n";
}
