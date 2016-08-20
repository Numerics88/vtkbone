/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneCoarsenModel - Generates a reduced-resolution FE model.
//
// .SECTION Description
// vtkbonevtkboneCoarsenModel takes as input a vtkboneFiniteElementModel, and
// generates a new vtkboneFiniteElement model that has elements (vtkCells)
// that have twice the linear dimension (so 8 times the volume). Every
// essential feature of the FE model is translated to these new larger
// elements, including Constraints such as boundary conditions, and
// associated element and node sets. 2x2x2 groups of input elements
// are mapped onto a single output element. Has long as at least
// one of the locations in the corresponding 2x2x2 input region actually
// has a element (Cell), then an output element (Cell) will be
// generated there. Material properties are averaged over the corresponding
// 2x2x2 input elements (Cells), which a non-existant element is treated
// as value zero for the purposes of calculating the average.
//
// If the input model has exactly one input material, then the output
// model will have 8 materials (implemented as a material array).
// Otherwise, a material array will be used that has length equal
// to the number of output elements, and every output element will be
// assigned its own material properties.
//
// Currently, elastoplastic materials in the input are converted to
// linear materials for the output.
//
// .SECTION See Also
// vtkboneFiniteElementModel vtkboneInterpolateCoarseSolution

#ifndef __vtkboneCoarsenModel_h
#define __vtkboneCoarsenModel_h

#include "vtkboneFiniteElementModelAlgorithm.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneCoarsenModel : public vtkboneFiniteElementModelAlgorithm
{
public:
  static vtkboneCoarsenModel *New();
  vtkTypeMacro(vtkboneCoarsenModel,vtkboneFiniteElementModelAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkboneCoarsenModel() {}
  ~vtkboneCoarsenModel() {}

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);
  virtual int SimpleExecute(vtkboneFiniteElementModel* input,
                            vtkboneFiniteElementModel* output);

  virtual int GeneratePointCoordinates(vtkboneFiniteElementModel* output,
                                       const unsigned int* outputPointGrid,
                                       unsigned int outputDims[3],
                                       double outputSpacing[3],
                                       double bounds[6],
                                       vtkIdType nOutputPoints);
  virtual int GenerateCells(vtkboneFiniteElementModel* output,
                            const unsigned int* outputPointGrid,
                            const unsigned int* outputCellGrid,
                            unsigned int outputDims[3],
                            vtkIdType nOutputCells);
  virtual int GenerateMaterials(
                   vtkboneFiniteElementModel* output,
                   vtkboneFiniteElementModel* input,
                   const unsigned int* reverseCellMap);
  virtual int GenerateMaterialsSingleInputMaterial(
                   vtkboneFiniteElementModel* output,
                   vtkboneFiniteElementModel* input,
                   const unsigned int* reverseCellMap);
  virtual int GenerateConstraints(vtkboneFiniteElementModel* output,
                                  vtkboneFiniteElementModel* input,
                                  const unsigned int* pointMap);
  virtual int GenerateConvergenceSet(vtkboneFiniteElementModel* output,
                                     vtkboneFiniteElementModel* input,
                                     const unsigned int* pointMap);
  virtual int GenerateNodeAndElementSets(vtkboneFiniteElementModel* output,
                                         vtkboneFiniteElementModel* input,
                                         const unsigned int* pointMap,
                                         const vtkIdType* cellMap);
  virtual int GenerateAdditionalInformation(vtkboneFiniteElementModel* output,
                                            vtkboneFiniteElementModel* input);

private:
  vtkboneCoarsenModel(const vtkboneCoarsenModel&);  // Not implemented.
  void operator=(const vtkboneCoarsenModel&);  // Not implemented.
};

#endif
