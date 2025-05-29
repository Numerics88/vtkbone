# Create rules for Python wrapping

find_package(Python3 COMPONENTS Development)
# PATHS ${CMAKE_CURRENT_LIST_DIR}/../conda/bin
# print CMAKE_CURRENT_LIST_DIR
message(STATUS "Python3 Development: ${Python3_VERSION} (${Python3_EXECUTABLE})")
message(STATUS "CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

vtk_module_wrap_python(
    MODULES         ${vtkbone_modules}
    TARGET          vtkbone::vtkbonemodules
    INSTALL_EXPORT  vtkbonePython
    PYTHON_PACKAGE  "vtkbone"
    CMAKE_DESTINATION   "${VTKBONE_CMAKE_DESTINATION}"
    LIBRARY_DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    SOABI           "${Python3_SOABI}"
    BUILD_STATIC    OFF)

vtk_module_python_default_destination(python_destination)
string(CONCAT _content
    "import vtkmodules\n"
    "from .vtkbone import *\n\n"
    "__all__ = ['vtkbone']\n"
    "__version__ = \"${VTKBONE_VERSION}\"\n")

file(GENERATE
    OUTPUT  "${PROJECT_BINARY_DIR}/${python_destination}/vtkbone/__init__.py"
    CONTENT "${_content}")

install(
    FILES   "${PROJECT_BINARY_DIR}/${python_destination}/vtkbone/__init__.py"
    DESTINATION "${python_destination}/vtkbone/")

export(
    EXPORT    vtkbonePython
    NAMESPACE vtkbone::
    FILE      "${VTKBONE_CMAKE_DESTINATION}/vtkbonePython-targets.cmake")

install(
    EXPORT    vtkbonePython
    NAMESPACE vtkbone::
    FILE      "vtkbonePython-targets.cmake"
    DESTINATION "${VTKBONE_CMAKE_DESTINATION}")
