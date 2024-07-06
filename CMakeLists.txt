cmake_minimum_required (VERSION 3.12)
project (vtkbone)

# === Add cmake dir to the cmake path
set (vtkbone_cmake_dir ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set (CMAKE_MODULE_PATH ${vtkbone_cmake_dir} ${CMAKE_MODULE_PATH})

# === Versioning
include (vtkboneVersion)

# === Configure project
include (vtkboneOptions)

# === Enable Testing
# Note: must be enabled in root CMake file.
option (ENABLE_TESTING "Build unit and other tests" ON)
if (ENABLE_TESTING)
    enable_testing()
endif ()

# === Source files
add_subdirectory (Source)

# === Documentation
add_subdirectory (Utilities)

# === Testing
if (ENABLE_TESTING)
    find_package (GTest REQUIRED)
    add_subdirectory (Testing)
endif ()
