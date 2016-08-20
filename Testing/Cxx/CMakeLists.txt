# Commented out, as all tests are currently written in Python.

# # Create a test list : everything into one static binary.
# create_test_sourcelist (Tests vtkboneCxxTests.cxx
#   TestCxxDummy.cxx
# )
# 
# add_executable (vtkboneCxxTests ${Tests})
# set_target_properties (vtkboneCxxTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
# target_link_libraries (vtkboneCxxTests vtkbone)
# 
# set (TestsToRun ${Tests})
# remove (TestsToRun vtkboneCxxTests.cxx)
# 
# # Add all the executables
# foreach (test ${TestsToRun})
#   get_filename_component (TName ${test} NAME_WE)
#   add_test (${TName} ${CMAKE_CURRENT_BINARY_DIR}/vtkboneCxxTests ${TName})
# endforeach (test)
