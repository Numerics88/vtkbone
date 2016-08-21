cmake_minimum_required (VERSION 3.1)
project (vtkbone)

# === Versioning

set(VTKBONE_MAJOR_VERSION 1)
set(VTKBONE_MINOR_VERSION 0)
set(VTKBONE_PATCH_VERSION 0)
set (VTKBONE_VERSION_TAG "devel")

# This is the version for CMake purposes, which must be of the form x.y.z
set(VTKBONE_VERSION
    ${VTKBONE_MAJOR_VERSION}.${VTKBONE_MINOR_VERSION}.${VTKBONE_PATCH_VERSION})

# This is the version for the code, which might include tags
set(VTKBONE_FULL_VERSION ${VTKBONE_MAJOR_VERSION}.${VTKBONE_MINOR_VERSION})
if (VTKBONE_PATCH_VERSION)
    set(VTKBONE_FULL_VERSION ${VTKBONE_FULL_VERSION}.${VTKBONE_PATCH_VERSION})
endif()
if (VTKBONE_VERSION_TAG)
    set(VTKBONE_FULL_VERSION ${VTKBONE_FULL_VERSION}-${VTKBONE_VERSION_TAG})
endif()

configure_file (
    "${PROJECT_SOURCE_DIR}/vtkbone_version.h.in"
    "${PROJECT_BINARY_DIR}/vtkbone_version.h")


# === Hide a lot of complication in CMakeOptions file.

include (${PROJECT_SOURCE_DIR}/CMakeOptions.cmake)


# === Source code files

# List of source files that should be wrapped with python (and/or TCL/Java).
# Note that there are fairly strict requirements on such files.
# Typically you only want VTK classes here.
set (VTKBONE_WRAPPED_SRCS
    Source/vtkboneAbaqusInputReader.cxx
    Source/vtkboneAbaqusInputWriter.cxx
    Source/vtkboneAIMReader.cxx
    Source/vtkboneAIMWriter.cxx
    Source/vtkboneApplyTestBase.cxx
    Source/vtkboneApplyBendingTest.cxx
    Source/vtkboneApplyCompressionTest.cxx
    Source/vtkboneApplyDirectionalShearTest.cxx
    Source/vtkboneApplySymmetricShearTest.cxx
    Source/vtkboneApplyTorsionTest.cxx
    Source/vtkboneCoarsenModel.cxx
    Source/vtkboneConstraint.cxx
    Source/vtkboneConstraintUtilities.cxx
    Source/vtkboneConstraintCollection.cxx
    Source/vtkboneDecimateImage.cxx
    Source/vtkboneErrorWarningObserver.cxx
    Source/vtkboneFaimVersion5InputReader.cxx
    Source/vtkboneFaimVersion5InputWriter.cxx
    Source/vtkboneFaimVersion5OutputReader.cxx
    Source/vtkboneFiniteElementModel.cxx
    Source/vtkboneFiniteElementModelAlgorithm.cxx
    Source/vtkboneFiniteElementModelGenerator.cxx
    Source/vtkboneGenerateHommingaMaterialTable.cxx
    Source/vtkboneGenerateHomogeneousMaterialTable.cxx
    Source/vtkboneImageConnectivityFilter.cxx
    Source/vtkboneImageConnectivityMap.cxx
    Source/vtkboneImageToMesh.cxx
    Source/vtkboneInterpolateCoarseSolution.cxx
    Source/vtkboneLinearIsotropicMaterial.cxx
    Source/vtkboneLinearIsotropicMaterialArray.cxx
    Source/vtkboneLinearOrthotropicMaterial.cxx
    Source/vtkboneLinearOrthotropicMaterialArray.cxx
    Source/vtkboneLinearAnisotropicMaterial.cxx
    Source/vtkboneLinearAnisotropicMaterialArray.cxx
    Source/vtkboneMaterial.cxx
    Source/vtkboneMaterialArray.cxx
    Source/vtkboneMaterialTable.cxx
    Source/vtkboneMaterialTableAlgorithm.cxx
    Source/vtkboneMohrCoulombIsotropicMaterial.cxx
    Source/vtkboneMaximumPrincipalStrainIsotropicMaterial.cxx
    Source/vtkboneN88ModelReader.cxx
    Source/vtkboneN88ModelWriter.cxx
    Source/vtkboneNodeSetsByGeometry.cxx
    Source/vtkboneOrientationFilter.cxx
    Source/vtkboneReorderDataArray.cxx
    Source/vtkboneSelectionUtilities.cxx
    Source/vtkboneSelectVisiblePoints.cxx
    Source/vtkboneSolverParameters.cxx
    Source/vtkboneStressStrainMatrix.cxx
    Source/vtkboneTensorOfInertia.cxx
    Source/vtkboneVerifyUnstructuredGrid.cxx
    Source/vtkboneVersion.cxx
    Source/vtkboneVonMisesIsotropicMaterial.cxx
    )

