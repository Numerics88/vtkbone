/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneAbaqusInputWriter - writes an Abaqus input file.
// .SECTION Description
// Takes as input a vtkboneFiniteElementModel and writes as output an
// Abaqus input file.
//
// .SECTION See Also
// vtkboneFiniteElementModel vtkboneN88ModelWriter vtkboneAbaqusInputReader

#ifndef __vtkboneAbaqusInputWriter_h
#define __vtkboneAbaqusInputWriter_h

#include "vtkWriter.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkPoints;
class vtkboneFiniteElementModel;
class vtkDoubleArray;
class vtkIdList;
class vtkIdTypeArray;
class vtkCharArray;
class vtkboneConstraint;

class VTKBONE_EXPORT vtkboneAbaqusInputWriter : public vtkWriter
{
public:
  static vtkboneAbaqusInputWriter *New();
  vtkTypeMacro(vtkboneAbaqusInputWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Specify file name of file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkboneAbaqusInputWriter();
  ~vtkboneAbaqusInputWriter();

  virtual int FillInputPortInformation (int port, vtkInformation *info) override;

  virtual void WriteData() override;

  //BTX
  virtual int WriteHeading (std::ostream& f, vtkboneFiniteElementModel* model);
  virtual int WriteNodes (std::ostream& f, vtkboneFiniteElementModel* model);
  virtual int WriteElements (std::ostream& f, vtkboneFiniteElementModel* model);
  virtual int WriteMaterials (std::ostream& f, vtkboneFiniteElementModel* model);
  virtual int WriteNodeSets (std::ostream& f, vtkboneFiniteElementModel* model);
  virtual int WriteElementSets (std::ostream& f, vtkboneFiniteElementModel* model);
  virtual int WriteSolidSections (std::ostream& f, vtkboneFiniteElementModel* model);
  virtual int WriteStep (std::ostream& f, vtkboneFiniteElementModel* model);
  virtual int WriteBoundaries (std::ostream& f, vtkboneFiniteElementModel* model);
  virtual int WriteLoads (std::ostream& f, vtkboneFiniteElementModel* model);
  int WriteIndexArray (std::ostream& f, vtkIdTypeArray* data);
  std::string SpacesToUnderscores (std::string s);
  //ETX

  char* FileName;

private:
  vtkboneAbaqusInputWriter(const vtkboneAbaqusInputWriter&); // Not implemented
  void operator=(const vtkboneAbaqusInputWriter&); // Not implemented
};

#endif

