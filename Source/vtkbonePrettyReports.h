/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkbonePrettyReports - provides nice user-targetted summary of
//  vtkbone and some vtk objects.
//
// .SECTION Description
// Provides a output generating functions for some vtk objects.
// These are in the namespace vtkbonePrettyReports, and are all called Print.
// They are overloaded with various classes as input.
// For vtkbone classes, similar calls are implemented directly in the
// corresponding classes' .h/.cxx files.

#ifndef __vtkbonePrettyReports_h
#define __vtkbonePrettyReports_h

#include <ostream>

// Forward declarations
class vtkImageAccumulate;
class vtkMassProperties;

namespace vtkbonePrettyReports
{
  void Print(std::ostream& s, vtkImageAccumulate* obj);
  void Print(std::ostream& s, vtkMassProperties* obj);
}

#endif
