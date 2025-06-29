/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneInterpolateCoarseSolution
    @brief   Applies and interpolates a solution from a coarsed FE model.


 This is intended to be used after solving an FE problem that was reduced in size
 using vtkboneCoarsenModel, or that was generated from an image to which
 vtkboneDecimateImage was ap[lied. After solving the coarsened problem, the solution thus
 obtained can be added to the original model. The solution is interpolated to
 grid points (nodes) that are not present in the coarsened problem.

    @sa
 vtkboneFiniteElementModel vtkboneCoarsenModel vtkboneDecimateImage
*/

#ifndef __vtkboneInterpolateCoarseSolution_h
#define __vtkboneInterpolateCoarseSolution_h

#include "vtkboneFiniteElementModelAlgorithm.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneInterpolateCoarseSolution : public vtkboneFiniteElementModelAlgorithm
{
public:
  static vtkboneInterpolateCoarseSolution *New();
  vtkTypeMacro(vtkboneInterpolateCoarseSolution,vtkboneFiniteElementModelAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkGetObjectMacro(SolutionArray, vtkDataArray);

protected:
  vtkboneInterpolateCoarseSolution();
  ~vtkboneInterpolateCoarseSolution();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;

  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  vtkDataArray* SolutionArray;

private:
  vtkboneInterpolateCoarseSolution(const vtkboneInterpolateCoarseSolution&);  // Not implemented.
  void operator=(const vtkboneInterpolateCoarseSolution&);  // Not implemented.
};

#endif
