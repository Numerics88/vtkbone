/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneWin32Header - manage Windows system differences
// .SECTION Description
// The vtkboneWin32Header captures some system differences between Unix
// and Windows operating systems.

#ifndef __vtkboneWin32Header_h
#define __vtkboneWin32Header_h

#include "vtkboneConfigure.h"

#if defined(WIN32) && !defined(VTKBONE_STATIC)
#if defined(vtkbone_EXPORTS)
#define VTKBONE_EXPORT __declspec( dllexport )
#else
#define VTKBONE_EXPORT __declspec( dllimport )
#endif
#else
#define VTKBONE_EXPORT
#endif

#endif
