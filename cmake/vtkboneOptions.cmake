
# Ensure that CMake behaves predictably
set (CMAKE_EXPORT_NO_PACKAGE_REGISTRY ON)
set (CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY ON)
set (CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY ON)

# Macro to replace the extension of a file name
# Is there a standard way to do this?
macro (REPLACE_EXTENSION outpath inpath extension)
    get_filename_component (inpath_PATH ${inpath} PATH)
    get_filename_component (inpath_NAME_WE ${inpath} NAME_WE)
    file (TO_CMAKE_PATH "${inpath_PATH}/${inpath_NAME_WE}.${extension}" ${outpath})
endmacro (REPLACE_EXTENSION)

# Disable auto-linking for boost, as it causes endless grief
if (WIN32)
    add_definitions (-DBOOST_ALL_NO_LIB)
endif ()

# Libraries should be built into lib
if (NOT WIN32)
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif ()

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
