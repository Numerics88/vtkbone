/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneApplyCompressionTest - Generates a finite element mesh that
// corresponds to a compression test.
//
// .SECTION Description
// vtkboneApplyCompressionTest is an object that generates a finite element mesh
// corresponding to a compression test.  The bottom surface is fixed while
// a fixed displacement (or force) boundary condition is applied to the top
// surface.
//
// This class generates the following types of standard Numerics88 tests:
//   - Uniaxial (default)
//   - Axial (set TopSurfaceContactFriction and BottomSurfaceContactFriction)
//   - Confined (in addition, set ConfineSides)
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
//     Note that these typically only exist if TopSurfaceContactFriction is true.
// - "top_displacement" : All displacement node constraints from node set "face_z1".
// - "pinned_nodes" : The pin node constraints; exists only if Pin is true.
// - "face_x0_fixed" : Fixed node constraints from node set "face_x0"; only if ConfineSides is true.
// - "face_x1_fixed" : Fixed node constraints from node set "face_x1"; only if ConfineSides is true.
// - "face_y0_fixed" : Fixed node constraints from node set "face_y0"; only if ConfineSides is true.
// - "face_y1_fixed" : Fixed node constraints from node set "face_y1"; only if ConfineSides is true.
//
// .SECTION See Also
// vtkboneFiniteElementModel vtkboneConstraint
// vtkboneSolverParameters vtkboneMaterialTable

#ifndef __vtkboneApplyCompressionTest_h
#define __vtkboneApplyCompressionTest_h

#include "vtkboneApplyTestBase.h"
#include "vtkboneWin32Header.h"


class VTKBONE_EXPORT vtkboneApplyCompressionTest : public vtkboneApplyTestBase 
{
public:
  static vtkboneApplyCompressionTest *New();
  vtkTypeMacro(vtkboneApplyCompressionTest, vtkboneApplyTestBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  void PrintParameters(ostream& os, vtkIndent indent);

  // Description:
  // Set/get an applied apparent level strain.
  // Negative for compression, positive for tension.
  // Default is -0.01 (corresponds to -1% apparent level strain).
  vtkSetMacro(AppliedStrain, double);
  vtkGetMacro(AppliedStrain, double);

  // Description:
  // Set/get the absolute displacement applied to the mesh.
  // If zero (default), it will be derived from the AppliedStrain
  // and the model dimensions.
  vtkSetMacro(AppliedDisplacement, double);
  vtkGetMacro(AppliedDisplacement, double);

  // Description:
  // Sets top surface contact friction to 0 (Off) or infinite (On).
  // Default is off.
  vtkSetMacro(TopSurfaceContactFriction, int);
  vtkGetMacro(TopSurfaceContactFriction, int);
  vtkBooleanMacro(TopSurfaceContactFriction, int);

  // Description:
  // Sets bottom surface contact friction to 0 (Off) or infinite (On).
  // Default is off.
  vtkSetMacro(BottomSurfaceContactFriction, int);
  vtkGetMacro(BottomSurfaceContactFriction, int);
  vtkBooleanMacro(BottomSurfaceContactFriction, int);

  // Description:
  // Prevents the side surfaces (face_x0, face_x1, face_y0, face_y1) from moving
  // in the x or y directions in the Test Frame.
  // Default is off.
  vtkSetMacro(ConfineSides, int);
  vtkGetMacro(ConfineSides, int);
  vtkBooleanMacro(ConfineSides, int);

  // Description:
  // Set whether to add a pin that fully defines a model.  This pin
  // prevents arbitrary lateral translation and rotation.  Without it,
  // uniaxial models are singular (although n88solver can still solve them).
  // If any of TopSurfaceContactFriction,
  // BottomSurfaceContactFriction or ConfineSides is True, then this option
  // is ignored, as it is then no longer relevant
  // Default is off.
  vtkSetMacro(Pin, int);
  vtkGetMacro(Pin, int);
  vtkBooleanMacro(Pin, int);

  // Description:
  // Set/get the Cell Id of the cell to be pinned.  Only applies if a pin is used.
  // Default is 0.  Note that if PinCellClosestToXYCenter, this value will
  // be changed on Update.
  vtkSetMacro(PinCellId, vtkIdType);
  vtkGetMacro(PinCellId, vtkIdType);

  // Description:
  // Set whether to locate the pinned element closest to the x,y center (in
  // the Test Frame).  If False, then the existing PinCellId is used.
  // It takes some computational time to find the cell closest to the center.
  // Where 2 elements are equidistant from the center, the one with smallest
  // z coordinate will be chosen.
  // Default is on.
  vtkSetMacro(PinCellClosestToXYCenter, int);
  vtkGetMacro(PinCellClosestToXYCenter, int);
  vtkBooleanMacro(PinCellClosestToXYCenter, int);

protected:
  vtkboneApplyCompressionTest();
  ~vtkboneApplyCompressionTest();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  virtual int AddTopAndBottomConstraints(vtkboneFiniteElementModel* model);
  virtual int AddSideConstraints(vtkboneFiniteElementModel* model);
  virtual int AddConvergenceSet(vtkboneFiniteElementModel* model);
  virtual int AddPostProcessingSets(vtkboneFiniteElementModel* model);
  virtual vtkIdType GetCellClosestToXYCenter(vtkboneFiniteElementModel* model);
  virtual int PinElement(vtkboneFiniteElementModel* model);
  virtual int AddInformation(vtkboneFiniteElementModel* model);
  
  int UsePressureBoundaryConditions;
  double AppliedStrain;
  double AppliedDisplacement;
  int TopSurfaceContactFriction;
  int BottomSurfaceContactFriction;
  int ConfineSides;
  int Pin;
  vtkIdType PinCellId;
  int PinCellClosestToXYCenter;

private:
  vtkboneApplyCompressionTest(const vtkboneApplyCompressionTest&); // Not implemented
  void operator=(const vtkboneApplyCompressionTest&); // Not implemented
};

#endif

