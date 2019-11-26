/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneVonMisesIsotropicMaterial - An object representing an
//  isotropic von Mises elastoplastic material.
//
// .SECTION Description
// vtkboneVonMisesIsotropicMaterial stores the parameters for an
// isotropic material with von Mises elastoplastic properties.
//
// .SECTION See Also
// vtkboneMaterialTable vtkboneFiniteElementModel
// vtkboneGenerateHomogeneousVonMisesIsotropicMaterialTable
// vtkboneGenerateHommingaMaterialTable
// vtkboneLinearOrthotropicMaterial
// vtkboneLinearAnistropicMaterial
// vtkboneMohrCoulombIsotropicMaterial

#ifndef __vtkboneVonMisesIsotropicMaterial_h
#define __vtkboneVonMisesIsotropicMaterial_h

#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkIdTypeArray.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneVonMisesIsotropicMaterial : public vtkboneLinearIsotropicMaterial
  {
  public:
    static vtkboneVonMisesIsotropicMaterial* New();
    vtkTypeMacro(vtkboneVonMisesIsotropicMaterial, vtkboneLinearIsotropicMaterial);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    // Description:
    // Set Yield Strength.
    vtkSetMacro(YieldStrength, double);
    vtkGetMacro(YieldStrength, double);

    // Description:
    // Creates a copy of this object.
    // You should almost certainly give the copy or the original a new name.
    virtual vtkboneMaterial* Copy() override;

    // Description:
    // Creates a copy of this object, with all the modulii scaled by
    // factor.
    // You should almost certainly give the copy or the original a new name.
    // Note that you will have to delete the pointer when finished with it.
    virtual vtkboneMaterial* ScaledCopy(double factor) override;

  protected:
    vtkboneVonMisesIsotropicMaterial();
    ~vtkboneVonMisesIsotropicMaterial();

    double YieldStrength;

  private:
    vtkboneVonMisesIsotropicMaterial(const vtkboneVonMisesIsotropicMaterial&);  // Not implemented.
    void operator=(const vtkboneVonMisesIsotropicMaterial&);  // Not implemented.
  };

#endif

