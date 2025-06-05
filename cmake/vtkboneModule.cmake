# CMake code for handling VTK 9 and later

find_package(VTK COMPONENTS
   CommonCore
   CommonDataModel
   CommonExecutionModel
   IOCore
   FiltersExtraction
   FiltersGeometry
   FiltersFlowPaths)

# Set suffix for VTK directories
set(vtk_version_suffix "-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}")

# Use VTK version as suffix for our shared libraries
if (BUILD_SHARED_LIBS)
    set(vtkbone_library_suffix "${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}")
else()
    set(vtkbone_library_suffix)
endif()

# === Use the VTK 9 module API for library and wrapping

# Locate VTK module files in subdirectories
vtk_module_scan(
    MODULE_FILES        "${CMAKE_CURRENT_SOURCE_DIR}/Source/vtkbone.module"
    REQUEST_MODULES     "vtkbone::vtkbone"
    PROVIDES_MODULES    vtkbone_modules
    ENABLE_TESTS        "${BUILD_TESTING}")

# Create the rules for building modules
vtk_module_build(
    MODULES             ${vtkbone_modules}
    INSTALL_EXPORT      vtkbone
    CMAKE_DESTINATION   "${VTKBONE_CMAKE_DESTINATION}"
    LICENSE_DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/vtkbone-${VTKBONE_SHORT_VERSION}"
    HIERARCHY_DESTINATION "${CMAKE_INSTALL_LIBDIR}/vtk${vtk_version_suffix}/hierarchy/vtkbone"
    LIBRARY_NAME_SUFFIX "${vtkbone_library_suffix}"
    VERSION             "${VTKBONE_VERSION}"
    SOVERSION           "${VTKBONE_MAJOR_VERSION}")

# This makes it easty to make our options depend on how VTK was configured
include(CMakeDependentOption)

# Enable Python wrapping by default if VTK was wrapped
cmake_dependent_option(VTKBONE_WRAP_PYTHON "Wrap classes in Python" ON
    "VTK_WRAP_PYTHON" OFF)

# Create rules for Python wrapping
if (VTKBONE_WRAP_PYTHON)
    include(vtkboneModuleWrapPython)
endif()
