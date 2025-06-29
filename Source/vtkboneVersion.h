/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneVersion
    @brief   Versioning class for vtkbone

 Methods for determining the vtkbone version.
*/

#ifndef __vtkboneVersion_h
#define __vtkboneVersion_h

#include "vtkObject.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneVersion : public vtkObject
{
public:
  static vtkboneVersion* New();
  vtkTypeMacro(vtkboneVersion,vtkObject);

  //@{
  /*! Return a string identifier of the version of vtkbone. */
  static const char *GetVTKBONEVersion();
  static const char *GetvtkboneVersion();
  //@}

  //@{
  /*! Return the major version of vtkbone. */
  static int GetVTKBONEMajorVersion();
  static int GetvtkboneMajorVersion();
  //@}

  //@{
  /*! Return the minor version of vtkbone. */
  static int GetVTKBONEMinorVersion();
  static int GetvtkboneMinorVersion();
  //@}

  //@{
  /*! Return the minor minor version of vtkbone. */
  static int GetVTKBONEMinorMinorVersion();
  static int GetvtkboneMinorMinorVersion();
  //@}

protected:
  vtkboneVersion() {};
  ~vtkboneVersion() {};
private:
  vtkboneVersion(const vtkboneVersion&);  // Not implemented.
  void operator=(const vtkboneVersion&);  // Not implemented.
};

#endif
