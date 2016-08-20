#!/bin/bash
#
# This is an example setenv.sh for vtkbone that can go in the install directory.
# However, it must be modified depending on circumstances. For example,
# for a real distribution of vtkbone, vtk is built into the same install
# directory.

source /opt/vtk-6.3.0/setenv.sh

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
 
set_path VTKBONE_ROOT "${SCRIPT_DIR}"
set_path VTKBONE_DIR "${SCRIPT_DIR}/lib/cmake/vtkbone-7.2"

add_path PATH "${VTKBONE_ROOT}/bin"
add_path LD_LIBRARY_PATH "${VTKBONE_ROOT}/lib"
add_path PYTHONPATH "${VTKBONE_ROOT}/lib/python2.7/site-packages"
