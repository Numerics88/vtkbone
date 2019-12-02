
option (VTKBONE_WRAP_PYTHON
    "Wrap classes into the Python interpreted language."
    ON
)

macro(vtkbone_wrap_python library_name WRAP_SRCS)
    # Need VTK at this point
    if(NOT DEFINED VTK_CMAKE_DIR)
        message(SEND_ERROR "Cannot find VTK_CMAKE_DIR to load vtkWrapPython.cmake")
    endif()

    # Source VTK wrapping package
    SET(VTK_WRAP_PYTHON_FIND_LIBS ON)
    include (${VTK_CMAKE_DIR}/vtkWrapPython.cmake)
    if (WIN32)
        if (NOT BUILD_SHARED_LIBS)
            message (FATAL_ERROR "Python support requires BUILD_SHARED_LIBS to be ON.")
            set (VTKBONE_CAN_BUILD 0)
        endif ()
    endif ()

    # VTK must also be wrapped
    if (NOT VTK_WRAP_PYTHON)
        message ("Warning. VTKBONE_WRAP_PYTHON is ON but the VTK version you have "
                "chosen has not support for Python (VTK_WRAP_PYTHON is OFF).  "
                "Please set VTKBONE_WRAP_PYTHON to OFF.")
        set (VTKBONE_WRAP_PYTHON OFF)
    endif ()

    if (VTKBONE_WRAP_PYTHON)
        # Find python
        find_package(PythonInterp REQUIRED)
        find_package(PythonLibs REQUIRED)
        include_directories(${PYTHON_INCLUDE_DIRS})

        # Determine site-packages for python install
        if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
            set(SITE_PACKAGES "Lib/site-packages")
        else ()
            set(SITE_PACKAGES "lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages")
        endif()

        # Create all wrapped sources
        # Need this custom target for 'make' but not 'ninja'
        add_custom_target(${library_name}Hierarchy)
        vtk_wrap_python3 (${library_name}Python PYTHON_SRCS "${WRAP_SRCS}")

        foreach(c ${VTK_MODULES_REQUESTED})
            set (VTK_PYTHOND_LIBRARIES ${VTK_PYTHOND_LIBRARIES} ${c}PythonD)
        endforeach(c)

        # Create lib${library_name}PythonD
        add_library (${library_name}PythonD ${PYTHON_SRCS})
        target_link_libraries(${library_name}PythonD
            ${library_name}
            ${PYTHON_LIBRARIES}
            ${VTK_PYTHOND_LIBRARIES}
        )

        # Create lib${library_name}Python
        add_library (${library_name}Python MODULE ${library_name}PythonInit.cxx)
        target_link_libraries(${library_name}Python
            ${library_name}PythonD
            ${PYTHON_LIBRARIES}
        )
        set_target_properties(${library_name}Python PROPERTIES PREFIX "")
        if (WIN32 AND NOT CYGWIN)
            set_target_properties (${library_name}Python PROPERTIES SUFFIX ".pyd")
        endif ()

        # Install
        install (TARGETS ${library_name}PythonD
                EXPORT ${library_name}Targets
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)
        install (TARGETS ${library_name}Python
                EXPORT ${library_name}Targets
                RUNTIME DESTINATION "${SITE_PACKAGES}/${library_name}"
                LIBRARY DESTINATION "${SITE_PACKAGES}/${library_name}"
                ARCHIVE DESTINATION "${SITE_PACKAGES}/${library_name}")
    endif()
endmacro ()
