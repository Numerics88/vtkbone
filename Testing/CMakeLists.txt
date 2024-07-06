
add_subdirectory(Cxx)

if (VTKBONE_WRAP_PYTHON)
  add_subdirectory(Python)
endif (VTKBONE_WRAP_PYTHON)
