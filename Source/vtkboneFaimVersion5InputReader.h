/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneFaimVersion5InputReader - read input files for FAIM finite element solver
// .SECTION Description
// vtkboneFaimVersion5InputReader is a source object that reads .inp files for FAIM.
//
// vtkboneFaimVersion5InputReader creates a vtkboneFiniteElementModel dataset.
//
// Constraints if present will be added with the following names:
//   "FIXED CONSTRAINTS"
//   "DISPLACEMENT CONSTRAINTS"
//   "FORCE CONSTRAINTS"
//
// Node sets if present will be added with the names "NODE SET 1",
// "NODE SET 2", etc...
//
// .SECTION See Also
// vtkboneFiniteElementModel vtkboneN88ModelWriter


#ifndef __vtkboneFaimVersion5InputReader_h
#define __vtkboneFaimVersion5InputReader_h

#include "vtkboneFiniteElementModelAlgorithm.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkUnstructuredGrid;

class VTKBONE_EXPORT vtkboneFaimVersion5InputReader : public vtkboneFiniteElementModelAlgorithm
{
public:
  static vtkboneFaimVersion5InputReader *New();
  vtkTypeMacro(vtkboneFaimVersion5InputReader, vtkboneFiniteElementModelAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set/get the file name of the FAIM output file to read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkboneFaimVersion5InputReader();
  ~vtkboneFaimVersion5InputReader();

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);

  char *FileName;

private:
  // Prevent compiler from making public versions of these.
  vtkboneFaimVersion5InputReader(const vtkboneFaimVersion5InputReader&);
  void operator=(const vtkboneFaimVersion5InputReader&);
};

#endif

