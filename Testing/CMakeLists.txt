
SUBDIRS(Cxx)

IF (VTKBONE_WRAP_PYTHON)
  SUBDIRS(Python)
ENDIF (VTKBONE_WRAP_PYTHON)
