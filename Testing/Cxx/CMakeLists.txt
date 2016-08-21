include_directories ("${GTEST_INCLUDE_DIR}")
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/../../Source")

add_executable (vtkboneTests
    TestStressStrainMatrix.cxx
    ../../Source/vtkboneStressStrainMatrix.cxx
    )

target_link_libraries (vtkboneTests
    ${VTK_LIBRARIES}
    ${GTEST_BOTH_LIBRARIES})

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries (vtkboneTests pthread)
endif (CMAKE_SYSTEM_NAME STREQUAL "Linux")

add_test (NAME vtkboneTests COMMAND $<TARGET_FILE:vtkboneTests>)
