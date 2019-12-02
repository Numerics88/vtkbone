
# Create version file
configure_file (
    "${PROJECT_SOURCE_DIR}/Config/vtkbone_version.h.in"
    "${PROJECT_BINARY_DIR}/Config/vtkbone_version.h"
)

# Create the vtkboneConfigure.h file.
# To include this file in source,
#   include_directories (${PROJECT_BINARY_DIR}/Config/)
configure_file (
    "${PROJECT_SOURCE_DIR}/Config/vtkboneConfigure.h.in"
    "${PROJECT_BINARY_DIR}/Config/vtkboneConfigure.h"
)
install (FILES "${PROJECT_BINARY_DIR}/Config/vtkboneConfigure.h" DESTINATION include)

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
