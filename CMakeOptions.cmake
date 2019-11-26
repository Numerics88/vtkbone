
# Macro to replace the extension of a file name
# Is there a standard way to do this?
macro (REPLACE_EXTENSION outpath inpath extension)
    get_filename_component (inpath_PATH ${inpath} PATH)
    get_filename_component (inpath_NAME_WE ${inpath} NAME_WE)
    file (TO_CMAKE_PATH "${inpath_PATH}/${inpath_NAME_WE}.${extension}" ${outpath})
endmacro (REPLACE_EXTENSION)

# Testing
# Note: must be enabled in root CMake file.
option (ENABLE_TESTING "Build unit and other tests" ON)
if (ENABLE_TESTING)
    enable_testing()
    find_package (GTest REQUIRED)
endif ()

# Disable auto-linking for boost, as it causes endless grief
if (WIN32)
    add_definitions (-DBOOST_ALL_NO_LIB)
endif ()

# Libraries should be built into lib
if (NOT WIN32)
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif ()

# Requires Boost
# set (Boost_USE_STATIC_LIBS ON)
find_package (Boost 1.61.0 COMPONENTS date_time filesystem system REQUIRED)

# Requires n88util
find_package (n88util 2.0 REQUIRED)
mark_as_advanced (n88util_DIR)

# Requires AimIO
find_package (AimIO 1.0 REQUIRED)
mark_as_advanced (AimIO_DIR)

# Requires pQCT
find_package (pQCTIO 1.0 REQUIRED)
mark_as_advanced (pQCTIO_Dir)

add_subdirectory (Utilities)

# We need to include TBB untill VTK fixes their CMake exports
find_package(tbb REQUIRED)

find_package (VTK COMPONENTS
    vtkFiltersExtraction
    vtkFiltersGeometry
    vtkFiltersFlowPaths
    vtknetcdf
    NO_MODULE)
mark_as_advanced (VTK_DIR)
include ("${VTK_USE_FILE}")

option (VTKBONE_USE_VTKNETCDF "Use VTK netcdf libraries instead of separate netcdf libraries." OFF)
mark_as_advanced (VTKBONE_USE_VTKNETCDF)

if (VTKBONE_USE_VTKNETCDF)
    add_definitions (-DVTKBONE_USE_VTKNETCDF)
else ()
    # Requires NetCDF4
    if (NOT VTKBONE_USE_VTKNETCDF)
        option (N88_USE_NETCDF_CONFIG "Use netCDF config file" OFF)
        mark_as_advanced (N88_USE_NETCDF_CONFIG)
        if (N88_USE_NETCDF_CONFIG)
          find_package (netCDF CONFIG REQUIRED)
          mark_as_advanced (netCDF_DIR)
        else()
          # We manually find netCDF and everything that it depends on.
          # Assume it is built with zlib but not szip.
          if (DEFINED ENV{CONDA_PREFIX})
            if (WIN32)
               set (_nc_hints $ENV{CONDA_PREFIX}/Library/lib)
             else()
               set (_nc_hints $ENV{CONDA_PREFIX}/lib)
             endif()
          endif()
          find_path (netCDF_INCLUDE_DIR NAMES netcdf.h
                     HINTS ${_nc_hints})
          mark_as_advanced (netCDF_INCLUDE_DIR)
          find_library (netCDF_LIBRARY NAMES netcdf libnetcdf
                        HINTS ${_nc_hints})
          mark_as_advanced (netCDF_LIBRARY)
          find_path (HDF5_INCLUDE_DIR NAMES netcdf.h
                     HINTS ${_nc_hints})
          mark_as_advanced (HDF5_INCLUDE_DIR)
          find_library (HDF5_LIBRARY NAMES hdf5 libhdf5
                        HINTS ${_nc_hints})
          mark_as_advanced (HDF5_LIBRARY)
          find_library (HDF5_HL_LIBRARY NAMES hdf5_hl libhdf5_hl
                        HINTS ${_nc_hints})
          mark_as_advanced (HDF5_HL_LIBRARY)
          find_package (ZLIB REQUIRED)
          set (netCDF_LIBRARIES "${netCDF_LIBRARY};${HDF5_HL_LIBRARY};${HDF5_LIBRARY};${ZLIB_LIBRARIES}")
        endif()
    endif()
endif ()

# Build shared libs ?
# Defaults to the same VTK setting.
option (BUILD_SHARED_LIBS
        "Build shared libraries."
        ${VTK_BUILD_SHARED_LIBS})
set (VTKBONE_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
# On Linux systems, need to build static libraries with -fPIC
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT BUILD_SHARED_LIBS)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif ()


# === Wrap Python

if (VTK_WRAP_PYTHON)

    option (VTKBONE_WRAP_PYTHON
          "Wrap classes into the Python interpreted language."
          ON)

    if (VTKBONE_WRAP_PYTHON)
        SET(VTK_WRAP_PYTHON_FIND_LIBS ON)
        include (${VTK_CMAKE_DIR}/vtkWrapPython.cmake)
        if (WIN32)
            if (NOT BUILD_SHARED_LIBS)
                message (FATAL_ERROR "Python support requires BUILD_SHARED_LIBS to be ON.")
                set (VTKBONE_CAN_BUILD 0)
            endif ()
        endif ()
        find_package(PythonInterp REQUIRED)
        find_package(PythonLibs REQUIRED)
        # Strangely, PythonLibs keeps secret where the local project install directory is.
        # So we have to ask Python ourselves:
        execute_process ( COMMAND "${PYTHON_EXECUTABLE}" -c
          "from distutils.sysconfig import get_python_lib; print (get_python_lib())"
          OUTPUT_VARIABLE PYTHON_SITE_PACKAGES_DEFAULT
          OUTPUT_STRIP_TRAILING_WHITESPACE)
        set (PYTHON_SITE_PACKAGES "${PYTHON_SITE_PACKAGES_DEFAULT}" CACHE PATH
          "Local installation location of python packages")
      endif ()

else ()

    if (VTKBONE_WRAP_PYTHON)
        message ("Warning. VTKBONE_WRAP_PYTHON is ON but the VTK version you have "
                 "chosen has not support for Python (VTK_WRAP_PYTHON is OFF).  "
                 "Please set VTKBONE_WRAP_PYTHON to OFF.")
        set (VTKBONE_WRAP_PYTHON OFF)
    endif ()

endif ()
