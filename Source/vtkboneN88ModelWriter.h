/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneN88ModelWriter - writes an n88model file.
// .SECTION Description
// Takes as input a vtkboneFiniteElementModel and writes a n88model file,
// suitable for input to the Numerics88 finite element solver.
//
// Note that the output file format is 1-indexed.  (vtkboneFiniteElementModel is 0-indexed.)
//
// Any arrays associated with the Points or the Cells are assumed to be
// solution values, and are written as such, provided that (1) they are
// named, and (2) they are not specified as any of the special arrays: Scalars,
// Normals, GlobalIds, PedigreeIds.
//
// .SECTION See Also
// vtkboneFiniteElementModel vtkboneFiniteElementModelGenerator

#ifndef __vtkboneN88ModelWriter_h
#define __vtkboneN88ModelWriter_h

#include "vtkWriter.h"
#include "vtkboneWin32Header.h"
#include <set>

// Forward declarations
class vtkPoints;
class vtkboneFiniteElementModel;
class vtkDoubleArray;
class vtkIdList;
class vtkIdTypeArray;
class vtkCharArray;
class vtkDataArrayCollection;
class vtkboneConstraint;
class vtkDataSetAttributes;

class VTKBONE_EXPORT vtkboneN88ModelWriter : public vtkWriter 
{
public:
  static vtkboneN88ModelWriter *New();
  vtkTypeMacro(vtkboneN88ModelWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Specify file name of file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Set/get whether use compression when writing the data file.
  vtkSetMacro(Compression, int);
  vtkGetMacro(Compression, int);
  vtkBooleanMacro(Compression, int);

protected:
  vtkboneN88ModelWriter();
  ~vtkboneN88ModelWriter();

  void WriteData();

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  int DefineNetCDFFile(int ncid, vtkboneFiniteElementModel* model);
  int DefineMaterialDefinitions(int ncid, vtkboneFiniteElementModel* model);
  int DefinePart(int ncid, vtkboneFiniteElementModel* model);
  int DefineMaterialTable(int ncid, vtkboneFiniteElementModel* model);
  int DefineConstraints(int ncid, vtkboneFiniteElementModel* model);
  int DefineConstraint(int constraint_ncid, vtkboneConstraint* constraint, vtkboneFiniteElementModel *model);
  int DefineProblem(int ncid, vtkboneFiniteElementModel* model);
  int DefineSets(int ncid, vtkboneFiniteElementModel* model);
  int DefineSolution(int ncid, vtkboneFiniteElementModel* model);
  int WriteDataToNetCDFFile(int ncid, vtkboneFiniteElementModel* model);
  int WriteMaterialDefinitions(int ncid, vtkboneFiniteElementModel* model);
  int WriteNodes(int ncid, vtkboneFiniteElementModel* model);
  int WriteMaterialTable(int ncid, vtkboneFiniteElementModel* model);
  int WriteElements(int ncid, vtkboneFiniteElementModel* model);
  int WriteConstraints(int ncid, vtkboneFiniteElementModel* model);
  int WriteConstraint(int constraints_ncid,vtkboneConstraint* constraint,vtkboneFiniteElementModel* model);
  int WriteSets(int ncid, vtkboneFiniteElementModel* model);
  int WriteSolution(int ncid, vtkboneFiniteElementModel* model);

  //BTX
  int GetSolutionArrayNames(vtkDataSetAttributes* fieldData, std::set<std::string>& names);
  int GetNeededGaussPointGroups(vtkboneFiniteElementModel *model, std::set<size_t>& nGaussPoints);
  //ETX
  int WriteVTKDataArrayToNetCDF(int ncid, int varid, vtkDataArray* data);
  int WriteVTKDataArrayToNetCDF(int ncid, int varid, vtkDataArray* data, size_t dim1);
  int WriteVTKDataArrayToNetCDFOneIndexed(int ncid, int varid, vtkDataArray* data);
  int SetChunking (int ncid, int varid);

  char* FileName;
  int Compression;

private:
  vtkboneN88ModelWriter(const vtkboneN88ModelWriter&); // Not implemented
  void operator=(const vtkboneN88ModelWriter&); // Not implemented
};

#endif

