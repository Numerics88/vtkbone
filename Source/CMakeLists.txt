
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

set (VTKBONE_DISTRIBUTED_HDRS
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

set (VTKBONE_PRIVATE_HDRS
    AbaqusInputReaderHelper.h
    CommandStyleFileReader.h
    )

# === Configure the package
set (VTKBONE_USE_VTKNETCDF ON)
set (VTKBONE_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

add_subdirectory (Config)

# Create version header
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/Config/vtkbone_version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/vtkbone_version.h"
)
list (APPEND VTKBONE_PRIVATE_HDRS
    "${CMAKE_CURRENT_BINARY_DIR}/vtkbone_version.h"
)

# Create config header
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/Config/vtkboneConfigure.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/vtkboneConfigure.h"
)
list (APPEND VTKBONE_DISTRIBUTED_HDRS
    "${CMAKE_CURRENT_BINARY_DIR}/vtkboneConfigure.h"
)

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
mark_as_advanced (pQCTIO_DIR)

# Add include directories
include_directories (
    ${Boost_INCLUDE_DIRS}
    ${N88UTIL_INCLUDE_DIRS}
    ${AIMIO_INCLUDE_DIRS}
    ${PQCTIO_INCLUDE_DIRS}
)

# === Build the library as a VTK module

# Create the module
vtk_module_add_module (vtkbone::vtkbone
    SOURCES ${VTKBONE_SRCS}
    HEADERS ${VTKBONE_DISTRIBUTED_HDRS}
    PRIVATE_HEADERS ${VTKBONE_PRIVATE_HDRS}
)

# Link extra (non-VTK) libraries
vtk_module_link (vtkbone::vtkbone
    PRIVATE
        VTK::netcdf
        AimIO::AimIO
        pQCTIO::pQCTIO
        n88util::n88util
        Boost::date_time
        Boost::filesystem
        Boost::system
)
