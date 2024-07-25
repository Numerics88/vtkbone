/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneFaimVersion5OutputReader
    @brief   read results file from FAIM finite element solver

 vtkboneFaimVersion5OutputReader is a source object that reads .dat files output by faim.

 vtkboneFaimVersion5OutputReader creates a vtkboneFiniteElementModel dataset.
*/

#ifndef __vtkboneFaimVersion5OutputReader_h
#define __vtkboneFaimVersion5OutputReader_h

#include "vtkboneFiniteElementModelAlgorithm.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkboneFiniteElementModel;

class VTKBONE_EXPORT vtkboneFaimVersion5OutputReader : public vtkboneFiniteElementModelAlgorithm
{
public:
  static vtkboneFaimVersion5OutputReader *New();
  vtkTypeMacro(vtkboneFaimVersion5OutputReader, vtkboneFiniteElementModelAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /*! Set/get the file name of the FAIM output file to read. */
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  //@}

  //@{
  /*! Get the nodal displacements (mm).  Default is on. */
  vtkSetMacro(ReadNodalDisplacements, int);
  vtkGetMacro(ReadNodalDisplacements, int);
  vtkBooleanMacro(ReadNodalDisplacements, int);
  //@}

  //@{
  /*! Get the nodal reaction forces (N).  Default is on. */
  vtkSetMacro(ReadNodalReactionForces, int);
  vtkGetMacro(ReadNodalReactionForces, int);
  vtkBooleanMacro(ReadNodalReactionForces, int);
  //@}

  //@{
  /*! Get the element stresses (N/mm2).  Default is on. */
  vtkSetMacro(ReadElementStresses, int);
  vtkGetMacro(ReadElementStresses, int);
  vtkBooleanMacro(ReadElementStresses, int);
  //@}

  //@{
  /*! Get the element strains (N/mm2).	Default is on. */
  vtkSetMacro(ReadElementStrains, int);
  vtkGetMacro(ReadElementStrains, int);
  vtkBooleanMacro(ReadElementStrains, int);
  //@}

protected:
  vtkboneFaimVersion5OutputReader();
  ~vtkboneFaimVersion5OutputReader();

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;

  char *FileName;

  int ReadNodalDisplacements;
  int ReadNodalReactionForces;
  int ReadElementStresses;
  int ReadElementStrains;

private:
  // Prevent compiler from making public versions of these.
  vtkboneFaimVersion5OutputReader(const vtkboneFaimVersion5OutputReader&);
  void operator=(const vtkboneFaimVersion5OutputReader&);
};

#endif

