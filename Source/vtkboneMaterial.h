/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneMaterial
    @brief   An abstract base class for materials used in
 finite element models


 This base class may be derived to define arbitrary types of materials.

 All materials are required to have unique names.
 Materials are named as "NewMaterial%d" when constructed, where %d is a
 number assigned by a static counter.  This will usually ensure conflict-free
 names, however it is not thread safe.  It is recommended that you assign
 informative names using SetName.

    @sa
 vtkboneLinearIsotropicMaterial vtkboneLinearOrthotropicMaterial vtkboneMaterialTable
 vtkboneGenerateHomogeneousLinearIsotropicMaterialTable
 vtkboneGenerateHommingaMaterialTable
*/

#ifndef __vtkboneMaterial_h
#define __vtkboneMaterial_h

#include "vtkObject.h"
#include "vtkIdTypeArray.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneMaterial : public vtkObject
{
  public:
    static vtkboneMaterial* New();
    vtkTypeMacro(vtkboneMaterial, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    //@{
    /*! Set/get the name of the material. */
    vtkSetStringMacro(Name);
    vtkGetStringMacro(Name);
    //@}

    /*! Creates a copy of this object. You should almost certainly give the
	copy or the original a new name. */
    virtual vtkboneMaterial* Copy();

    /*! Creates a copy of this object, with all the modulii scaled by
	factor. You should almost certainly give the copy or the original a
	new name. Note that you will have to delete the pointer when
	finished with it. */
    virtual vtkboneMaterial* ScaledCopy(double factor);

  protected:
    vtkboneMaterial();
    ~vtkboneMaterial();

    char* Name;

    static int nameCounter;

  private:
    vtkboneMaterial(const vtkboneMaterial&);  // Not implemented.
    void operator=(const vtkboneMaterial&);  // Not implemented.
};

#endif

