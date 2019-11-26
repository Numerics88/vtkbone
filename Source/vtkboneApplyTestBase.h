/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneApplyTestBase - Base class for applying a test to create a
//   vtkboneFiniteElementModel.
//
// .SECTION Description
// vtkboneApplyTestBase is an object that generates finite element input meshes.
// It is indended to be used as a base class for child classes that apply
// certain concrete types of test (i.e. vtkboneApplyCompressionTest).
//
// It requires two inputs.  Input 0 must be a vtkUnstructuredGrid (or an
// existing vtkboneFiniteElementModel) giving the segmented geometry of the
// model, while input 2 must be the vtkboneMaterialTable object that will be
// associated with the model.
//
// If your data consist of points on a grid, you can first convert it to the
// necessary elements using vtkboneImageToMesh.
//
// This object handles the geometric transformations necessary to apply
// tests along different test axes.  Two frames of reference are defined:
// (1) the coordinate system of the input data (the "Data Frame"), and
// (2) the coordinate system of the test (the "Test Frame").  In the Test
// Frame, tests have a constant orientation, with the test axis always
// being the z axis (in the compression test for example, the compression is
// applied along the z axis of the Test Frame).  The relation between the
// Test Frame and the Data frame is set by the value TestAxis according
// to the following table.
//
// Transformations from Test Frame to Data Frame:
//
// - With TestAxis=Z :  X -> X, Y -> Y, Z -> Z
// - With TestAxis=X :  X -> Y, Y -> Z, Z -> X
// - With TestAxis=Y :  X -> Z, Y -> X, Z -> Y
//
// In terms of rotations, for a setting of TestAxis=X, the transformation
// from the Test Frame to the Data Frame is a rotation of -90º about the
// x axis, followed by a -90º rotation about the z' axis.  For a setting
// of TestAxis=Y, the transformation from the Test Frame to the Data Frame
// is a rotation of 90º about the y axis, followed by a 90º rotation about
// the z' axis.  (These transformation sequences are not unique.)
//
// References to "Top" refer to the maximum z surface in the Test Frame;
// References to "Bottom" refer to the minimum z surface in the Test Frame.
// Likewise "Sides" refers to the x and y surfaces in the Test Frame.
//
// The output data coordinates are in the Data Frame.
//
// The following Node Sets are created:
//
// - "face_z0" : Nodes on bottom surface in Test Frame.
// - "face_z1" : Nodes on top surface in Test Frame.
// - "face_x0" : Nodes on x=xmin surface in Test Frame.
// - "face_x1" : Nodes on x=xmax surface in Test Frame.
// - "face_y0" : Nodes on y=ymin surface in Test Frame.
// - "face_y1" : Nodes on y=ymax surface in Test Frame.
//
// Corresponding element sets are also generated.
//
// Note that the options TopConstraintSpecificMaterial and UnevenTopSurface
// will affect which nodes are selected for face_z1; similarly for
// BottomConstraintSpecificMaterial and UnevenBottomSurface and face_x0.
//
// .SECTION See Also
// vtkboneFiniteElementModel vtkboneImageToMesh vtkboneConstraint
// vtkboneSolverParameters vtkboneMaterialTable


#ifndef __vtkboneApplyTestBase_h
#define __vtkboneApplyTestBase_h

#include "vtkboneFiniteElementModelGenerator.h"
#include "vtkboneWin32Header.h"

#include "vtkboneFiniteElementModel.h"   // Needed for enum definitions.
#include "vtkboneConstraint.h"   // Needed for enum definitions.


class VTKBONE_EXPORT vtkboneApplyTestBase : public vtkboneFiniteElementModelGenerator
{
public:
  static vtkboneApplyTestBase *New();
  vtkTypeMacro(vtkboneApplyTestBase, vtkboneFiniteElementModelGenerator);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void PrintParameters(ostream& os, vtkIndent indent);

  enum TestAxis_t {
    TEST_AXIS_X = 0,
    TEST_AXIS_Y = 1,
    TEST_AXIS_Z = 2
  };

  // Description:
  // Limit the applied boundary conditions on the top surface to the specified
  // material.
  // -1 indicates any material (default).
  vtkSetMacro(TopConstraintSpecificMaterial, int);
  vtkGetMacro(TopConstraintSpecificMaterial, int);

  // Description:
  // Limit the applied boundary conditions on the bottom surface to the specified
  // material.
  // -1 indicates any material (default).
  vtkSetMacro(BottomConstraintSpecificMaterial, int);
  vtkGetMacro(BottomConstraintSpecificMaterial, int);

  // Description:
  // In cases where the surface to which force or displacement
  // boundary conditions are on an uneven surface, setting this
  // option on will search for all nodes on the uneven surface.
  // The default, without this setting turned on, will only
  // find nodes that intersect the plane defined by the WholeExtent.
  // Default is off.
  vtkSetMacro(UnevenTopSurface, int);
  vtkGetMacro(UnevenTopSurface, int);
  vtkBooleanMacro(UnevenTopSurface, int);

