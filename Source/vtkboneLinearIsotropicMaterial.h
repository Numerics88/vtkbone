/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneLinearIsotropicMaterial
    @brief   An object representing a
 linear isotropic material.


 vtkboneLinearIsotropicMaterial stores the parameters for a
 linear isotropic material.

    @sa
 vtkboneMaterialTable vtkboneFiniteElementModel
 vtkboneGenerateHomogeneousLinearIsotropicMaterialTable
 vtkboneGenerateHommingaMaterialTable
 vtkboneLinearOrthotropicMaterial
 vtkboneLinearAnistropicMaterial
*/

#ifndef __vtkboneLinearIsotropicMaterial_h
#define __vtkboneLinearIsotropicMaterial_h

#include "vtkboneMaterial.h"
#include "vtkIdTypeArray.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneLinearIsotropicMaterial : public vtkboneMaterial
{
  public:
    static vtkboneLinearIsotropicMaterial* New();
    vtkTypeMacro(vtkboneLinearIsotropicMaterial, vtkboneMaterial);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    //@{
    /*! Set the Young's modulus (E). Assuming that your length units are
	mm, then modulus units are MPa. Default is 6829 MPa. */
    vtkSetMacro(YoungsModulus, double);
    vtkGetMacro(YoungsModulus, double);
    //@}

    //@{
    /*! Set the isotropic Poisson's ratio (nu). Default is 0.3. */
    vtkSetMacro(PoissonsRatio, double);
    vtkGetMacro(PoissonsRatio, double);
    //@}

    /*! Get the isotropic shear modulus (G).  Note that for the isotropic
	case, this is given by G = E/(2*(1 + nu)) */
    double GetShearModulus();

    /*! Creates a copy of this object. You should almost certainly give the
	copy or the original a new name. */
    virtual vtkboneMaterial* Copy() override;

    /*! Creates a copy of this object, with all the modulii scaled by
	factor. You should almost certainly give the copy or the original a
	new name. Note that you will have to delete the pointer when
	finished with it. */
    virtual vtkboneMaterial* ScaledCopy(double factor) override;

  protected:
    vtkboneLinearIsotropicMaterial();
    ~vtkboneLinearIsotropicMaterial();

    double YoungsModulus;
    double PoissonsRatio;

  private:
    vtkboneLinearIsotropicMaterial(const vtkboneLinearIsotropicMaterial&);  // Not implemented.
    void operator=(const vtkboneLinearIsotropicMaterial&);  // Not implemented.
};

#endif

