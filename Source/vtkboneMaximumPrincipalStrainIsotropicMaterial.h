/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneMaximumPrincipalStrainIsotropicMaterial
    @brief   An object representing an
  isotropic maximum principal strain elastoplastic material.


 vtkboneMaximumPrincipalStrainIsotropicMaterial stores the parameters for an
 isotropic material with maximum principal strain elastoplastic properties.

    @sa
 vtkboneMaterialTable vtkboneFiniteElementModel
 vtkboneGenerateHomogeneousMaximumPrincipalStrainIsotropicMaterialTable
 vtkboneGenerateHommingaMaterialTable
 vtkboneLinearOrthotropicMaterial
 vtkboneLinearAnistropicMaterial
 vtkboneVonMisesIsotropicMaterial
 vtkboneMohrCoulombIsotropicMaterial
*/

#ifndef __vtkboneMaximumPrincipalStrainIsotropicMaterial_h
#define __vtkboneMaximumPrincipalStrainIsotropicMaterial_h

#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkIdTypeArray.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneMaximumPrincipalStrainIsotropicMaterial : public vtkboneLinearIsotropicMaterial
{
  public:
    static vtkboneMaximumPrincipalStrainIsotropicMaterial* New();
    vtkTypeMacro(vtkboneMaximumPrincipalStrainIsotropicMaterial, vtkboneLinearIsotropicMaterial);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    //@{
    /*! Set the yield strains in tension and compression. */
    virtual void SetYieldStrains (
      double MaximumTensilePrincipalStrain,
      double MaximumCompressivePrincipalStrain);
    //@}

    vtkGetMacro(MaximumTensilePrincipalStrain, double);
    vtkGetMacro(MaximumCompressivePrincipalStrain, double);

    /*! Creates a copy of this object. You should almost certainly give the
	copy or the original a new name. */
    virtual vtkboneMaterial* Copy() override;

    /*! Creates a copy of this object, with all the modulii scaled by
	factor. You should almost certainly give the copy or the original a
	new name. Note that you will have to delete the pointer when
	finished with it. */
    virtual vtkboneMaterial* ScaledCopy(double factor) override;

  protected:
    vtkboneMaximumPrincipalStrainIsotropicMaterial();
    ~vtkboneMaximumPrincipalStrainIsotropicMaterial();

    double MaximumTensilePrincipalStrain;
    double MaximumCompressivePrincipalStrain;

  private:
    vtkboneMaximumPrincipalStrainIsotropicMaterial(const vtkboneMaximumPrincipalStrainIsotropicMaterial&);  // Not implemented.
    void operator=(const vtkboneMaximumPrincipalStrainIsotropicMaterial&);  // Not implemented.
};

#endif

