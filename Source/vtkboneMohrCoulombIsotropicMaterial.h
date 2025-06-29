/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneVonMisesIsotropicMaterial
    @brief   An object representing an
  isotropic Mohr Coulomb elastoplastic material.


 vtkboneMohrCoulombIsotropicMaterial stores the parameters for an
 isotropic material with Mohr Coulomb elastoplastic properties.

    @sa
 vtkboneMaterialTable vtkboneFiniteElementModel
 vtkboneGenerateHomogeneousMohrCoulombIsotropicMaterialTable
 vtkboneGenerateHommingaMaterialTable
 vtkboneLinearOrthotropicMaterial
 vtkboneLinearAnistropicMaterial
 vtkboneVonMisesIsotropicMaterial
*/

#ifndef __vtkboneMohrCoulombIsotropicMaterial_h
#define __vtkboneMohrCoulombIsotropicMaterial_h

#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkIdTypeArray.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneMohrCoulombIsotropicMaterial : public vtkboneLinearIsotropicMaterial
{
  public:
    static vtkboneMohrCoulombIsotropicMaterial* New();
    vtkTypeMacro(vtkboneMohrCoulombIsotropicMaterial, vtkboneLinearIsotropicMaterial);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    //@{
    /*! Set/get c. */
    vtkSetMacro(C, double);
    vtkGetMacro(C, double);
    //@}

    //@{
    /*! Set/get phi. */
    vtkSetMacro(Phi, double);
    vtkGetMacro(Phi, double);
    //@}

    /*! Set the yield strengths in tension and compression. Note that the
	yield strength has units of stress. This is an alternate way to
	define a Mohr Coulomb material, and will determine C and Phi. */
    virtual void SetYieldStrengths (double YT, double YC);

    /*! Creates a copy of this object. You should almost certainly give the
	copy or the original a new name. */
    virtual vtkboneMaterial* Copy() override;

    /*! Creates a copy of this object, with all the modulii scaled by
	factor. You should almost certainly give the copy or the original a
	new name. Note that you will have to delete the pointer when
	finished with it. */
    virtual vtkboneMaterial* ScaledCopy(double factor) override;

  protected:
    vtkboneMohrCoulombIsotropicMaterial();
    ~vtkboneMohrCoulombIsotropicMaterial();

    double C;
    double Phi;

  private:
    vtkboneMohrCoulombIsotropicMaterial(const vtkboneMohrCoulombIsotropicMaterial&);  // Not implemented.
    void operator=(const vtkboneMohrCoulombIsotropicMaterial&);  // Not implemented.
};

#endif