# List of source files that should not be wrapped with python (and/or TCL/Java).
# This is typically source code that is not VTK classes.
set (VTKBONE_NONWRAPPED_SRCS
    Source/vtkbonePrettyReports.cxx
    HelperCode/CommandStyleFileReader.cxx
    HelperCode/AbaqusInputReaderHelper.cxx
    )
set_source_files_properties (${VTKBONE_NONWRAPPED_SRCS}
    PROPERTIES WRAP_EXCLUDE ON)

# All sources
set (VTKBONE_SRCS ${VTKBONE_WRAPPED_SRCS} ${VTKBONE_NONWRAPPED_SRCS})

set (VTKBONE_DISTRIBUTED_INCLUDE_FILES
    Source/vtkboneAbaqusInputReader.h
    Source/vtkboneAbaqusInputWriter.h
    Source/vtkboneAIMReader.h
    Source/vtkboneAIMWriter.h
    Source/vtkboneApplyTestBase.h
    Source/vtkboneApplyBendingTest.h
    Source/vtkboneApplyCompressionTest.h
    Source/vtkboneApplyDirectionalShearTest.h
    Source/vtkboneApplySymmetricShearTest.h
    Source/vtkboneApplyTorsionTest.h
    Source/vtkboneCoarsenModel.h
    Source/vtkboneConstraint.h
    Source/vtkboneConstraintCollection.h
    Source/vtkboneConstraintUtilities.h
    Source/vtkboneDecimateImage.h
    Source/vtkboneErrorWarningObserver.h
    Source/vtkboneFaimVersion5InputReader.h
    Source/vtkboneFaimVersion5InputWriter.h
    Source/vtkboneFaimVersion5OutputReader.h
    Source/vtkboneFiniteElementModel.h
    Source/vtkboneFiniteElementModelAlgorithm.h
    Source/vtkboneFiniteElementModelGenerator.h
    Source/vtkboneGenerateHommingaMaterialTable.h
    Source/vtkboneGenerateHomogeneousMaterialTable.h
    Source/vtkboneImageConnectivityFilter.h
    Source/vtkboneImageConnectivityMap.h
    Source/vtkboneImageToMesh.h
    Source/vtkboneInterpolateCoarseSolution.h
    Source/vtkboneLinearIsotropicMaterial.h
    Source/vtkboneLinearIsotropicMaterialArray.h
    Source/vtkboneLinearOrthotropicMaterial.h
    Source/vtkboneLinearOrthotropicMaterialArray.h
    Source/vtkboneLinearAnisotropicMaterial.h
    Source/vtkboneLinearAnisotropicMaterialArray.h
    Source/vtkboneMaterial.h
    Source/vtkboneMaterialArray.h
    Source/vtkboneMaterialTable.h
    Source/vtkboneMaterialTableAlgorithm.h
    Source/vtkboneMaximumPrincipalStrainIsotropicMaterial.h
    Source/vtkboneMohrCoulombIsotropicMaterial.h
    Source/vtkboneN88ModelReader.h
    Source/vtkboneN88ModelWriter.h
    Source/vtkboneNodeSetsByGeometry.h
    Source/vtkboneOrientationFilter.h
    Source/vtkbonePrettyReports.h
    Source/vtkboneReorderDataArray.h
    Source/vtkboneSelectionUtilities.h
    Source/vtkboneSelectVisiblePoints.h
    Source/vtkboneSolverParameters.h
    Source/vtkboneStressStrainMatrix.h
    Source/vtkboneTensorOfInertia.h
    Source/vtkboneVerifyUnstructuredGrid.h    
    Source/vtkboneVersion.h
    Source/vtkboneVonMisesIsotropicMaterial.h
    Source/vtkboneWin32Header.h
    )

