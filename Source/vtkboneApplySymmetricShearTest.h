/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneApplySymmetricShearTest
    @brief   Generates a finite element mesh that
 corresponds to a symmetric shear test.


 vtkboneApplySymmetricShearTest is an object that generates a finite element mesh
 corresponding to a symmetric shear test.  In a shear test, the side surfaces
 are displaced (tilted mostly) to correspond to a given shear strain.

 Refer to vtkboneApplyTestBase for details regarding the orientation of the
 test.

 This object requires two inputs.  Input 0 must be a vtkUnstructuredGrid (or an
 existing vtkboneFiniteElementModel) giving the segmented geometry of the
 model, while input 2 must be the vtkboneMaterialTable object that will be
 associated with the model.

 The following Constraints are generated:

 - "face_x0_lateral"
 - "face_x1_lateral"
 - "face_y0_lateral"
 - "face_y1_lateral"

 If ConfineSidesVertically is true, the following additional constraints
 are generated:

 - "face_x0_vertical"
 - "face_x1_vertical"
 - "face_y0_vertical"
 - "face_y1_vertical"

 If ConfineTopAndBottomVertically is true, the following additional constraints
 are generated:

 - "face_z0_vertical"
 - "face_z1_vertical"

 Note that for these sets, "lateral" means in the x and y directions in
 the Test Frame, while "vertical" means in the z direction in the Test Frame.

    @sa
 vtkboneApplyDirectionalShearTest
 vtkboneApplyTestBase vtkboneFiniteElementModel vtkboneConstraint
 vtkboneSolverParameters vtkboneMaterialTable
*/

#ifndef __vtkboneApplySymmetricShearTest_h
#define __vtkboneApplySymmetricShearTest_h

#include "vtkboneApplyTestBase.h"
#include "vtkboneWin32Header.h"


class VTKBONE_EXPORT vtkboneApplySymmetricShearTest : public vtkboneApplyTestBase
{
public:
  static vtkboneApplySymmetricShearTest *New();
  vtkTypeMacro(vtkboneApplySymmetricShearTest, vtkboneApplyTestBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void PrintParameters(ostream& os, vtkIndent indent);

  //@{
  /*! Sets the amount of apparant engineering shear strain. In this context
      it is "apparant" because it is taken over the test volume as a whole.
      Default is 0.01 . */
  vtkSetMacro(ShearStrain, double);
  vtkGetMacro(ShearStrain, double);
  //@}

  //@{
  /*! If True, restrict nodes on the side faces from moving in the z
      direction (in the Test Frame).  Default is True. */
  vtkSetMacro(ConfineSidesVertically, int);
  vtkGetMacro(ConfineSidesVertically, int);
  vtkBooleanMacro(ConfineSidesVertically, int);
  //@}

  //@{
  /*! If True, restrict nodes on the top and bottom faces from moving in
      the z direction (in the Test Frame).  Default is True. */
  vtkSetMacro(ConfineTopAndBottomVertically, int);
  vtkGetMacro(ConfineTopAndBottomVertically, int);
  vtkBooleanMacro(ConfineTopAndBottomVertically, int);
  //@}

protected:
  vtkboneApplySymmetricShearTest();
  ~vtkboneApplySymmetricShearTest();

  double ShearStrain;
  int ConfineSidesVertically;
  int ConfineTopAndBottomVertically;

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;

  virtual int AddSideConstraints(vtkboneFiniteElementModel* model);
  virtual int AddTopAndBottomConstraints(vtkboneFiniteElementModel* model);
  virtual int AddConvergenceSet(vtkboneFiniteElementModel* model);
  virtual int AddPostProcessingSets(vtkboneFiniteElementModel* model);
  virtual int AddInformation(vtkboneFiniteElementModel* model);

private:
  vtkboneApplySymmetricShearTest(const vtkboneApplySymmetricShearTest&); // Not implemented
  void operator=(const vtkboneApplySymmetricShearTest&); // Not implemented
};

#endif

