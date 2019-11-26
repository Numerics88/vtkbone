/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneGenerateHommingaMaterialTable - generates a
// material table corresponding to the Homminga material model.
//
// .SECTION Description
// This class generates a
// vtkboneMaterialTable table that consists of a sequence
// of material definitions with modulii according to the
// following formula:
// @verbatim
//   E = E_max * (i-i_first/(i_last - i_first + 1))^exponent
// @endverbatim
//
// This material model is introduced in the following publication:
//    Homminga et al. (2001) J Biomech 34(4):513-517.
//
// A material table is generated with a single entry which is
// respectively one of
//    vtkboneLinearIsotropicMaterialArray
//    vtkboneLinearOrthotropicMaterialArray
//    vtkboneAnisotropicMaterialArray
// depending on what input material is used.
// This material array will have material index equal to FirstIndex, and
// length LastIndex - FirstIndex + 1.
//
// .SECTION See Also
// vtkboneMaterialTable vtkboneMaterial
// vtkboneGenerateHomogeneousLinearIsotropicMaterialTable
// vtkboneLinearIsotropicMaterialArray
// vtkboneLinearOrthotropicMaterialArray
// vtkboneAnisotropicMaterialArray

#ifndef __vtkboneGenerateHommingaMaterialTable_h
#define __vtkboneGenerateHommingaMaterialTable_h

#include "vtkboneMaterialTableAlgorithm.h"
#include "vtkboneMaterial.h"
#include "vtkboneWin32Header.h"


class VTKBONE_EXPORT vtkboneGenerateHommingaMaterialTable : public vtkboneMaterialTableAlgorithm
{
public:
  static vtkboneGenerateHommingaMaterialTable *New();
  vtkTypeMacro(vtkboneGenerateHommingaMaterialTable, vtkboneMaterialTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set/Get the material ID equal for the first material definition in the sequence.
  // Default 1.
  vtkSetMacro(FirstIndex, int);
  vtkGetMacro(FirstIndex, int);

  // Description:
  // Set/Get the material ID equal for the first material definition in the sequence.
  // Note that the material table has in fact only one entry, which is a material
  // array at material ID FirstIndex.
  // Default 127.
  vtkSetMacro(LastIndex, int);
  vtkGetMacro(LastIndex, int);

  // Description:
  // Set/Get exponent.
  // Default 1.7 .
  vtkSetMacro(Exponent, double);
  vtkGetMacro(Exponent, double);

  // Description:
  // Set the material that is to represent "Full Scale" or maximum
  // density/strength.  This object can be any kind of derived class of
  // vtkboneMaterial, but not a vtkboneMaterial object itself, which is
  // intended to be only a base class.
  // The default is an instance of vtkboneLinearIsotropicMaterial constructed
  // with its default parameters.
  vtkSetObjectMacro(FullScaleMaterial, vtkboneMaterial);
  vtkGetObjectMacro(FullScaleMaterial, vtkboneMaterial);

protected:
  vtkboneGenerateHommingaMaterialTable();
  ~vtkboneGenerateHommingaMaterialTable();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  int FirstIndex;
  int LastIndex;
  double Exponent;
  vtkboneMaterial* FullScaleMaterial;

private:
  vtkboneGenerateHommingaMaterialTable(const vtkboneGenerateHommingaMaterialTable&); // Not implemented
  void operator=(const vtkboneGenerateHommingaMaterialTable&); // Not implemented
};

#endif

