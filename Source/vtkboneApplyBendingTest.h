/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneApplyBendingTest - Generates a finite element mesh that
// corresponds to a bending test.
//
// .SECTION Description
// vtkboneApplyBendingTest is an object that generates a finite element mesh
// corresponding to a bending test.  In a bending test, the Top and Bottom
// surfaces are tilted in opposing directions.
//
// The tilting planes are defined by a Neutral Axis, which is the line on
// the x-y plane which is not displaced by the tilt (i.e. the tilt axis).
//
// Refer to vtkboneApplyTestBase for details regarding the orientation of the
// test.
//
// This object requires two inputs.  Input 0 must be a vtkUnstructuredGrid (or an
// existing vtkboneFiniteElementModel) giving the segmented geometry of the
// model, while input 2 must be the vtkboneMaterialTable object that will be
// associated with the model.
//
// Very often, you will want to use the center of mass as the neutral axis
// origin.  For this purpose, you can use either vtkboneTensorOfInertia or
// vtkMassProperties to obtain the center of mass.
//
// The following Constraints are generated:
//
// - "bottom_fixed" : All fixed nodes constraints from node set "face_z0".
// - "bottom_displacement" : All displacement node constraints from node set "face_z0".
// - "top_fixed" : All fixed nodes constraints from node set "face_z1".
// - "top_displacement" : All displacement node constraints from node set "face_z1".
//
// The object information ROTATION_CENTER is set to the NeutralAxis,
// converted to the Data Frame.  As there is no z value of the NeutralAxis,
// the equivalent sense of the ROTATION_CENTER is set to the mid-point of
// the bounds along that axis.
//
// .SECTION See Also
// vtkboneFiniteElementModel vtkboneConstraint
// vtkboneSolverParameters vtkboneMaterialTable

#ifndef __vtkboneApplyBendingTest_h
#define __vtkboneApplyBendingTest_h

#include "vtkboneApplyTestBase.h"
#include "vtkboneWin32Header.h"


class VTKBONE_EXPORT vtkboneApplyBendingTest : public vtkboneApplyTestBase
{
public:
  static vtkboneApplyBendingTest *New();
  vtkTypeMacro(vtkboneApplyBendingTest, vtkboneApplyTestBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void PrintParameters(ostream& os, vtkIndent indent);

  // Description:
  // Specifies the x,y coordinates of the Neutral Axis origin.
  // Default is 0,0 .
  vtkSetVector2Macro(NeutralAxisOrigin, double);
  vtkGetVector2Macro(NeutralAxisOrigin, double);

  // Description:
  // Specifies the angle of the Neutral Axis origin.  0 is along the x axis.
  // Units are radians.  Default is pi/2 (i.e. along the y axis).
  vtkSetMacro(NeutralAxisAngle, double);
  vtkGetMacro(NeutralAxisAngle, double);

  // Description:
  // Specifies the bending angle.  The Top and Bottom surfaces are each tilted
  // by half this amount.  Units are radians.  Default is 0.01745,
  // corresponding to 1º.
  vtkSetMacro(BendingAngle, double);
  vtkGetMacro(BendingAngle, double);

protected:
  vtkboneApplyBendingTest();
  ~vtkboneApplyBendingTest();

  double NeutralAxisOrigin[2];
  double NeutralAxisAngle;
  double BendingAngle;

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;

  virtual int AddTopAndBottomConstraints(vtkboneFiniteElementModel* model);
  virtual int AddConvergenceSet(vtkboneFiniteElementModel* model);
  virtual int AddPostProcessingSets(vtkboneFiniteElementModel* model);
  virtual int AddInformation(vtkboneFiniteElementModel* model);

private:
  vtkboneApplyBendingTest(const vtkboneApplyBendingTest&); // Not implemented
  void operator=(const vtkboneApplyBendingTest&); // Not implemented
};

#endif

