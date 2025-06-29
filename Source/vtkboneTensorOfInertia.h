/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneTensorOfInertia
    @brief   calculates the tensor of inertia.


 Accepts either vtkImageData or vtkUnstructuredGrid as input.

 vtkUnstructuredGrid inputs must consists of only cells of type VTK_VOXEL.

    @sa
 vtkMassProperties
*/

#ifndef __vtkboneTensorOfInertia_h
#define __vtkboneTensorOfInertia_h

#include "vtkAlgorithm.h"
#include "vtkImageData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkboneWin32Header.h"
#include "vtkboneTensor.h"

// forward declarations
class vtkMatrix3x3;

class VTKBONE_EXPORT vtkboneTensorOfInertia : public vtkAlgorithm
{
public:
  /*! Constructs with initial values of zero. */
  static vtkboneTensorOfInertia *New();

  vtkTypeMacro(vtkboneTensorOfInertia,vtkAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

   //@{
   /*! Process a request from the executive. Will be delegated to 
       RequestData */
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*) override;
   //@}

  //@{
  /*! Assign a data object as input. */
  void SetInputData(vtkDataObject *);
  void SetInputData(int, vtkDataObject*);
  //@}

  //@{
  /*! Set/Get UseSpecific.  If set, will only consider cells with integer
      scalar value equal to SpecificValue. */
  vtkSetMacro(UseSpecificValue, int);
  vtkGetMacro(UseSpecificValue, int);
  vtkBooleanMacro(UseSpecificValue,int);
  //@}

  //@{
  /*! Set/Get the SpecificValue.  If set, automatically sets
      UseSpecificValue on. Applies only to image inputs. */
  void SetSpecificValue (int v) {this->SpecificValue=v; this->UseSpecificValue=1;}
  vtkGetMacro(SpecificValue, int);
  //@}

  //@{
  /*! Set/Get UseThresholds.  If set, will only consider cells with scalar
      value between LowerThreshold and UpperThreshold (inclusive). Applies
      only to image inputs. */
  vtkSetMacro(UseThresholds, int);
  vtkGetMacro(UseThresholds, int);
  vtkBooleanMacro(UseThresholds,int);
  //@}

  //@{
  /*! Set/Get the LowerThreshold.  If set, automatically sets UseThresholds
      on. Default value is VTK_DOUBLE_MIN. Applies only to image inputs. */
  void SetLowerThreshold (double v) {this->LowerThreshold=v; this->UseThresholds=1;}
  vtkGetMacro(LowerThreshold, double);
  //@}

  //@{
  /*! Set/Get the UpperThreshold.  If set, automatically sets UseThresholds
      on. Default value is VTK_DOUBLE_MAX. Applies only to image inputs. */
  void SetUpperThreshold (double v) {this->UpperThreshold=v; this->UseThresholds=1;}
  vtkGetMacro(UpperThreshold, double);
  //@}

  //@{
  /*! Compute and return the count of cells matching the criteria. */
  vtkGetMacro(Count, vtkIdType);
  //@}

  //@{
  /*! Compute and return the volume. */
  vtkGetMacro(Volume, double);
  //@}

  //@{
  /*! Compute and return the mass. */
  vtkGetMacro(Mass, double);
  //@}

  //@{
  /*! Compute and return the center of mass. */
  vtkGetVector3Macro(CenterOfMass, double);
  double GetCenterOfMassX() {return this->CenterOfMass[0];}
  double GetCenterOfMassY() {return this->CenterOfMass[1];}
  double GetCenterOfMassZ() {return this->CenterOfMass[2];}
  //@}

  /*! Compute and return the tensor of inertia about the center of mass. */
  void GetTensorOfInertia(vtkboneTensor* MOI);

  /*! Compute and return the tensor of inertia about the origin. */
  void GetTensorOfInertiaAboutOrigin(vtkboneTensor* MOI);

  /*! Get the eigenvectors of the tensor of inertia.  The principal axes
      are in the columns of A.	THe eigenvectors are aligned as close as
      possible to x,y,z. */
  void GetEigenvectors(vtkMatrix3x3* A);

  //@{
  /*! Get the principal axis lying closest to the specified Cartesian axis. */
  vtkGetVector3Macro(PrincipalAxisClosestToX, double);
  vtkGetVector3Macro(PrincipalAxisClosestToY, double);
  vtkGetVector3Macro(PrincipalAxisClosestToZ, double);
  //@}

  //@{
  /*! Get the principal moments of inertia.  These are ordered the same as
      the principal axes of inertia. */
  vtkGetVector3Macro(PrincipalMoments, double);
  double GetPrincipalMoment0()
    {return this->PrincipalMoments[0];}
  double GetPrincipalMoment1()
    {return this->PrincipalMoments[1];}
  double GetPrincipalMoment2()
    {return this->PrincipalMoments[2];}
  //@}

  //@{
  /*! Given a tensor of inertia T0 about the center of mass of a body with
      mass m, calculate the tensor of inertia B about the point r. Note:
      Invariant under r -> -r, which can be handy. */
  static void TranslateTensorOfInertiaFromCOM(
      vtkboneTensor* T0,
      double mass,
      double r[3],
      vtkboneTensor* T);
  //@}

  //@{
  /*! Given a tensor of inertia T about some point r of a body with mass m,
      calculates the tensor of inertia B about the center of mass. Note:
      Invariant under r -> -r, which can be handy. */
  static void TranslateTensorOfInertiaToCOM(
      vtkboneTensor* T,
      double mass,
      double r[3],
      vtkboneTensor* T0);
  //@}

  //@{
  /*! Given a tensor of inertia T1 about some point r1 of a body with mass
      m, calculate the tensor of inertia T2 about some other point r2. */
  static void TranslateTensorOfInertia(
      vtkboneTensor* T1,
      double r1[3],
      double mass,
      double r2[3],
      vtkboneTensor* T2);
  //@}

protected:
  vtkboneTensorOfInertia();
  ~vtkboneTensorOfInertia();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  virtual int ProcessImage(vtkImageData* image);

  virtual int ProcessUnstructuredGrid(vtkUnstructuredGrid* grid);

  virtual int FillInputPortInformation(int port, vtkInformation* info) override;

  // Settings
  int           UseSpecificValue;
  int           UseThresholds;
  int           SpecificValue;
  double        LowerThreshold;
  double        UpperThreshold;

  // Results
  vtkIdType     	Count;
  double        	Volume;
  double        	Mass;
  double        	CenterOfMass[3];
  vtkboneTensor*  TensorOfInertiaAboutOrigin;
  vtkboneTensor*  TensorOfInertia;
  vtkMatrix3x3* 	Eigenvectors;
  double        	PrincipalMoments[3];
  double        	PrincipalAxisClosestToX[3];
  double        PrincipalAxisClosestToY[3];
  double        PrincipalAxisClosestToZ[3];

private:
  vtkboneTensorOfInertia(const vtkboneTensorOfInertia&);  // Not implemented.
  void operator=(const vtkboneTensorOfInertia&);  // Not implemented.
};

#endif
