/*=========================================================================

                                vtkbone

  VTK classes for building and analyzing Numerics88 finite element models.

  Copyright (c) 2010-2025, Numerics88 Solutions.
  All rights reserved.

=========================================================================*/

#ifndef __AbaqusInputReaderHelper_h
#define __AbaqusInputReaderHelper_h

#include "CommandStyleFileReader.h"

// Forward declarations
class vtkAlgorithm;
class vtkboneFiniteElementModel;
class vtkObject;

/** @class AbaqusInputReaderHelper

  A class to read Abaqus Data files.

  The data will be written in a VTK object of type vtkboneFiniteElementModel.

  This object is not itself a VTK object and is intended to be encapsulated
  inside a VTK obect (vtkboneAbaqusInputReader).  The purpose of this keep to
  public interface of vtkboneAbaqusInputReader as simple as possible.  As
  well, this hides from the VTK interface C++ STL objects that are used for
  the implementation.  Having them publically visible in the interface
  would break wrapping (python and TK).

  This object supports only a limited subset of Abaqus element types, namely
  what we have needed.
*/
class AbaqusInputReaderHelper : public CommandStyleFileReader
{
public:

  // Interval of lines read at which to make a progress update call.
  static const int progessInterval = 1000;

  // Constructor
  // stream - An already-opened input stream (file or string)
  // streamSize - File size.  You can set it to zero if you don't know it
  //              in advance, but then no progress updates occur.
  // boss - The vtk object to which to report progress updates. Usually this
  //        will be the calling object.  You may set it to NULL, in which
  //        case no progress updates will occur.
  // model - Object in which to store read data.
  AbaqusInputReaderHelper (std::istream&             stream,
                      long                      streamSize,
                      vtkAlgorithm*             boss,
                      vtkboneFiniteElementModel* model);

  int GetAbortStatus() {return abortExecute;}

  // Typedef for a function call that handles messages.
  typedef void (*messageHandler_t)(vtkObject*, const std::string&);

  // Set a VTK object to report messages to, also set (typically static)
  // functions to handle debug/warning messages.
  void RegisterMessageObject (vtkObject* obj,
                              messageHandler_t debugCall,
                              messageHandler_t warningCall);

protected:

  virtual int Finish();

  // Override DebugMessage and WarningMessage from CommandStyleDataReader
  // with out own that will use the VTK Debug/Warning facilities
  virtual void DebugMessage (const std::string& msg);
  virtual void WarningMessage (const std::string& msg);

  // Reimplemented from the base class to support progress indicator
  // and abortExecute flag.
  int GetLine();

  int IsCommand ();
  int IsCommentLine();

  // The following are all Section Handlers.
  // They are registered in the constructor.
  int Read_HEADING();
  int Read_NODE();
  int Read_ELEMENT();
  int Read_NSET();
  int Read_ELSET();
  int Read_MATERIAL();
  int Read_ELASTIC();
  int Read_SOLID_SECTION();
  int Read_STEP();
  int Read_STATIC();
  int Read_BOUNDARY();
  int Read_CLOAD();
  int Read_END_STEP();

  vtkObject* messageObject;
  messageHandler_t debugHandler;
  messageHandler_t warningHandler;

  long                       streamSize;
  vtkAlgorithm*              boss;
  vtkboneFiniteElementModel*  model;
  int                        abortExecute;

  typedef std::map<std::string, std::string> parameters_t;
  parameters_t               parameters;

  std::string                currentMaterial;
  std::string                allElementsSet;

};

#endif  // __AbaqusInputReaderHelper_h
