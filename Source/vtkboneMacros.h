/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

#ifndef __vtkboneMacros_h
#define __vtkboneMacros_h

// Define a function to string value for an enum.
// This requires:
//  1. An enum defined in a class, and named (enumname)_t
//  2. The last value of the enum must be NUMBER_OF_(enumname)
//  3. Declare the function like this in the header file:
//        static const char* Get(enumname)AsString (enumname_t arg);
//  4. Make a list of string values for your enum in the .cxx file, like this:
//      const char* const (enumname)_s[] = {"BLUE", "RED", "GREEN"};
//  5. Put this macro right after the list of string values
#define vtkboneGetAsStringMacro(classname,enumname) \
const char* classname::Get##enumname##AsString (int arg) \
{ \
  if (arg >=0 && arg < classname::NUMBER_OF_##enumname) \
    { \
    return enumname##_s[arg]; \
    } \
  else \
    { \
    return NULL; \
    } \
}

#endif
