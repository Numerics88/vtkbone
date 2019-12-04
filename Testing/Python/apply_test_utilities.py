from __future__ import division
from numpy.core import *
import vtk
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk


def compare_constraint (constraint, expected_indices, expected_senses,
                        expected_values, debug=0):
    """Compares constraint with expected values.

    The order of the constraints and the expected values may differ.
   
    Returns 1 if they match, 0 otherwise.
    """

    # Sort expected values by index, senses, value
    sort_indices = lexsort((expected_values, expected_senses, expected_indices, ))
    expected_indices = expected_indices[sort_indices]
    expected_senses = expected_senses[sort_indices]
    expected_values = expected_values[sort_indices]
    
    # Extract arrays from constraint
    indices_vtk = constraint.GetIndices()
    indices = vtk_to_numpy(indices_vtk)
    senses_vtk = constraint.GetAttributes().GetArray("SENSE")
    assert(not senses_vtk is None)
    senses = vtk_to_numpy(senses_vtk)
    values_vtk = constraint.GetAttributes().GetArray("VALUE")
    assert(not values_vtk is None)
    values = vtk_to_numpy(values_vtk)

    # Sort constraint values by index, senses, value
    sort_indices = lexsort((values, senses, indices))
    indices = indices[sort_indices]
    senses = senses[sort_indices]
    values = values[sort_indices]

    if debug:
        print(expected_indices)
        print(expected_senses)
        print(expected_values)
        print(indices)
        print(senses)
        print(values)

    return (alltrue(indices == expected_indices) 
            and alltrue(senses == expected_senses) 
            and alltrue(abs(values - expected_values) < 1E-7))
