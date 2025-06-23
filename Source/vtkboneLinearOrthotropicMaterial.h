/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneLinearOrthotropicMaterial
    @brief   An object representing a
 linear orthotropic material.


 vtkboneLinearOrthotropicMaterial stores the parameters for a
 linear orthotropic material.

    @sa
 vtkboneMaterialTable vtkboneFiniteElementModel
 vtkboneGenerateHomogeneousLinearIsotropicMaterialTable
 vtkboneGenerateHommingaMaterialTable
 vtkboneLinearIsotropicMaterial
 vtkboneLinearAnistropicMaterial
*/

#ifndef __vtkboneLinearOrthotropicMaterial_h
#define __vtkboneLinearOrthotropicMaterial_h

#include "vtkboneMaterial.h"
#include "vtkIdTypeArray.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneLinearOrthotropicMaterial : public vtkboneMaterial
{
  public:
    static vtkboneLinearOrthotropicMaterial* New();
    vtkTypeMacro(vtkboneLinearOrthotropicMaterial, vtkboneMaterial);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    //@{
    /*! Set/get the Young's modulus (Ex, Ey, Ez). Assuming that your length
	units are mm, then modulus units are MPa. Default is 6829 MPa. */
    vtkSetMacro(YoungsModulusX, double);
    vtkGetMacro(YoungsModulusX, double);
    vtkSetMacro(YoungsModulusY, double);
    vtkGetMacro(YoungsModulusY, double);
    vtkSetMacro(YoungsModulusZ, double);
    vtkGetMacro(YoungsModulusZ, double);
    //@}

    //@{
    /*! Set/get the orthotropic Poisson's ratio (nu_yz, nu_zx, nu_xy).
	Default is 0.3. */
    vtkSetMacro(PoissonsRatioYZ, double);
    vtkGetMacro(PoissonsRatioYZ, double);
    vtkSetMacro(PoissonsRatioZX, double);
    vtkGetMacro(PoissonsRatioZX, double);
    vtkSetMacro(PoissonsRatioXY, double);
    vtkGetMacro(PoissonsRatioXY, double);
    //@}

    //@{
    /*! Set derived orthotropic Poisson's ratio. Note that nu_ij/Ei =
	n_ji/Ej . */
    double GetPoissonsRatioZY();
    double GetPoissonsRatioXZ();
    double GetPoissonsRatioYX();
    //@}

    //@{
    /*! Get/get the orthotropic shear modulus (G_yz, G_zx, G_xy). Default
	is 2626.5 MPa. */
    vtkSetMacro(ShearModulusYZ, double);
    vtkGetMacro(ShearModulusYZ, double);
    vtkSetMacro(ShearModulusZX, double);
    vtkGetMacro(ShearModulusZX, double);
    vtkSetMacro(ShearModulusXY, double);
    vtkGetMacro(ShearModulusXY, double);
    //@}

    //@{
    /*! Set derived orthotropic shear modulus. Note that G_ij = G_ji . */
    double GetShearModulusZY() {return ShearModulusYZ;}
    double GetShearModulusXZ() {return ShearModulusZX;}
    double GetShearModulusYX() {return ShearModulusXY;}
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
    vtkboneLinearOrthotropicMaterial();
    ~vtkboneLinearOrthotropicMaterial();

    double YoungsModulusX;
    double YoungsModulusY;
    double YoungsModulusZ;
    double PoissonsRatioYZ;
    double PoissonsRatioZX;
    double PoissonsRatioXY;
    double ShearModulusYZ;
    double ShearModulusZX;
    double ShearModulusXY;

  private:
    vtkboneLinearOrthotropicMaterial(const vtkboneLinearOrthotropicMaterial&);  // Not implemented.
    void operator=(const vtkboneLinearOrthotropicMaterial&);  // Not implemented.
};

#endif

