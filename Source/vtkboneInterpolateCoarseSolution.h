/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneInterpolateCoarseSolution - Applies and interpolates a solution from a coarsed FE model.
//
// .SECTION Description
// This is intended to be used after solving an FE problem that was reduced in size
// using vtkboneCoarsenModel, or that was generated from an image to which
// vtkboneDecimateImage was ap[lied. After solving the coarsened problem, the solution thus
// obtained can be added to the original model. The solution is interpolated to
// grid points (nodes) that are not present in the coarsened problem.
//
// .SECTION See Also
// vtkboneFiniteElementModel vtkboneCoarsenModel vtkboneDecimateImage

#ifndef __vtkboneInterpolateCoarseSolution_h
#define __vtkboneInterpolateCoarseSolution_h

#include "vtkboneFiniteElementModelAlgorithm.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneInterpolateCoarseSolution : public vtkboneFiniteElementModelAlgorithm
{
public:
  static vtkboneInterpolateCoarseSolution *New();
  vtkTypeMacro(vtkboneInterpolateCoarseSolution,vtkboneFiniteElementModelAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro(SolutionArray, vtkDataArray);  

protected:
  vtkboneInterpolateCoarseSolution();
  ~vtkboneInterpolateCoarseSolution();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  vtkDataArray* SolutionArray;

private:
  vtkboneInterpolateCoarseSolution(const vtkboneInterpolateCoarseSolution&);  // Not implemented.
  void operator=(const vtkboneInterpolateCoarseSolution&);  // Not implemented.
};

#endif
