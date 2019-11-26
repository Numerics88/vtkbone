/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneGenerateHomogeneousMaterialTable - generates a
// homogeneous material table.
//
// .SECTION Description
// vtkboneGenerateHomogeneousMaterialTable generates a
// vtkboneMaterialTable table that consists of a specified number of
// identical vtkboneMaterial objects.  The vtkboneMaterial object can be
// any kind of derived class of vtkboneMaterial, so that this class
// supports for example the generation of both homogeneous linear isotropic
// and homogeneous linear orthotropic material tables.
//
// .SECTION See Also
// vtkboneMaterialTable vtkboneMaterial
// vtkboneLinearIsotropicMaterial vtkboneLinearOrthotropicMaterial
// vtkboneGenerateHomogeneousLinearOrthotropicMaterialTable
// vtkboneGenerateHommingaMaterialTable

#ifndef __vtkboneGenerateHomogeneousMaterialTable_h
#define __vtkboneGenerateHomogeneousMaterialTable_h

#include "vtkboneMaterialTableAlgorithm.h"
#include "vtkboneMaterial.h"
#include "vtkboneWin32Header.h"


class VTKBONE_EXPORT vtkboneGenerateHomogeneousMaterialTable : public vtkboneMaterialTableAlgorithm
{
public:
  static vtkboneGenerateHomogeneousMaterialTable *New();
  vtkTypeMacro(vtkboneGenerateHomogeneousMaterialTable, vtkboneMaterialTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set the material. This object can be any kind of derived class of
  // vtkboneMaterial, but not a vtkboneMaterial object itself, which is
  // intended to be only a base class.
  // The default is an instance of vtkboneLinearIsotropicMaterial constructed
  // with its default parameters.
  vtkSetObjectMacro(Material, vtkboneMaterial);
  vtkGetObjectMacro(Material, vtkboneMaterial);

  // Description:
  // Set/Get the first index to use in the table.
  // Default 1.
  vtkSetMacro(FirstIndex, int);
  vtkGetMacro(FirstIndex, int);

  // Description:
  // Set/Get the last index to use in the table.
  // Default 127.
  vtkSetMacro(LastIndex, int);
  vtkGetMacro(LastIndex, int);

  // Description:
  // Pass a list of Ids to construct table entries for.  If set, then FirstIndex
  // and LastIndex are ignored.
  // Default is NULL.
  vtkSetObjectMacro(MaterialIdList, vtkDataArray);
  vtkGetObjectMacro(MaterialIdList, vtkDataArray);

protected:
  vtkboneGenerateHomogeneousMaterialTable();
  ~vtkboneGenerateHomogeneousMaterialTable();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  int FirstIndex;
  int LastIndex;
  vtkboneMaterial* Material;
  vtkDataArray* MaterialIdList;

private:
  vtkboneGenerateHomogeneousMaterialTable(const vtkboneGenerateHomogeneousMaterialTable&); // Not implemented
  void operator=(const vtkboneGenerateHomogeneousMaterialTable&); // Not implemented
};

#endif

