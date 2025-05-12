
# PYTHON_PACKAGE_DIR=`python -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())"`

# echo "Python package library is $PYTHON_PACKAGE_DIR"

# FIXLIB="${PYTHON_PACKAGE_DIR}"/vtkbone/vtkbonePython.so
# echo "Modifying rpath and libraries of ${FIXLIB}"
# install_name_tool -add_rpath '@executable_path/../lib' "${FIXLIB}"
# install_name_tool -change libhdf5.10.dylib @rpath/libhdf5.10.dylib "${FIXLIB}"
# install_name_tool -change libhdf5_hl.10.dylib @rpath/libhdf5_hl.10.dylib "${FIXLIB}"
# install_name_tool -change libpython2.7.dylib '@executable_path/../lib/libpython2.7.dylib' "${FIXLIB}"

# FIXLIB="${PYTHON_PACKAGE_DIR}"/../../libvtkbonePythonD.dylib
# echo "Modifying rpath and libraries of ${FIXLIB}"
# install_name_tool -add_rpath '@executable_path/../lib' "${FIXLIB}"
# install_name_tool -change libhdf5.10.dylib @rpath/libhdf5.10.dylib "${FIXLIB}" 
# install_name_tool -change libhdf5_hl.10.dylib @rpath/libhdf5_hl.10.dylib "${FIXLIB}"
# install_name_tool -change libpython2.7.dylib '@executable_path/../lib/libpython2.7.dylib' "${FIXLIB}"

# FIXLIB="${PYTHON_PACKAGE_DIR}"/../../libvtkbone.dylib
# echo "Modifying rpath and libraries of ${FIXLIB}"
# install_name_tool -add_rpath '@executable_path/../lib' "${FIXLIB}"
# install_name_tool -change libhdf5.10.dylib @rpath/libhdf5.10.dylib "${FIXLIB}"
# install_name_tool -change libhdf5_hl.10.dylib @rpath/libhdf5_hl.10.dylib "${FIXLIB}"
