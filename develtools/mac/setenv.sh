#!/bin/bash
#
# This is an example setenv for running the faim software from the build
# directory.
#
# To use:
#  - copy it to your build directory
#  - modify the paths below to correspond to your system
#  - run "source setenv.sh"
#
# Note that the first time you configure with cmake, you should use
# setenv_build.sh instead.

source /Users/Shared/install/vtk-6.3.0/setenv.sh

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

set_path VTKBONE_ROOT "${SCRIPT_DIR}"

add_path PATH "${VTKBONE_ROOT}/bin"
add_path DYLD_LIBRARY_PATH "${VTKBONE_ROOT}/lib"
add_path PYTHONPATH "${VTKBONE_ROOT}/lib"
add_path PYTHONPATH "${VTKBONE_ROOT}/Wrapping/Python"
