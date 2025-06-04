#!/bin/bash
#
# This script must be run from the directory containing the library or executable to modify.

libpath=libvtkbone.dylib
insertString=/Users/Shared/install/vtk-6.3.0/lib/

# Iterate over dependencies
for myfile in $(otool -L $libpath | grep libvtk | grep -v vtkbone | awk '{print $1}')
do
   # Skip system libraries
   if [[ "$myfile" == /usr/lib/* || "$myfile" == /Library/Developer/CommandLineTools/* ]]; then
      echo "Skipping system library: $myfile"
      continue
   fi

   # Modify library path
   echo "Changing $myfile to $insertString$(basename $myfile)"
   install_name_tool -change "$myfile" "$insertString$(basename $myfile)" "$libpath"
done