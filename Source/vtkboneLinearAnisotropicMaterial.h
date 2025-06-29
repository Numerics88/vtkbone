/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneLinearAnisotropicMaterial
    @brief   An object representing a
 linear anisotropic material.


 vtkboneLinearAnisotropicMaterial stores the parameters for a
 linear anisotropic material.

    @sa
 vtkboneMaterialTable vtkboneFiniteElementModel
 vtkboneGenerateHomogeneousLinearIsotropicMaterialTable
 vtkboneGenerateHommingaMaterialTable
 vtkboneLinearIsotropicMaterial
 vtkboneLinearOrthotropicMaterial
*/

#ifndef __vtkboneLinearAnisotropicMaterial_h
#define __vtkboneLinearAnisotropicMaterial_h

#include "vtkboneMaterial.h"
#include "vtkIdTypeArray.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkDataArray;

class VTKBONE_EXPORT vtkboneLinearAnisotropicMaterial : public vtkboneMaterial
{
  public:
    static vtkboneLinearAnisotropicMaterial* New();
    vtkTypeMacro(vtkboneLinearAnisotropicMaterial, vtkboneMaterial);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    /*! Sets the stress-strain matrix. Arguement should be 36 values,
	corresponding to 6x6 symmetric matrix. */
    void SetStressStrainMatrix (const double* k);

    /*! Sets the stress-strain matrix. Arguement should be 36 values,
	corresponding to 6x6 symmetric matrix. */
    void SetStressStrainMatrix (vtkDataArray* k);

    /*! Gets a pointer to the stress-strain matrix, which is 36 values,
	Copies the stress-strain matrix to k as 36 values, corresponding to
	6x6 symmetric matrix. */
    double* GetStressStrainMatrix() {return this->StressStrainMatrix; };

    /*! Copies the stress-strain matrix to k as 36 values, corresponding to
	6x6 symmetric matrix. k must be allocated beforehand. */
    void GetStressStrainMatrix (double* k);

    /*! Copies the stress-strain matrix to k as 36 values, corresponding to
	6x6 symmetric matrix. k will be resized to 6x6. */
    void GetStressStrainMatrix (vtkDataArray* k);

    /*! Creates a copy of this object. You should almost certainly give the
	copy or the original a new name. */
    virtual vtkboneMaterial* Copy() override;

    /*! Creates a copy of this object, with all the modulii scaled by
	factor. You should almost certainly give the copy or the original a
	new name. Note that you will have to delete the pointer when
	finished with it. */
    virtual vtkboneMaterial* ScaledCopy(double factor) override;

  protected:
    vtkboneLinearAnisotropicMaterial();
    ~vtkboneLinearAnisotropicMaterial();

    double StressStrainMatrix[6*6];

  private:
    vtkboneLinearAnisotropicMaterial(const vtkboneLinearAnisotropicMaterial&);  // Not implemented.
    void operator=(const vtkboneLinearAnisotropicMaterial&);  // Not implemented.
};

#endif

