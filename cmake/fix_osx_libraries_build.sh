#!/bin/bash
#
# This script must be run from directory containing the library or executable to modify.

libpath=libvtkbone.dylib
insertString=/Users/Shared/install/vtk-6.3.0/lib/
for myfile in `otool -L $libpath | grep libvtk | grep -v vtkbone | awk '{print $1}'`
do
   echo "changing $myfile to $insertString$myfile" 
   install_name_tool -change "$myfile" "$insertString$myfile" "$libpath"
done