  // Description:
  // Flag to set limit to depth of top surface, as measured from top boundary
  // of volume.  Only applies if UnevenTopSurface is true.
  // Default is off.
  vtkSetMacro(UseTopSurfaceMaximumDepth, int);
  vtkGetMacro(UseTopSurfaceMaximumDepth, int);
  vtkBooleanMacro(UseTopSurfaceMaximumDepth, int);

  // Description:
  // Sets the maximum depth of the top surface if UnevenTopSurface and
  // UseTopSurfaceMaximumDepth are true.
  vtkSetMacro(TopSurfaceMaximumDepth, double);
  vtkGetMacro(TopSurfaceMaximumDepth, double);
  vtkBooleanMacro(TopSurfaceMaximumDepth, double);

  // Description:
  // In cases where the surface to which force or displacement
  // boundary conditions are on an uneven surface, setting this
  // option on will search for all nodes on the uneven surface.
  // The default, without this setting turned on, will only
  // find nodes that intersect the plane defined by the WholeExtent.
  // Default is off.
  vtkSetMacro(UnevenBottomSurface, int);
  vtkGetMacro(UnevenBottomSurface, int);
  vtkBooleanMacro(UnevenBottomSurface, int);

  // Description:
  // Flag to set limit to depth of bottom surface, as measured from bottom boundary
  // of volume.  Only applies if UnevenBottomSurface is true.
  // Default is off.
  vtkSetMacro(UseBottomSurfaceMaximumDepth, int);
  vtkGetMacro(UseBottomSurfaceMaximumDepth, int);
  vtkBooleanMacro(UseBottomSurfaceMaximumDepth, int);

  // Description:
  // Sets the maximum depth of the bottom surface if UnevenBottomSurface and
  // UseBottomSurfaceMaximumDepth are true.
  vtkSetMacro(BottomSurfaceMaximumDepth, double);
  vtkGetMacro(BottomSurfaceMaximumDepth, double);
  vtkBooleanMacro(BottomSurfaceMaximumDepth, double);

  // Description:
  // Set/Get the testing axis (x=0, y=1, z=2)
  // Default is 2.
  vtkSetMacro(TestAxis, int);
  vtkGetMacro(TestAxis, int);

  // Description:
  // Given an sense (i.e. an axis direction) in the Test Frame, returns
  // the sense in the Data Frame.
  int DataFrameSense(int testFrameSense);

  // Description:
  // Given an sense (i.e. an axis direction) in the Data Frame, returns
  // the sense in the Test Frame.
  int TestFrameSense(int dataFrameSense);

  // Description:
  // Given an sense (i.e. an axis direction) in the Test Frame and a
  // polarity (0=negative, 1=positive), return the polarity flag in the Data Frame.
  int DataFramePolarity(int testFrameSense, int polarity);

  // Description:
  // Given bounds in the Data Frame, a sense (i.e. an axis direction) in the
  // Test Frame and a polarity in the Test Frame (0=negative, 1=postive),
  // return the value of the indicated bound.
  double TestFrameBound(double bounds[6], int testFrameSense, int polarity);

  // Description:
  // Given a VTK local point Id for a voxel in the Test Frame (in the range
  // 0 to 7), return the point Id in the Data Frame.
  int GetVoxelLocalId(int testFrameLocalId);

protected:
  vtkboneApplyTestBase();
  ~vtkboneApplyTestBase();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;

  int TopConstraintSpecificMaterial;
  int BottomConstraintSpecificMaterial;
  int UnevenTopSurface;
  int UseTopSurfaceMaximumDepth;
  double TopSurfaceMaximumDepth;
  int UnevenBottomSurface;
  int UseBottomSurfaceMaximumDepth;
  double BottomSurfaceMaximumDepth;
  int TestAxis;

  // Description:
  // Add Sets.  The default behaviour of this method is to call
  // AddDataFrameZFacesSets, AddDataFrameYFacesSets and
  // AddDataFrameXFacesSets.  However child classes can and should
  // override this behaviour if they require different node and element sets.
  virtual int AddSets(vtkboneFiniteElementModel* model);

  // Description:
  // Add node and element sets on particular faces.
  // These methods may be overridden by child classes if they require
  // unusual methods of selecting the nodes.
  virtual int AddDataFrameZFacesSets(vtkboneFiniteElementModel* model);
  virtual int AddDataFrameXFacesSets(vtkboneFiniteElementModel* model);
  virtual int AddDataFrameYFacesSets(vtkboneFiniteElementModel* model);

private:
  vtkboneApplyTestBase(const vtkboneApplyTestBase&); // Not implemented
  void operator=(const vtkboneApplyTestBase&); // Not implemented
};

#endif

