/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneStressStrainMatrix - Some calculation functions for stress-strain matrices.
//
// .SECTION Description
// Functions to generate the stress-strain matrix for isotropic and orthotropic parameters.
// Also functions to convert to and from upper triangular packed form for the 6x6 stress-strain
// matrix.
//
// The upper triangular packed form of a 6x6 symmetric matrix is:
//   K11, K12, K22, K13, K23, K33, K14, K24, K34, K44, K15, K25, K35, K45, K55, K16, K26, K36, K46, K56, K66
// Due to symmetry, this is equivalent to
//   K11, K21, K22, K31, K32, K33, K41, K42, K43, K44, K51, K52, K53, K54, K55, K61, K62, K63, K64, K65, K66     
//
// .SECTION See Also
// vtkboneLinearIsotropicMaterial
// vtkboneLinearOrthotropicMaterial
// vtkboneLinearAniotropicMaterial

#ifndef __vtkboneStressStrainMatrix_h
#define __vtkboneStressStrainMatrix_h

#include "vtkObject.h"
#include "vtkDataArray.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneStressStrainMatrix : public vtkObject
{
public:
  static vtkboneStressStrainMatrix *New();
  vtkTypeMacro(vtkboneStressStrainMatrix,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the stress-strain matrix using isotropic parameters.
  void SetIsotropic(double E, double nu);

  // Description:
  // Set the stress-strain matrix using orthotropic parameters.
  void SetOrthotropic(double Exx,
                      double Eyy,
                      double Ezz,
                      double nuyz,
                      double nuzx,
                      double nuxy,
                      double Gyz,
                      double Gzx,
                      double Gxy);
  void SetOrthotropic(const double E[3], const double nu[3], const double G[3]);
  void SetOrthotropic(const float E[3], const float nu[3], const float G[3]);

  // Description:
  // Set the stress-strain matrix.
  void SetStressStrainMatrix (const double* D);
  void SetStressStrainMatrix (const float* D);
  void SetStressStrainMatrix (vtkDataArray* D);

  // Description:
  // Set the stress-strain matrix using the 21 upper triangular packed values.
  void SetUpperTriangularPacked (const double* UT);
  void SetUpperTriangularPacked (const float* UT);
  void SetUpperTriangularPacked (vtkDataArray* UT);

  // Description:
  // Get the stress-strain matrix.
  void GetStressStrainMatrix (double* D);
  void GetStressStrainMatrix (float* D);
  void GetStressStrainMatrix (vtkDataArray* D);

  // Description:
  // Get the 21 upper triangular packed values of the stress-strain matrix.
  void GetUpperTriangularPacked (double* UT);
  void GetUpperTriangularPacked (float* UT);
  void GetUpperTriangularPacked (vtkDataArray* UT);

protected:

  vtkboneStressStrainMatrix() {}
  ~vtkboneStressStrainMatrix() {}

  //BTX
  double StressStrainMatrix[6][6];
  //ETX

private:
  vtkboneStressStrainMatrix(const vtkboneStressStrainMatrix&);  // Not implemented.
  void operator=(const vtkboneStressStrainMatrix&);  // Not implemented.
};

#endif
