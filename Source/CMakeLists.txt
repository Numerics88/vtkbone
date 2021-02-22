
# === Set input files

# List of source files that should be wrapped with python (and/or TCL/Java).
# Note that there are fairly strict requirements on such files.
# Typically you only want VTK classes here.
set (VTKBONE_WRAPPED_SRCS
    vtkboneAbaqusInputReader.cxx
    vtkboneAbaqusInputWriter.cxx
    vtkboneAIMReader.cxx
    vtkboneAIMWriter.cxx
    vtkboneApplyTestBase.cxx
    vtkboneApplyBendingTest.cxx
    vtkboneApplyCompressionTest.cxx
    vtkboneApplyDirectionalShearTest.cxx
    vtkboneApplySymmetricShearTest.cxx
    vtkboneApplyTorsionTest.cxx
    vtkboneCoarsenModel.cxx
    vtkboneConstraint.cxx
    vtkboneConstraintUtilities.cxx
    vtkboneConstraintCollection.cxx
    vtkboneDecimateImage.cxx
    vtkboneErrorWarningObserver.cxx
    vtkboneFaimVersion5InputReader.cxx
    vtkboneFaimVersion5InputWriter.cxx
    vtkboneFaimVersion5OutputReader.cxx
    vtkboneFiniteElementModel.cxx
    vtkboneFiniteElementModelAlgorithm.cxx
    vtkboneFiniteElementModelGenerator.cxx
    vtkboneGenerateHommingaMaterialTable.cxx
    vtkboneGenerateHomogeneousMaterialTable.cxx
    vtkboneImageConnectivityFilter.cxx
    vtkboneImageConnectivityMap.cxx
    vtkboneImageToMesh.cxx
    vtkboneInterpolateCoarseSolution.cxx
    vtkboneLinearIsotropicMaterial.cxx
    vtkboneLinearIsotropicMaterialArray.cxx
    vtkboneLinearOrthotropicMaterial.cxx
    vtkboneLinearOrthotropicMaterialArray.cxx
    vtkboneLinearAnisotropicMaterial.cxx
    vtkboneLinearAnisotropicMaterialArray.cxx
    vtkboneMaterial.cxx
    vtkboneMaterialArray.cxx
    vtkboneMaterialTable.cxx
    vtkboneMaterialTableAlgorithm.cxx
    vtkboneMohrCoulombIsotropicMaterial.cxx
    vtkboneMaximumPrincipalStrainIsotropicMaterial.cxx
    vtkboneN88ModelReader.cxx
    vtkboneN88ModelWriter.cxx
    vtkboneNodeSetsByGeometry.cxx
    vtkboneOrientationFilter.cxx
    vtkbonepQCTReader.cxx
    vtkboneReorderDataArray.cxx
    vtkboneSelectionUtilities.cxx
    vtkboneSelectVisiblePoints.cxx
    vtkboneSolverParameters.cxx
    vtkboneStressStrainMatrix.cxx
    vtkboneTensorOfInertia.cxx
    vtkboneTensor.cxx
    vtkboneVerifyUnstructuredGrid.cxx
    vtkboneVersion.cxx
    vtkboneVonMisesIsotropicMaterial.cxx
    )

# List of source files that should not be wrapped with python (and/or TCL/Java).
# This is typically source code that is not VTK classes.
set (VTKBONE_NONWRAPPED_SRCS
    vtkbonePrettyReports.cxx
    CommandStyleFileReader.cxx
    AbaqusInputReaderHelper.cxx
    )
set_source_files_properties (${VTKBONE_NONWRAPPED_SRCS}
    PROPERTIES WRAP_EXCLUDE ON)

# All sources
set (VTKBONE_SRCS ${VTKBONE_WRAPPED_SRCS} ${VTKBONE_NONWRAPPED_SRCS})

set (VTKBONE_DISTRIBUTED_INCLUDE_FILES
    vtkboneAbaqusInputReader.h
    vtkboneAbaqusInputWriter.h
    vtkboneAIMReader.h
    vtkboneAIMWriter.h
    vtkboneApplyTestBase.h
    vtkboneApplyBendingTest.h
    vtkboneApplyCompressionTest.h
    vtkboneApplyDirectionalShearTest.h
    vtkboneApplySymmetricShearTest.h
    vtkboneApplyTorsionTest.h
    vtkboneCoarsenModel.h
    vtkboneConstraint.h
    vtkboneConstraintCollection.h
    vtkboneConstraintUtilities.h
    vtkboneDecimateImage.h
    vtkboneErrorWarningObserver.h
    vtkboneFaimVersion5InputReader.h
    vtkboneFaimVersion5InputWriter.h
    vtkboneFaimVersion5OutputReader.h
    vtkboneFiniteElementModel.h
    vtkboneFiniteElementModelAlgorithm.h
    vtkboneFiniteElementModelGenerator.h
    vtkboneGenerateHommingaMaterialTable.h
    vtkboneGenerateHomogeneousMaterialTable.h
    vtkboneImageConnectivityFilter.h
    vtkboneImageConnectivityMap.h
    vtkboneImageToMesh.h
    vtkboneInterpolateCoarseSolution.h
    vtkboneLinearIsotropicMaterial.h
    vtkboneLinearIsotropicMaterialArray.h
    vtkboneLinearOrthotropicMaterial.h
    vtkboneLinearOrthotropicMaterialArray.h
    vtkboneLinearAnisotropicMaterial.h
    vtkboneLinearAnisotropicMaterialArray.h
    vtkboneMaterial.h
    vtkboneMaterialArray.h
    vtkboneMaterialTable.h
    vtkboneMaterialTableAlgorithm.h
    vtkboneMaximumPrincipalStrainIsotropicMaterial.h
    vtkboneMohrCoulombIsotropicMaterial.h
    vtkboneN88ModelReader.h
    vtkboneN88ModelWriter.h
    vtkboneNodeSetsByGeometry.h
    vtkboneOrientationFilter.h
    vtkbonepQCTReader.h
    vtkbonePrettyReports.h
    vtkboneReorderDataArray.h
    vtkboneSelectionUtilities.h
    vtkboneSelectVisiblePoints.h
    vtkboneSolverParameters.h
    vtkboneStressStrainMatrix.h
    vtkboneTensorOfInertia.h
    vtkboneTensor.h
    vtkboneVerifyUnstructuredGrid.h
    vtkboneVersion.h
    vtkboneVonMisesIsotropicMaterial.h
    vtkboneWin32Header.h
    vtkboneMacros.h
    )

