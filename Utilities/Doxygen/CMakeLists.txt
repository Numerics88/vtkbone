#
# Build the documentation
#

INCLUDE (${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)

IF (BUILD_DOCUMENTATION)
  OPTION (DOCUMENTATION_DOWNLOAD_VTK_TAGFILE
         "Download the VTK tag file." ON)
  MARK_AS_ADVANCED (
    DOCUMENTATION_DOWNLOAD_VTK_TAGFILE
  )

  #
  # Configure the script and the doxyfile, then add target
  #
  IF(NOT DOT_PATH)
    GET_FILENAME_COMPONENT(DOT_PATH ${DOT} PATH)
  ENDIF(NOT DOT_PATH)

  # This is a rather hideous hack. The VTK source code contains perl scripts
  # that are required for building the vtkbone documentation. In VTK 5,
  # VTKConfig.cmake contained VTK_SOURCE_DIR. It no longer does. So now we just
  # ask the user to tell us where the VTK source code is.
  IF (NOT VTK_SOURCE_DIR)
    SET (VTK_SOURCE_DIR "NOT_FOUND" CACHE PATH "For building documentation. Can no longer determine automatically.")
  ENDIF()

  CONFIGURE_FILE(
    ${VTKBONE_SOURCE_DIR}/Utilities/Doxygen/doxyfile.in
    ${VTKBONE_BINARY_DIR}/Utilities/Doxygen/doxyfile)

  CONFIGURE_FILE(
    ${VTKBONE_SOURCE_DIR}/Utilities/Doxygen/doc_makeall.sh.in
    ${VTKBONE_BINARY_DIR}/Utilities/Doxygen/doc_makeall.sh)

  ADD_CUSTOM_TARGET(vtkboneDoxygenDoc
    ${BASH}
    ${VTKBONE_BINARY_DIR}/Utilities/Doxygen/doc_makeall.sh)

ENDIF (BUILD_DOCUMENTATION)