# === Set include directories

include_directories (${PROJECT_BINARY_DIR})
include_directories (${PROJECT_SOURCE_DIR})
include_directories (${PROJECT_SOURCE_DIR}/Source)
include_directories (${Boost_INCLUDE_DIR})
include_directories (${CMAKE_SOURCE_DIR}/Utilities)
include_directories (${CMAKE_SOURCE_DIR}/HelperCode)
include_directories (${N88UTIL_INCLUDE_DIRS})


# === Create the vtkbone C++ library

add_library (vtkbone ${VTKBONE_SRCS})
target_link_libraries (vtkbone
    ${VTK_LIBRARIES}
    AimIO
    n88util
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    )

if (VTKBONE_USE_VTKNETCDF)
    target_link_libraries (vtkbone
        ${vtknetcdf_LIBRARIES}
        ${vtkhdf5_LIBRARIES})
else ()
    target_link_libraries (vtkbone ${netCDF_LIBRARIES})
endif ()

if (BUILD_SHARED_LIBS)
    set_target_properties(vtkbone
        PROPERTIES
            OUTPUT_NAME vtkbone-${VTKBONE_MAJOR_VERSION}.${VTKBONE_MINOR_VERSION})
endif()

install (TARGETS vtkbone
         EXPORT vtkboneTargets
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)

install (FILES ${VTKBONE_DISTRIBUTED_INCLUDE_FILES} DESTINATION include)


# === Create the vtkbone Python library.

# VTK_WRAP_PYTHON3 does not handle relative paths, only bare file names or
# complete paths.  Therefore convert to full paths.
foreach (FILE ${VTKBONE_WRAPPED_SRCS})
    set (VTKBONE_WRAPPED_SRCS_ABSPATH
         ${VTKBONE_WRAPPED_SRCS_ABSPATH} "${PROJECT_SOURCE_DIR}/${FILE}")
endforeach (FILE ${VTKBONE_WRAPPED_SRCS})
if (VTK_WRAP_PYTHON AND VTKBONE_WRAP_PYTHON)
    include_directories("${PYTHON_INCLUDE_PATH}")
    # Following 6 lines seem to be necessary in 6.0 to be able to find headers:
    include_directories (${VTK_DIR}/Wrapping/Python)
    include_directories (${VTK_DIR}/Wrapping/PythonCore)
    include_directories (${VTK_DIR}/Utilities/Python)
    include_directories (${VTK_CMAKE_DIR}/../Wrapping/Python)
    include_directories (${VTK_CMAKE_DIR}/../Wrapping/PythonCore)
    include_directories (${VTK_CMAKE_DIR}/../Utilities/Python)
    vtk_wrap_python3 (vtkbonePython vtkbonePython_SRCS "${VTKBONE_WRAPPED_SRCS_ABSPATH}")
    add_library (vtkbonePythonD ${vtkbonePython_SRCS})
    add_library (vtkbonePython MODULE vtkbonePythonInit.cxx)
    # The following is a horrible hack and probably not the way to
    # actually properly link, but for now it works.
    foreach (LIB ${VTK_LIBRARIES})
        if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
            set (FILE "${VTK_INSTALL_PREFIX}/lib/${LIB}Python27D-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.lib")
        elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            set (FILE "${VTK_INSTALL_PREFIX}/lib/lib${LIB}Python27D-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.dylib")
        else ()
            set (FILE "${VTK_INSTALL_PREFIX}/lib/lib${LIB}Python27D-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.so")
        endif()
        if (EXISTS "${FILE}")
            set (VTKBONE_PYTHOND_LIBRARIES ${VTKBONE_PYTHOND_LIBRARIES} ${FILE})
        endif ()
    endforeach ()
    target_link_libraries(vtkbonePythonD
                          vtkbone
                          vtkWrappingPythonCore
                          ${VTKBONE_PYTHOND_LIBRARIES}
                          ${VTK_PYTHON_LIBRARIES})
    target_link_libraries(vtkbonePython vtkbonePythonD)
    set_target_properties(vtkbonePython PROPERTIES PREFIX "")
    if (WIN32 AND NOT CYGWIN)
        set_target_properties (vtkbonePython PROPERTIES SUFFIX ".pyd")
    endif ()
    install (TARGETS vtkbonePythonD
             EXPORT vtkboneTargets
             RUNTIME DESTINATION bin
             LIBRARY DESTINATION lib
             ARCHIVE DESTINATION lib)
    install (TARGETS vtkbonePython
             EXPORT vtkboneTargets
             RUNTIME DESTINATION "${PYTHON_SITE_PACKAGES}/vtkbone"
             LIBRARY DESTINATION "${PYTHON_SITE_PACKAGES}/vtkbone")
    # Copy vtkbone to the build directory.
    configure_file ("${PROJECT_SOURCE_DIR}/Wrapping/Python/vtkbone/__init__.py"
                    "${PROJECT_BINARY_DIR}/Wrapping/Python/vtkbone/__init__.py")
    # Compile .py files to .pyc files
    configure_file("${PROJECT_SOURCE_DIR}/Wrapping/Python/compile_all_vtk.py.in"
                   "${PROJECT_BINARY_DIR}/Wrapping/Python/compile_all_vtk.py")
    add_custom_command(
        COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_BINARY_DIR}/Wrapping/Python/compile_all_vtk.py"
        DEPENDS "${PROJECT_BINARY_DIR}/Wrapping/Python/compile_all_vtk.py"
        OUTPUT "${PROJECT_BINARY_DIR}/Wrapping/Python/vtk_compile_complete"
        )
    add_custom_target(vtkbonepython_pyc ALL
        DEPENDS "${PROJECT_BINARY_DIR}/Wrapping/Python/vtk_compile_complete")
    install (DIRECTORY "${PROJECT_BINARY_DIR}/Wrapping/Python/vtkbone"
             DESTINATION "${PYTHON_SITE_PACKAGES}"
             PATTERN vtk_compile_complete EXCLUDE)
