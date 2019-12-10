cmake_minimum_required (VERSION 3.12)
project (vtkbone)

# === Versioning
include (${PROJECT_SOURCE_DIR}/cmake/vtkboneVersion.cmake)

# === Configure project
include (${PROJECT_SOURCE_DIR}/cmake/vtkboneOptions.cmake)
add_subdirectory(Config)
include (${PROJECT_SOURCE_DIR}/cmake/vtkboneWrapPython.cmake)

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
