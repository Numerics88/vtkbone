
# - Find netCDF library
# - Derived from the FindSZIP.cmake that is included with netcdf
#
# Find the native netCDF includes and library
# This module defines
#  netCDF_INCLUDE_DIRS, where to find netcdf.h.
#  netCDF_LIBRARIES, libraries to link against to use netCDF.
#  netCDF_FOUND, If false, do not try to use netCDF.
#    also defined, but not for general use are
#  netCDF_LIBRARY, where to find the netCDF library.
#  netCDF_LIBRARY_DEBUG - Debug version of netCDF library
#  netCDF_LIBRARY_RELEASE - Release Version of netCDF library

# set (FIND_netCDF_DEBUG TRUE)

# MESSAGE (STATUS "Finding netCDF library and headers..." )

############################################
#
# Check the existence of the libraries.
#
############################################
#########################################################################

MACRO (netCDF_ADJUST_LIB_VARS basename)
  IF (${basename}_INCLUDE_DIR)

    # if only the release version was found, set the debug variable also to the release version
    IF (${basename}_LIBRARY_RELEASE AND NOT ${basename}_LIBRARY_DEBUG)
      SET (${basename}_LIBRARY_DEBUG ${${basename}_LIBRARY_RELEASE})
      SET (${basename}_LIBRARY       ${${basename}_LIBRARY_RELEASE})
      SET (${basename}_LIBRARIES     ${${basename}_LIBRARY_RELEASE})
    ENDIF (${basename}_LIBRARY_RELEASE AND NOT ${basename}_LIBRARY_DEBUG)

    # if only the debug version was found, set the release variable also to the debug version
    IF (${basename}_LIBRARY_DEBUG AND NOT ${basename}_LIBRARY_RELEASE)
      SET (${basename}_LIBRARY_RELEASE ${${basename}_LIBRARY_DEBUG})
      SET (${basename}_LIBRARY         ${${basename}_LIBRARY_DEBUG})
      SET (${basename}_LIBRARIES       ${${basename}_LIBRARY_DEBUG})
    ENDIF (${basename}_LIBRARY_DEBUG AND NOT ${basename}_LIBRARY_RELEASE)
    IF (${basename}_LIBRARY_DEBUG AND ${basename}_LIBRARY_RELEASE)
      # if the generator supports configuration types then set
      # optimized and debug libraries, or if the CMAKE_BUILD_TYPE has a value
      IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
	SET (${basename}_LIBRARY       optimized ${${basename}_LIBRARY_RELEASE} debug ${${basename}_LIBRARY_DEBUG})
      ELSE(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
	# if there are no configuration types and CMAKE_BUILD_TYPE has no value
	# then just use the release libraries
	SET (${basename}_LIBRARY       ${${basename}_LIBRARY_RELEASE} )
      ENDIF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
      SET (${basename}_LIBRARIES       optimized ${${basename}_LIBRARY_RELEASE} debug ${${basename}_LIBRARY_DEBUG})
    ENDIF (${basename}_LIBRARY_DEBUG AND ${basename}_LIBRARY_RELEASE)

    SET (${basename}_LIBRARY ${${basename}_LIBRARY} CACHE FILEPATH "The ${basename} library")

    IF (${basename}_LIBRARY)
      SET (${basename}_FOUND 1)
    ENDIF (${basename}_LIBRARY)

  ENDIF (${basename}_INCLUDE_DIR )

  # Make variables changeble to the advanced user
  MARK_AS_ADVANCED (${basename}_LIBRARY ${basename}_LIBRARY_RELEASE ${basename}_LIBRARY_DEBUG ${basename}_INCLUDE_DIR )
ENDMACRO (netCDF_ADJUST_LIB_VARS)


# Try to find netCDF using an installed netCDF-config.cmake
if( NOT netCDF_FOUND )
    if (FIND_netCDF_DEBUG)
      message ("Looking for netCDF config.")
    endif()
    find_package( netCDF QUIET NO_MODULE )
endif()

# Try to find netCDF manually
if( NOT netCDF_FOUND )

  if (FIND_netCDF_DEBUG)
    message ("Looking for netCDF include and libraries.")
  endif()

  # Need HDF5
  find_package( HDF5 COMPONENTS C HL REQUIRED )
  mark_as_advanced (HDF5_DIR)

  # Look for the header file.
  SET (netCDF_INCLUDE_SEARCH_DIRS
      $ENV{NETCDF_ROOT}/include
      $ENV{NETCDF_ROOT}/include/netcdf
      /usr/include
      /usr/include/netcdf
  )

  SET (netCDF_LIB_SEARCH_DIRS
      $ENV{NETCDF_ROOT}/lib
      /usr/lib
  )

  SET (netCDF_BIN_SEARCH_DIRS
      $ENV{NETCDF_ROOT}/bin
      /usr/bin
  )

  FIND_PATH (netCDF_INCLUDE_DIR
      NAMES netcdf.h
      PATHS ${netCDF_INCLUDE_SEARCH_DIRS}
  )

  IF (WIN32 AND NOT MINGW)
      SET (netCDF_SEARCH_DEBUG_NAMES "libnetcdf_d;netcdf_d")
      SET (netCDF_SEARCH_RELEASE_NAMES "netcdf;libnetcdf")
  ELSE (WIN32 AND NOT MINGW)
      SET (netCDF_SEARCH_DEBUG_NAMES "netcdf_d")
      SET (netCDF_SEARCH_RELEASE_NAMES "netcdf")
  ENDIF (WIN32 AND NOT MINGW)

  # Look for the library.
  FIND_LIBRARY (netCDF_LIBRARY_DEBUG
      NAMES ${netCDF_SEARCH_DEBUG_NAMES}
      PATHS ${netCDF_LIB_SEARCH_DIRS}
  )

  FIND_LIBRARY (netCDF_LIBRARY_RELEASE
      NAMES ${netCDF_SEARCH_RELEASE_NAMES}
      PATHS ${netCDF_LIB_SEARCH_DIRS}
  )

  netCDF_ADJUST_LIB_VARS (netCDF)

  IF (netCDF_INCLUDE_DIR AND netCDF_LIBRARY)
    SET (netCDF_FOUND 1)
    SET (netCDF_LIBRARIES ${netCDF_LIBRARY})
    SET (netCDF_INCLUDE_DIRS ${netCDF_INCLUDE_DIR})
    IF (netCDF_LIBRARY_DEBUG)
      GET_FILENAME_COMPONENT (netCDF_LIBRARY_PATH ${netCDF_LIBRARY_DEBUG} PATH)
      SET (netCDF_LIB_DIR  ${netCDF_LIBRARY_PATH})
    ELSEIF (netCDF_LIBRARY_RELEASE)
      GET_FILENAME_COMPONENT (netCDF_LIBRARY_PATH ${netCDF_LIBRARY_RELEASE} PATH)
      SET (netCDF_LIB_DIR  ${netCDF_LIBRARY_PATH})
    ENDIF (netCDF_LIBRARY_DEBUG)

  ELSE (netCDF_INCLUDE_DIR AND netCDF_LIBRARY)
    SET (netCDF_FOUND 0)
    SET (netCDF_LIBRARIES)
    SET (netCDF_INCLUDE_DIRS)
  ENDIF (netCDF_INCLUDE_DIR AND netCDF_LIBRARY)

  # Report the results.
  IF (NOT netCDF_FOUND)
    SET (netCDF_DIR_MESSAGE
        "netCDF was not found. Make sure netCDF_LIBRARY and netCDF_INCLUDE_DIR are set or set the NETCDF_ROOT environment variable."
    )
    IF (NOT netCDF_FIND_QUIETLY)
      MESSAGE (STATUS "${netCDF_DIR_MESSAGE}")
    ELSE (NOT netCDF_FIND_QUIETLY)
      IF (netCDF_FIND_REQUIRED)
        MESSAGE (FATAL_ERROR "netCDF was NOT found and is required by this project")
      ENDIF (netCDF_FIND_REQUIRED)
    ENDIF (NOT netCDF_FIND_QUIETLY)
  ENDIF (NOT netCDF_FOUND)

  IF (netCDF_FOUND)
    INCLUDE (CheckSymbolExists)
    #############################################
    # Find out if netCDF was build using dll's
    #############################################
    # Save required variable
    SET (CMAKE_REQUIRED_INCLUDES_SAVE ${CMAKE_REQUIRED_INCLUDES})
    SET (CMAKE_REQUIRED_FLAGS_SAVE    ${CMAKE_REQUIRED_FLAGS})
    # Add netCDF_INCLUDE_DIR to CMAKE_REQUIRED_INCLUDES
    SET (CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${netCDF_INCLUDE_DIRS}")

  #  CHECK_SYMBOL_EXISTS (netCDF_BUILT_AS_DYNAMIC_LIB "SZconfig.h" HAVE_netCDF_DLL)

  #  IF (HAVE_netCDF_DLL STREQUAL "TRUE")
  #    SET (HAVE_netCDF_DLL "1")
  #  ENDIF (HAVE_netCDF_DLL STREQUAL "TRUE")

    # Restore CMAKE_REQUIRED_INCLUDES and CMAKE_REQUIRED_FLAGS variables
    SET (CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVE})
    SET (CMAKE_REQUIRED_FLAGS    ${CMAKE_REQUIRED_FLAGS_SAVE})
    #
    #############################################
  ENDIF (netCDF_FOUND)

  IF (FIND_netCDF_DEBUG)
    MESSAGE ("netCDF_INCLUDE_DIR: ${netCDF_INCLUDE_DIR}")
    MESSAGE ("netCDF_INCLUDE_DIRS: ${netCDF_INCLUDE_DIRS}")
    MESSAGE ("netCDF_LIBRARY_DEBUG: ${netCDF_LIBRARY_DEBUG}")
    MESSAGE ("netCDF_LIBRARY_RELEASE: ${netCDF_LIBRARY_RELEASE}")
    MESSAGE ("netCDF_LIBRARY: ${netCDF_LIBRARY}")
    MESSAGE ("netCDF_LIBRARIES: ${netCDF_LIBRARIES}")
    MESSAGE ("HAVE_netCDF_DLL: ${HAVE_netCDF_DLL}")
    MESSAGE ("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
    MESSAGE ("HDF5_LIBRARIES: ${HDF5_LIBRARIES}")
  ENDIF (FIND_netCDF_DEBUG)


  #############################################
  # Set up targets

  cmake_policy(PUSH)
  cmake_policy(VERSION 2.6)
  # Commands may need to know the format version.
  set(CMAKE_IMPORT_FILE_VERSION 1)

  # Create imported target netcdf
  if(NOT TARGET netcdf)
      add_library(netcdf UNKNOWN IMPORTED)
  endif()

  set_target_properties(netcdf PROPERTIES
    INTERFACE_LINK_LIBRARIES "${HDF5_LIBRARIES}"
  )

  # Import target "netcdf" for configuration "Release"
  if (netCDF_LIBRARY_RELEASE)
      set_property(TARGET netcdf APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
      set_property(TARGET netcdf PROPERTY IMPORTED_LOCATION_RELEASE ${netCDF_LIBRARY_RELEASE})
#      GET_FILENAME_COMPONENT (netCDF_SONAME_RELEASE ${netCDF_LIBRARY_RELEASE} NAME)
#      set_property(TARGET netcdf PROPERTY IMPORTED_SONAME_RELEASE ${netCDF_SONAME_RELEASE})
  endif()
  # Import target "netcdf" for configuration "Debug"
  if (netCDF_LIBRARY_DEBUG)
      set_property(TARGET netcdf APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
      set_property(TARGET netcdf PROPERTY IMPORTED_LOCATION_DEBUG ${netCDF_LIBRARY_DEBUG})
#      GET_FILENAME_COMPONENT (netCDF_SONAME_DEBUG ${netCDF_LIBRARY_DEBUG} NAME)
#      set_property(TARGET netcdf PROPERTY IMPORTED_SONAME_RELEASE ${netCDF_SONAME_DEBUG})
  endif()

  set(CMAKE_IMPORT_FILE_VERSION)
  cmake_policy(POP)

endif()
