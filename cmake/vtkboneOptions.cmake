
# Ensure that CMake behaves predictably
set (CMAKE_EXPORT_NO_PACKAGE_REGISTRY ON)
set (CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY ON)
set (CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY ON)

# Disable auto-linking for boost, as it causes endless grief
if (WIN32)
    add_definitions (-DBOOST_ALL_NO_LIB)
endif ()

# Defines CMAKE_INSTALL_LIBDIR etc. with relative path
# as well as CMAKE_INSTALL_FULL_LIBDIR etc. with absolute path
include(GNUInstallDirs)

# Libraries should be built into lib
if (NOT WIN32)
    set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif ()

# Where to put cmake config files (use our own version number)
if (WIN32 AND NOT CYGWIN)
    set (VTKBONE_CMAKE_DESTINATION CMake)
else()
    set (VTKBONE_CMAKE_DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/vtkbone-${VTKBONE_SHORT_VERSION}")
endif()
