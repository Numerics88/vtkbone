#!/bin/bash
#
# This is an example setenv for running before configuring with cmake
# (for a development build).
#
# To use:
#  - copy it to your build directory
#  - modify the paths below to correspond to your system
#  - run "source setenv_build.sh"
#  - configure with cmake
#
# This only needs to be done when doing a fresh configure with cmake; 
# subsequently use setenv.sh .

source /usr/local/bin/setenv_functions.sh

set_path BOOST_ROOT "$HOME/build/boost_1_59_0"
set_path GTEST_ROOT "$HOME/build/googletest-release-1.7.0"
set_path HDF5_DIR "$HOME/build/hdf5-1.8.15-patch1-static/share/cmake"
set_path NETCDF_ROOT "$HOME/build/netCDF-c-4.3.3.1-static"
set_path VTK_DIR "/Users/Shared/install/vtk-6.3.0/lib/cmake/vtk-6.3"
