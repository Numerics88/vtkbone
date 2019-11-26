/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneAbaqusInputReader - A class to read Abaqus input files.
// .SECTION Description
//
// The data will be written in a VTK object of type vtkboneFiniteElementModel.
//
// This object supports only a limited subset of Abaqus features.  The
// following Abaqus keywords are supported.  Note that note every optional
// parameter for the keywords is supported:
//  - HEADING
//  - NODE
//  - ELEMENT
//  - NSET
//  - ELSET
//  - MATERIAL
//  - ELASTIC
//  - SOLID SECTION
//  - STEP
//  - STATIC
//  - BOUNDARY
//  - CLOAD
//
// Unrecognized keywords and parameters will be ignored.
//
// NOTE:    If you have a data file that you expect can be represented as
//          a vtkboneFiniteElementModel, but are unable to read it with
//          this class, please contact Numerics88 Solution Ltd.
//
// .SECTION See Also
// vtkboneFiniteElementModel vtkboneAbaqusInputWriter

#ifndef __vtkboneAbaqusInputReader_h
#define __vtkboneAbaqusInputReader_h

#include "vtkboneFiniteElementModelAlgorithm.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneAbaqusInputReader : public vtkboneFiniteElementModelAlgorithm
{
public:

  static vtkboneAbaqusInputReader* New();
  vtkTypeMacro(vtkboneAbaqusInputReader,vtkboneFiniteElementModelAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set/get the file name of the ABAQUS Input file to read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  //BTX
  static void DebugMessage (vtkObject* self, const std::string& msg);
  static void WarningMessage (vtkObject* self, const std::string& msg);
  //ETX

protected:

  vtkboneAbaqusInputReader();
  ~vtkboneAbaqusInputReader();

  int RequestData (vtkInformation *,
                   vtkInformationVector **,
                   vtkInformationVector *);

  char *FileName;

private:
  // Prevent compiler from making public versions of these.
  vtkboneAbaqusInputReader (const vtkboneAbaqusInputReader&);
  void operator= (const vtkboneAbaqusInputReader&);
};

#endif
