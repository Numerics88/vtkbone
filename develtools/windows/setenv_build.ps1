$home_fs = $home -replace "\\", "/"

$env:BOOST_ROOT = $home_fs + "/build/boost_1_59_0"
$env:GTEST_ROOT = $home_fs + "/build/googletest-release-1.7.0"
$env:HDF5_DIR = $home_fs + "/Install/HDF5-1.8.15-win64/cmake"
$env:NETCDF_DIR = $home_fs + "/Install/netCDF-4.3.3.1/lib/cmake"
$env:VTK_DIR = $home_fs + "/Install/VTK-6.3.0/lib/cmake/vtk-6.3"
