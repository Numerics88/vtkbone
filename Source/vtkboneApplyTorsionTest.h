/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneApplyTorsionTest
    @brief   Generates a finite element mesh that
 corresponds to a torsion test.


 vtkboneApplyTorsionTest is an object that generates a finite element mesh
 corresponding to a torsion test.  In a torsion test, the Bottom
 surface is fixed (all directions), while the Top surface is rotated.

 Refer to vtkboneApplyTestBase for details regarding the orientation of the
 test.

 This object requires two inputs.  Input 0 must be a vtkUnstructuredGrid (or an
 existing vtkboneFiniteElementModel) giving the segmented geometry of the
 model, while input 2 must be the vtkboneMaterialTable object that will be
 associated with the model.

 Very often, you will want to use the center of mass as the torsion axis
 origin.  For this purpose, you can use either vtkboneTensorOfInertia or
 vtkMassProperties to obtain the center of mass.

 The following Constraints are generated:

 - "bottom_fixed" : All fixed nodes constraints from node set "face_z0".
 - "top_fixed" : All fixed nodes constraints from node set "face_z1".
 - "top_displacement" : All displacement node constraints from node set "face_z1".

 The object information ROTATION_CENTER is set to the TwistAxis,
 converted to the Data Frame.  As there is no z value of the TwistAxis,
 the equivalent sense of the ROTATION_CENTER is set to the mid-point of
 the bounds along that axis.

    @sa
 vtkboneFiniteElementModel vtkboneConstraint
 vtkboneSolverParameters vtkboneMaterialTable
*/

#ifndef __vtkboneApplyTorsionTest_h
#define __vtkboneApplyTorsionTest_h

#include "vtkboneApplyTestBase.h"
#include "vtkboneWin32Header.h"


class VTKBONE_EXPORT vtkboneApplyTorsionTest : public vtkboneApplyTestBase
{
public:
  static vtkboneApplyTorsionTest *New();
  vtkTypeMacro(vtkboneApplyTorsionTest, vtkboneApplyTestBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void PrintParameters(ostream& os, vtkIndent indent);

  //@{
  /*! Specifies the x,y coordinates of the Twist Axis origin. Default is
      0,0 . */
  vtkSetVector2Macro(TwistAxisOrigin, double);
  vtkGetVector2Macro(TwistAxisOrigin, double);
  //@}

  //@{
  /*! Specifies the rotation angle of the Top surface.	Units are radians.
      Default is 0.01745, corresponding to 1º. */
  vtkSetMacro(TwistAngle, double);
  vtkGetMacro(TwistAngle, double);
  //@}

protected:
  vtkboneApplyTorsionTest();
  ~vtkboneApplyTorsionTest();

  double TwistAxisOrigin[2];
  double TwistAngle;

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;

  virtual int AddTopAndBottomConstraints(vtkboneFiniteElementModel* model);
  virtual int AddConvergenceSet(vtkboneFiniteElementModel* model);
  virtual int AddPostProcessingSets(vtkboneFiniteElementModel* model);
  virtual int AddInformation(vtkboneFiniteElementModel* model);

private:
  vtkboneApplyTorsionTest(const vtkboneApplyTorsionTest&); // Not implemented
  void operator=(const vtkboneApplyTorsionTest&); // Not implemented
};

#endif

