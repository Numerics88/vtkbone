include(CMakePackageConfigHelpers)

# Create the main config.cmake file
configure_package_config_file(
    vtkbone-config.cmake.in
    "${PROJECT_BINARY_DIR}/${VTKBONE_CMAKE_DESTINATION}/vtkbone-config.cmake"
    INSTALL_DESTINATION "${VTKBONE_CMAKE_DESTINATION}")

# Automatically generate version file
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${VTKBONE_CMAKE_DESTINATION}/vtkbone-config-version.cmake"
    VERSION "${VTKBONE_VERSION}"
    COMPATIBILITY AnyNewerVersion)

# Make a copy (actually a link) at project root
file(GENERATE
    OUTPUT  "${PROJECT_BINARY_DIR}/vtkbone-config.cmake"
    CONTENT "include(\"${PROJECT_BINARY_DIR}/${VTKBONE_CMAKE_DESTINATION}/vtkbone-config.cmake\")\n")

# Ditto for the version.cmake file
configure_file(
    "${PROJECT_BINARY_DIR}/${VTKBONE_CMAKE_DESTINATION}/vtkbone-config-version.cmake"
    "${PROJECT_BINARY_DIR}/vtkbone-config-version.cmake"
    COPYONLY)

# Install the config files
install(
    FILES
       "${PROJECT_BINARY_DIR}/${VTKBONE_CMAKE_DESTINATION}/vtkbone-config.cmake"
       "${PROJECT_BINARY_DIR}/${VTKBONE_CMAKE_DESTINATION}/vtkbone-config-version.cmake"
    DESTINATION "${VTKBONE_CMAKE_DESTINATION}"
    COMPONENT   "development")