endif ()


# === CMake config files for importing this project

# Set some locations for the install
set(INSTALL_LIB_DIR lib)
set(INSTALL_BIN_DIR bin)
set(INSTALL_INCLUDE_DIR include)
if(WIN32 AND NOT CYGWIN)
    set(INSTALL_CMAKE_DIR CMake)
else()
    set(INSTALL_CMAKE_DIR "lib/cmake/vtkbone-${VTKBONE_MAJOR_VERSION}.${VTKBONE_MINOR_VERSION}")
endif()

# Make relative paths absolute
foreach(p LIB BIN INCLUDE CMAKE)
    set(var INSTALL_${p}_DIR)
    if(NOT IS_ABSOLUTE "${${var}}")
        set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
    endif()
endforeach()

# Add all targets to the build-tree export set
export (TARGETS vtkbone
    FILE "${PROJECT_BINARY_DIR}/vtkboneTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE vtkbone)

# Create the vtkboneConfig.cmake and vtkboneConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
     "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS
    "${PROJECT_SOURCE_DIR}/Source"
    "${PROJECT_BINARY_DIR}")
configure_file(vtkboneConfig.cmake.in
    "${PROJECT_BINARY_DIR}/vtkboneConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${VTKBONE_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(vtkboneConfig.cmake.in
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/vtkboneConfig.cmake" @ONLY)
# ... for both
configure_file(vtkboneConfigVersion.cmake.in
    "${PROJECT_BINARY_DIR}/vtkboneConfigVersion.cmake" @ONLY)

# Install the vtkboneConfig.cmake and vtkboneConfigVersion.cmake
install(FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/vtkboneConfig.cmake"
    "${PROJECT_BINARY_DIR}/vtkboneConfigVersion.cmake"
    DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
 
# Install the export set for use with the install-tree
install(EXPORT vtkboneTargets DESTINATION
    "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Create the vtkboneConfigure.h file.
configure_file (
    "${PROJECT_SOURCE_DIR}/vtkboneConfigure.h.in"
    "${PROJECT_BINARY_DIR}/vtkboneConfigure.h")
install (FILES "${PROJECT_BINARY_DIR}/vtkboneConfigure.h" DESTINATION include)


# === Testing

if (ENABLE_TESTING)
    add_subdirectory (Testing)
endif ()
