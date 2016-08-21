/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneLinearAnisotropicMaterialArray - An object representing an
// array of linear anisotropic materials.
//
// .SECTION Description
// vtkboneLinearAnisotropicMaterialArray stores the parameters for an
// array of linear anisotropic materials.

#ifndef __vtkboneLinearAnisotropicMaterialArray_h
#define __vtkboneLinearAnisotropicMaterialArray_h

#include "vtkboneLinearAnisotropicMaterial.h"
#include "vtkboneMaterialArray.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkFloatArray;

class VTKBONE_EXPORT vtkboneLinearAnisotropicMaterialArray : public vtkboneMaterialArray
  {
  public:
    static vtkboneLinearAnisotropicMaterialArray* New();
    vtkTypeMacro(vtkboneLinearAnisotropicMaterialArray, vtkboneMaterialArray);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Destructively re-sizes the arrays.
    virtual void Resize(vtkIdType size);

    // Description:
    // Get the size of the material array (the number of materials).
    virtual vtkIdType GetSize();

    // Description:
    // Set the array for stress-strain matrices.
    // Assuming that your length units are mm, then modulus units are MPa.
    // The stress-strain matrices are in upper triangular format. Thus the
    // number of components must be set to 21. Each matrix is stored as:
    // K11, K12... K16, K22, K23... K26...
    vtkGetObjectMacro(StressStrainMatrixUpperTriangular, vtkFloatArray);

    // Description:
    // Set the array for stress-strain matrices.
    // Assuming that your length units are mm, then modulus units are MPa.
    // The stress-strain matrices are in upper triangular format. Thus the
    // number of components must be set to 21. Each matrix is stored as:
    // K11, K12... K16, K22, K23... K26...
    virtual void SetStressStrainMatrixUpperTriangular(vtkFloatArray* K);

    // Description
    // Sets the values for index k to correspond to the
    // specified individual material.
    // Material array must already have been constructed (for example with Resize).
    virtual void SetItem (vtkIdType k,
                          vtkboneLinearAnisotropicMaterial* material);

    // Description
    // Sets the values for index k to correspond to the
    // specified individual material, with modulii scaled by factor.
    // Material array must already have been constructed (for example with Resize).
    virtual void SetScaledItem (vtkIdType k,
                                vtkboneLinearAnisotropicMaterial* material,
                                double factor);

    // Description
    // Sets the values for index k to correspond to the
    // specified individual material, with modulii scaled by factor.
    virtual void SetScaledItemUpperTriangular (vtkIdType k,
                                               float* ut,
                                               double factor);
    virtual void SetScaledItemUpperTriangular (vtkIdType k,
                                               double* ut,
                                               double factor);

    float* GetItemUpperTriangular (vtkIdType k);

    // Description:
    // Creates a copy of this object.
    // You should almost certainly give the copy or the original a new name.
    virtual vtkboneMaterial* Copy();

    // Description:
    // Creates a copy of this object, with all the modulii scaled by
    // factor.
    // You should almost certainly give the copy or the original a new name.
    // Note that you will have to delete the pointer when finished with it.
    virtual vtkboneMaterial* ScaledCopy(double factor);

  protected:
    vtkboneLinearAnisotropicMaterialArray();
    ~vtkboneLinearAnisotropicMaterialArray();

    vtkFloatArray* StressStrainMatrixUpperTriangular;

  private:
    vtkboneLinearAnisotropicMaterialArray(const vtkboneLinearAnisotropicMaterialArray&);  // Not implemented.
    void operator=(const vtkboneLinearAnisotropicMaterialArray&);  // Not implemented.
  };

#endif

