/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneN88ModelReader - read n88model files
// .SECTION Description
// vtkboneN88ModelReader is a source object that reads .n88model files that
// represent finite element models.
//
// vtkboneN88ModelReader creates a vtkboneFiniteElementModel dataset.
//
// .SECTION See Also
// vtkboneFiniteElementModel vtkboneN88ModelWriter


#ifndef __vtkboneN88ModelReader_h
#define __vtkboneN88ModelReader_h

#include "vtkboneFiniteElementModelAlgorithm.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkUnstructuredGrid;
class vtkboneConstraint;

class VTKBONE_EXPORT vtkboneN88ModelReader : public vtkboneFiniteElementModelAlgorithm
{
public:
  static vtkboneN88ModelReader *New();
  vtkTypeMacro(vtkboneN88ModelReader, vtkboneFiniteElementModelAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set/get the file name of the file to read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Set/get whether to read the MaterialTable.
  vtkSetMacro(ReadMaterials, int);
  vtkGetMacro(ReadMaterials, int);
  vtkBooleanMacro(ReadMaterials, int);

  // Description:
  // Get the active problem name.
  vtkGetStringMacro(ActiveSolution);

  // Description:
  // Get the active problem name.
  // Note that if an ActiveSolution exists in the file, then the ActiveProblem
  // will be set from that, and not from the ActiveProblem attribute.
  vtkGetStringMacro(ActiveProblem);

  // Description:
  // Get the active part name used by the ActiveProblem.
  vtkGetStringMacro(ActivePart);

protected:
  vtkboneN88ModelReader();
  ~vtkboneN88ModelReader();

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;

  int ReadDataFromNetCDFFile(int ncid, vtkboneFiniteElementModel* model);
  int ReadProblem(int ncid, vtkboneFiniteElementModel* model);
  int ReadAttributes(int ncid, vtkboneFiniteElementModel* model);
  int ReadNodes(int ncid, vtkboneFiniteElementModel* model);
  int ReadMaterialTable(int ncid, vtkboneFiniteElementModel* model);
  int ReadElements(int ncid, vtkboneFiniteElementModel* model);
  int ReadConstraints(int ncid, vtkboneFiniteElementModel* model);
  int ReadConvergenceSet(int ncid,vtkboneFiniteElementModel *model);
  int ReadConstraint(int constraints_ncid,const char* name,vtkboneConstraint*& constraint);
  int ReadSets(int ncid, vtkboneFiniteElementModel* model);
  int ReadSolutions(int ncid, vtkboneFiniteElementModel* model);

  // not publically modifiable
  vtkSetStringMacro(ActiveSolution);
  vtkSetStringMacro(ActiveProblem);
  vtkSetStringMacro(ActivePart);

  char* FileName;
  int ReadMaterials;
  char* ActiveSolution;
  char* ActiveProblem;
  char* ActivePart;

private:
  // Prevent compiler from making public versions of these.
  vtkboneN88ModelReader(const vtkboneN88ModelReader&);
  void operator=(const vtkboneN88ModelReader&);
};

#endif

