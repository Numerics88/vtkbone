/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneLinearOrthotropicMaterialArray
    @brief   An object representing an
 array of linear orthotropic materials.


 vtkboneLinearOrthotropicMaterialArray stores the parameters for an
 array of linear isotropic materials.
*/

#ifndef __vtkboneLinearOrthotropicMaterialArray_h
#define __vtkboneLinearOrthotropicMaterialArray_h

#include "vtkboneLinearOrthotropicMaterial.h"
#include "vtkboneMaterialArray.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkFloatArray;

class VTKBONE_EXPORT vtkboneLinearOrthotropicMaterialArray : public vtkboneMaterialArray
{
  public:
    static vtkboneLinearOrthotropicMaterialArray* New();
    vtkTypeMacro(vtkboneLinearOrthotropicMaterialArray, vtkboneMaterialArray);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    /*! Destructively re-sizes the arrays. */
    virtual void Resize(vtkIdType size) override;

    /*! Get the size of the material array (the number of materials). */
    virtual vtkIdType GetSize() override;

    //@{
    /*! Get the array for Young's modulus (E). Note that the number of
	components is be three, equivalent to E_yz, E_zx, E_xy. Assuming
	that your length units are mm, then modulus units are MPa. */
    vtkGetObjectMacro(YoungsModulus, vtkFloatArray);
    //@}

    /*! Set the array for Young's modulus (E). Note that the number of
	components must be three, equivalent to E_yz, E_zx, E_xy. Assuming
	that your length units are mm, then modulus units are MPa. */
    virtual void SetYoungsModulus(vtkFloatArray* E);

    //@{
    /*! Get the array for the isotropic Poisson's ratio (nu). Note that the
	number of components is be three, equivalent to nu_yz, nu_zx,
	nu_xy. */
    vtkGetObjectMacro(PoissonsRatio, vtkFloatArray);
    //@}

    /*! Set the array for the isotropic Poisson's ratio (nu). Note that the
	number of components must be three, equivalent to nu_yz, nu_zx,
	nu_xy. */
    virtual void SetPoissonsRatio (vtkFloatArray* nu);

    //@{
    /*! Get the array for shear modulus (E). Note that the number of
	components is be three, equivalent to G_yz, G_zx, G_xy. Assuming
	that your length units are mm, then modulus units are MPa. */
    vtkGetObjectMacro(ShearModulus, vtkFloatArray);
    //@}

    /*! Set the array for shear modulus (G). Note that the number of
	components must be three, equivalent to G_yz, G_zx, G_xy. Assuming
	that your length units are mm, then modulus units are MPa. */
    virtual void SetShearModulus(vtkFloatArray* G);

    //@{
    /*! Sets the values for index k to correspond to the specified
	individual material. Material array must already have been
	constructed (for example with Resize). */
    virtual void SetItem (vtkIdType k,
                          vtkboneLinearOrthotropicMaterial* material);
    //@}

    //@{
    /*! Sets the values for index k to correspond to the specified
	individual material, with modulii scaled by factor. Material array
	must already have been constructed (for example with Resize). */
    virtual void SetScaledItem (vtkIdType k,
                                vtkboneLinearOrthotropicMaterial* material,
                                double factor);
    //@}

    /*! Creates a copy of this object. You should almost certainly give the
	copy or the original a new name. */
    virtual vtkboneMaterial* Copy() override;

    /*! Creates a copy of this object, with all the modulii scaled by
	factor. You should almost certainly give the copy or the original a
	new name. Note that you will have to delete the pointer when
	finished with it. */
    virtual vtkboneMaterial* ScaledCopy(double factor) override;

  protected:
    vtkboneLinearOrthotropicMaterialArray();
    ~vtkboneLinearOrthotropicMaterialArray();

    vtkFloatArray* YoungsModulus;
    vtkFloatArray* PoissonsRatio;
    vtkFloatArray* ShearModulus;

  private:
    vtkboneLinearOrthotropicMaterialArray(const vtkboneLinearOrthotropicMaterialArray&);  // Not implemented.
    void operator=(const vtkboneLinearOrthotropicMaterialArray&);  // Not implemented.
};

#endif