# === Options around shared libraries

# Build shared libs ?
# Defaults to the same VTK setting.
option (BUILD_SHARED_LIBS
        "Build shared libraries."
        ${VTK_BUILD_SHARED_LIBS})
set (VTKBONE_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

# === Configure the package

add_subdirectory(Config)

# === Find packages

# Requires Boost
# set (Boost_USE_STATIC_LIBS ON)
find_package (Boost 1.61.0 COMPONENTS date_time filesystem thread system REQUIRED)

# Requires n88util
find_package (n88util 2.0 REQUIRED)
mark_as_advanced (n88util_DIR)

# Requires AimIO
find_package (AimIO 1.0 REQUIRED)
mark_as_advanced (AimIO_DIR)

# Requires pQCT
find_package (pQCTIO 1.0 REQUIRED)
mark_as_advanced (pQCTIO_Dir)

# Requires VTK
find_package (VTK COMPONENTS
    vtkFiltersExtraction
    vtkFiltersGeometry
    vtkFiltersFlowPaths
    vtkIOCore
    NO_MODULE
)
mark_as_advanced (VTK_DIR)

# Requires netCDF
find_package(netCDF REQUIRED)

# Include all directories
include_directories (${PROJECT_BINARY_DIR}/Source/Config/)
include_directories (${Boost_INCLUDE_DIR})
include_directories (${N88UTIL_INCLUDE_DIRS})
include_directories (${AIMIO_INCLUDE_DIRS})
include_directories (${PQCTIO_INCLUDE_DIRS})
include_directories (${netCDF_INCLUDE_DIRS})
include ("${VTK_USE_FILE}")

# === Create the vtkbone C++ library
add_library (vtkbone ${VTKBONE_SRCS})
target_link_libraries (vtkbone
    PUBLIC
        vtkFiltersExtraction vtkFiltersGeometry vtkFiltersFlowPaths vtkIOCore
    PRIVATE
        AimIO::AimIO
        pQCTIO::pQCTIO
        n88util::n88util
        Boost::date_time
 				Boost::filesystem
 				Boost::system
				${ZLIB_LIBRARY}
				${HDF5_HL_LIBRARY}
				${HDF5_LIBRARY}
        ${netCDF_LIBRARY}
)
# The ZLIB, HDF5_HL, HDF5 and netCDF libraries above used to be replaced
# specifically by 'netcdf'.

# On Linux systems, need to build static libraries with -fPIC
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT BUILD_SHARED_LIBS)
    target_compile_options(vtkbone PRIVATE -fPIC)
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

# Add all targets to the build-tree export set
export (TARGETS vtkbone
    FILE "${PROJECT_BINARY_DIR}/vtkboneTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE vtkbone)

# Install the vtkboneConfig.cmake and vtkboneConfigVersion.cmake
install(
    FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/vtkboneConfig.cmake"
        "${PROJECT_BINARY_DIR}/vtkboneConfigVersion.cmake"
    DESTINATION
        "${INSTALL_CMAKE_DIR}"
    COMPONENT
        dev
)
 
# Install the export set for use with the install-tree
install(EXPORT vtkboneTargets
    NAMESPACE
        vtkbone::
    DESTINATION
        "${INSTALL_CMAKE_DIR}"
    COMPONENT
        dev
)

# === Wrap Python
include (${PROJECT_SOURCE_DIR}/cmake/vtkboneWrapPython.cmake)
if (VTKBONE_WRAP_PYTHON)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    vtkbone_wrap_python(vtkbone "${VTKBONE_WRAPPED_SRCS}")

    #=== Setup remainder of package

    # Copy vtkbone to the build directory.
    configure_file (
        "${PROJECT_SOURCE_DIR}/Wrapping/Python/vtkbone/__init__.py"
        "${PROJECT_BINARY_DIR}/Wrapping/Python/vtkbone/__init__.py"
    )

    # Compile .py files to .pyc files
    configure_file(
        "${PROJECT_SOURCE_DIR}/Wrapping/Python/compile_all_vtk.py.in"
        "${PROJECT_BINARY_DIR}/Wrapping/Python/compile_all_vtk.py"
    )
    add_custom_command(
        COMMAND "${Python_EXECUTABLE}" "${PROJECT_BINARY_DIR}/Wrapping/Python/compile_all_vtk.py"
        DEPENDS "${PROJECT_BINARY_DIR}/Wrapping/Python/compile_all_vtk.py"
        OUTPUT "${PROJECT_BINARY_DIR}/Wrapping/Python/vtk_compile_complete"
    )
    add_custom_target(vtkbonepython_pyc ALL
        DEPENDS "${PROJECT_BINARY_DIR}/Wrapping/Python/vtk_compile_complete"
    )

    install (DIRECTORY "${PROJECT_BINARY_DIR}/Wrapping/Python/vtkbone"
        DESTINATION "${SITE_PACKAGES}"
        PATTERN vtk_compile_complete EXCLUDE
    )
endif()
