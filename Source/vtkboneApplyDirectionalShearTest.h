/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneApplyDirectionalShearTest - Generates a finite element mesh that
// corresponds to a directional shear test.
//
// .SECTION Description
// vtkboneApplyDirectionalShearTest is an object that generates a finite element mesh
// corresponding to a directional shear test.  In a directional shear test, the Bottom surface is
// fixed in all directions, while the top surface is contrained in the z
// direction, and shifted in the x,y plane by an amount given by ShearVector
// (optionally scaled by the model height, as determined by 
// ScaleShearToHeight).
//
// Refer to vtkboneApplyTestBase for details regarding the orientation of the
// test.
//
// This object requires two inputs.  Input 0 must be a vtkUnstructuredGrid (or an
// existing vtkboneFiniteElementModel) giving the segmented geometry of the
// model, while input 2 must be the vtkboneMaterialTable object that will be
// associated with the model.
//
// The following Constraints are generated:
//
// - "bottom_fixed" : All fixed nodes constraints from node set "face_z0".
// - "top_fixed" : All fixed nodes constraints from node set "face_z1".
// - "top_displacement" : All displacement node constraints from node set "face_z1".
//
// .SECTION See Also
// vtkboneApplySymmetricShearTest
// vtkboneFiniteElementModelvtkboneConstraint
// vtkboneSolverParameters vtkboneMaterialTable

#ifndef __vtkboneApplyDirectionalShearTest_h
#define __vtkboneApplyDirectionalShearTest_h

#include "vtkboneApplyTestBase.h"
#include "vtkboneWin32Header.h"


class VTKBONE_EXPORT vtkboneApplyDirectionalShearTest : public vtkboneApplyTestBase 
{
public:
  static vtkboneApplyDirectionalShearTest *New();
  vtkTypeMacro(vtkboneApplyDirectionalShearTest, vtkboneApplyTestBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  void PrintParameters(ostream& os, vtkIndent indent);

  // Description:
  // Sets the amount of shear as a vector in the x,y plane (in the Test Frame).
  // Units are length units if ScaleShearToHeight is False, otherwise
  // a unitless ratio.  Default is (0.01,0) .
  vtkSetVector2Macro(ShearVector, double);
  vtkGetVector2Macro(ShearVector, double);

  // Description:
  // If True, the ShearVector will be multiplied by the height of the model
  // extent (in the z direction in the Test Frame) to determine the
  // shear displacement applied.
  vtkSetMacro(ScaleShearToHeight, int);
  vtkGetMacro(ScaleShearToHeight, int);
  vtkBooleanMacro(ScaleShearToHeight, int);

protected:
  vtkboneApplyDirectionalShearTest();
  ~vtkboneApplyDirectionalShearTest();

  double ShearVector[2];
  int ScaleShearToHeight;

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  virtual int AddTopAndBottomConstraints(vtkboneFiniteElementModel* model);
  virtual int AddConvergenceSet(vtkboneFiniteElementModel* model);
  virtual int AddPostProcessingSets(vtkboneFiniteElementModel* model);
  virtual int AddInformation(vtkboneFiniteElementModel* model);
  
private:
  vtkboneApplyDirectionalShearTest(const vtkboneApplyDirectionalShearTest&); // Not implemented
  void operator=(const vtkboneApplyDirectionalShearTest&); // Not implemented
};

#endif

