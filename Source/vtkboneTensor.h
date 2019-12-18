/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkboneTensor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkboneTensor - supporting class to enable assignment and referencing of tensors
// .SECTION Description
// vtkboneTensor is a floating point representation of an nxn tensor. vtkboneTensor
// provides methods for assignment and reference of tensor components. It
// does it in such a way as to minimize data copying.
//
// .SECTION Caveats
// vtkboneTensor performs its operations using pointer reference. You are
// responsible for supplying data storage (if necessary) if local copies
// of data are being made.

#ifndef vtkboneTensor_h
#define vtkboneTensor_h

#include "vtkObject.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneTensor : public vtkObject
{
public:
  static vtkboneTensor *New();
  vtkTypeMacro(vtkboneTensor,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Initialize tensor components to 0.0.
  void Initialize();

  // Description:
  // Get the tensor component (i,j).
  double GetComponent(int i, int j) {return this->T[i+3*j];};

  // Description:
  // Set the value of the tensor component (i,j).
  void SetComponent(int i, int j, double v) {if (i > 2 || j > 2) {vtkErrorMacro("trying to set tensor component i or j > 2: i = " << i << ", j = " << j); return;}; this->T[i+3*j] = v;};

  // Description:
  // Add to the value of the tensor component at location (i,j).
  void AddComponent(int i, int j, double v) { if (i > 2 || j > 2) {vtkErrorMacro("trying to add tensor component i or j > 2: i = " << i << ", j = " << j); return;}; this->T[i+3*j] += v;};

  // Description:
  // Return column vector from tensor. (Assumes 2D matrix form and 0-offset.)
  double *GetColumn(int j) { if (j > 2) {vtkErrorMacro("trying to get tensor column j > 2: j = " << j); return NULL;}; return this->T + 3*j;};

  // Description:
  // Deep copy of one tensor to another tensor.
  void DeepCopy(vtkboneTensor *t);

  // Description:
  // Provide double * type conversion.
  operator double*() {return this->T;};

  // Description:
  // Data member left public for efficiency.
  double *T;

protected:
  vtkboneTensor();
  ~vtkboneTensor() {}

  double Storage[9];
private:
  vtkboneTensor(const vtkboneTensor&);  // Not implemented.
  void operator=(const vtkboneTensor&);  // Not implemented.
};

//----------------------------------------------------------------------------
inline void vtkboneTensor::Initialize()
{
  for (int j=0; j<3; j++)
    {
    for (int i=0; i<3; i++)
      {
      this->T[i+j*3] = 0.0;
      }
    }
}

//----------------------------------------------------------------------------
inline void vtkboneTensor::DeepCopy(vtkboneTensor *t)
{
  for (int j=0; j < 3; j++)
    {
    for (int i=0; i < 3; i++)
      {
      this->T[i+3*j] = t->T[i+3*j];
      }
    }
}

#endif
