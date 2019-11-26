/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneTensorOfInertia - calculates the tensor of inertia.
//
// .SECTION Description
// Accepts either vtkImageData or vtkUnstructuredGrid as input.
//
// vtkUnstructuredGrid inputs must consists of only cells of type VTK_VOXEL.
//
// .SECTION See Also
// vtkMassProperties

#ifndef __vtkboneTensorOfInertia_h
#define __vtkboneTensorOfInertia_h

#include "vtkAlgorithm.h"
#include "vtkImageData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkboneWin32Header.h"

// forward declarations
class vtkTensor;
class vtkMatrix3x3;

class VTKBONE_EXPORT vtkboneTensorOfInertia : public vtkAlgorithm
{
public:
  // Description:
  // Constructs with initial values of zero.
  static vtkboneTensorOfInertia *New();

  vtkTypeMacro(vtkboneTensorOfInertia,vtkAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

   // Description:
  // Process a request from the executive.
  // Will be delegated to  RequestData
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);
 
  // Description:
  // Assign a data object as input.
  void SetInputData(vtkDataObject *);
  void SetInputData(int, vtkDataObject*);

  // Description:
  // Set/Get UseSpecific.  If set, will only consider cells with
  // integer scalar value equal to SpecificValue.
  vtkSetMacro(UseSpecificValue, int);
  vtkGetMacro(UseSpecificValue, int);
  vtkBooleanMacro(UseSpecificValue,int);

  // Description:
  // Set/Get the SpecificValue.  If set, automatically sets UseSpecificValue on.
  // Applies only to image inputs.
  void SetSpecificValue (int v) {this->SpecificValue=v; this->UseSpecificValue=1;}
  vtkGetMacro(SpecificValue, int);

  // Description:
  // Set/Get UseThresholds.  If set, will only consider cells with
  // scalar value between LowerThreshold and UpperThreshold (inclusive).
  // Applies only to image inputs.
  vtkSetMacro(UseThresholds, int);
  vtkGetMacro(UseThresholds, int);
  vtkBooleanMacro(UseThresholds,int);

  // Description:
  // Set/Get the LowerThreshold.  If set, automatically sets UseThresholds on.
  // Default value is VTK_DOUBLE_MIN.
  // Applies only to image inputs.
  void SetLowerThreshold (double v) {this->LowerThreshold=v; this->UseThresholds=1;}
  vtkGetMacro(LowerThreshold, double);

  // Description:
  // Set/Get the UpperThreshold.  If set, automatically sets UseThresholds on.
  // Default value is VTK_DOUBLE_MAX.
  // Applies only to image inputs.
  void SetUpperThreshold (double v) {this->UpperThreshold=v; this->UseThresholds=1;}
  vtkGetMacro(UpperThreshold, double);

  // Description:
  // Compute and return the count of cells matching the criteria.
  vtkGetMacro(Count, vtkIdType);

  // Description:
  // Compute and return the volume.
  vtkGetMacro(Volume, double);

  // Description:
  // Compute and return the mass.
  vtkGetMacro(Mass, double);

  // Description:
  // Compute and return the center of mass.
  vtkGetVector3Macro(CenterOfMass, double);
  double GetCenterOfMassX() {return this->CenterOfMass[0];}
  double GetCenterOfMassY() {return this->CenterOfMass[1];}
  double GetCenterOfMassZ() {return this->CenterOfMass[2];}

  // Description:
  // Compute and return the tensor of inertia about the center of mass.
  void GetTensorOfInertia(vtkTensor* MOI);

  // Description:
  // Compute and return the tensor of inertia about the origin.
  void GetTensorOfInertiaAboutOrigin(vtkTensor* MOI);

  // Description:
  // Get the eigenvectors of the tensor of inertia.  The principal axes
  // are in the columns of A.  THe eigenvectors are aligned as close
  // as possible to x,y,z.
  void GetEigenvectors(vtkMatrix3x3* A);

  // Description:
  // Get the principal axis lying closest to the specified Cartesian axis.
  vtkGetVector3Macro(PrincipalAxisClosestToX, double);
  vtkGetVector3Macro(PrincipalAxisClosestToY, double);
  vtkGetVector3Macro(PrincipalAxisClosestToZ, double);
  
  // Description:
  // Get the principal moments of inertia.  These are ordered the same as
  // the principal axes of inertia.
  vtkGetVector3Macro(PrincipalMoments, double);
  double GetPrincipalMoment0()
    {return this->PrincipalMoments[0];}
  double GetPrincipalMoment1()
    {return this->PrincipalMoments[1];}
  double GetPrincipalMoment2()
    {return this->PrincipalMoments[2];}

  // Description:
  // Given a tensor of inertia T0 about the center of mass of a body
  // with mass m, calculate the tensor of inertia B about the point r.
  // Note: Invariant under r -> -r, which can be handy.
  static void TranslateTensorOfInertiaFromCOM(
      vtkTensor* T0,
      double mass,
      double r[3],
      vtkTensor* T);

  // Description:
  // Given a tensor of inertia T about some point r of a body with mass m,
  // calculates the tensor of inertia B about the center of mass.
  // Note: Invariant under r -> -r, which can be handy.
  static void TranslateTensorOfInertiaToCOM(
      vtkTensor* T,
      double mass,
      double r[3],
      vtkTensor* T0);

  // Description:
  // Given a tensor of inertia T1 about some point r1 of a body with mass m,
  // calculate the tensor of inertia T2 about some other point r2.
  static void TranslateTensorOfInertia(
      vtkTensor* T1,
      double r1[3],
      double mass,
      double r2[3],
      vtkTensor* T2);

protected:
  vtkboneTensorOfInertia();
  ~vtkboneTensorOfInertia();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  virtual int ProcessImage(vtkImageData* image);

  virtual int ProcessUnstructuredGrid(vtkUnstructuredGrid* grid);

  virtual int FillInputPortInformation(int port, vtkInformation* info);

  // Settings
  int           UseSpecificValue;
  int           UseThresholds;
  int           SpecificValue;
  double        LowerThreshold;
  double        UpperThreshold;
  
  // Results
  vtkIdType     Count;
  double        Volume;
  double        Mass;
  double        CenterOfMass[3];
  vtkTensor*    TensorOfInertiaAboutOrigin;
  vtkTensor*    TensorOfInertia;
  vtkMatrix3x3* Eigenvectors;
  double        PrincipalMoments[3];
  double        PrincipalAxisClosestToX[3];
  double        PrincipalAxisClosestToY[3];
  double        PrincipalAxisClosestToZ[3];

private:
  vtkboneTensorOfInertia(const vtkboneTensorOfInertia&);  // Not implemented.
  void operator=(const vtkboneTensorOfInertia&);  // Not implemented.
};

#endif
