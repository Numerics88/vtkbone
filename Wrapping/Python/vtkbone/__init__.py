from __future__ import absolute_import

# On some systems, not manually importing vtk before vtkbonePython
# results in a library error.
import vtk

try:
    from .vtkbonePython import *
except ImportError:
    from vtkbonePython import *
