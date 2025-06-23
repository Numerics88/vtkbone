/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneFiniteElementModelGenerator
    @brief   Class for generating a finite element mesh.


 vtkboneFiniteElementModelGenerator is an object that combines a
 vtkUnstructuredGrid object and a vtkboneMaterialTable object to produce
 a vtkboneFiniteElementModel object.

 Input 0 must be a vtkUnstructuredGrid (or an
 existing vtkboneFiniteElementModel) giving the segmented geometry of the
 model, while input 2 must be the vtkboneMaterialTable object that will be
 associated with the model.

 If your data consist of points on a grid, you can first convert it to the
 necessary elements using vtkboneImageToMesh.

 This object does not apply any constraints to the model.  Derived classes
 of this base class me however do so.  (See for example
 vtkboneApplyCompressionTest.)

    @sa
 vtkboneApplyCompressionTest vtkboneFiniteElementModel vtkboneImageToMesh vtkboneConstraint
 vtkboneSolverParameters vtkboneMaterialTable vtkboneFiniteElementModel vtkMeshWriter
*/

#ifndef __vtkboneFiniteElementModelGenerator_h
#define __vtkboneFiniteElementModelGenerator_h

#include "vtkboneFiniteElementModelAlgorithm.h"
#include "vtkboneWin32Header.h"

#include "vtkboneFiniteElementModel.h"   // Needed for enum definitions.
#include "vtkboneConstraint.h"   // Needed for enum definitions.


class VTKBONE_EXPORT vtkboneFiniteElementModelGenerator : public vtkboneFiniteElementModelAlgorithm
{
public:
  static vtkboneFiniteElementModelGenerator *New();
  vtkTypeMacro(vtkboneFiniteElementModelGenerator, vtkboneFiniteElementModelAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /*! Sets whether PedigreeIds arraysare generated and associated with
      Point/Cell data.	These are standard VTK arrays that can be used to
      trace back original Point Ids after a VTK filter has been used to
      select a subset of the original object. This is very useful for
      finding point sets for constraints, and the default is on.  However,
      if you know you do not need these arrays, you can save some memory by
      not generating them. */
  vtkSetMacro(AddPedigreeIdArrays, int);
  vtkGetMacro(AddPedigreeIdArrays, int);
  vtkBooleanMacro(AddPedigreeIdArrays, int);
  //@}

  void SetModelSourceDescription(const char*)
  {
 	vtkWarningMacro(<< "SetModelSourceDescription is deprecated.\n");
  }

protected:
  vtkboneFiniteElementModelGenerator();
  ~vtkboneFiniteElementModelGenerator();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;

  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  int AddPedigreeIdArrays;

private:
  vtkboneFiniteElementModelGenerator(const vtkboneFiniteElementModelGenerator&); // Not implemented
  void operator=(const vtkboneFiniteElementModelGenerator&); // Not implemented
};

#endif

