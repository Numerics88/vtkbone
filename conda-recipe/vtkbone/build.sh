set -x

# Create build directory
mkdir -p build
cd build
BUILD_CONFIG=Release

# Specify Python
PYTHON_INCLUDE_DIR=$(python -c 'import sysconfig;print("{0}".format(sysconfig.get_path("platinclude")))')
# PYTHON_LIBRARY=$(python -c 'import sysconfig;print("{0}/{1}".format(*map(sysconfig.get_config_var, ("LIBDIR", "LDLIBRARY"))))')
PYTHON_LIBRARY=$(python -c '
import sysconfig
libdir = sysconfig.get_config_var("LIBDIR")
ldlib = sysconfig.get_config_var("LDLIBRARY")
if libdir and ldlib:
    print(f"{libdir}/{ldlib}")
else:
    print("")
')
PYTHON_SITE_PACKAGES="${PREFIX}/lib/python${PY_VER}/site-packages"
# Fallback to shared library if static library is missing
if [[ ! -f "${PYTHON_LIBRARY}" ]]; then
    PYTHON_LIBRARY="${BUILD_PREFIX}/lib/libpython${PY_VER}.dylib"
fi

# OS specifics
declare -a CMAKE_PLATFORM_FLAGS
case $(uname | tr '[:upper:]' '[:lower:]') in
  linux*)
		# (Old workaround for VTK 8.2.0)
		# See crazy vtk hacks here: https://github.com/conda-forge/vtk-feedstock/issues/86
		# sed -i '/vtkhdf5_LIBRARIES/d' $BUILD_PREFIX/lib/cmake/vtk-8.2/Modules/vtkhdf5.cmake

		# Environment variables for nosetests
		export LD_LIBRARY_PATH="${PREFIX}/lib:${LD_LIBRARY_PATH}"
		export PYTHONPATH="${PREFIX}/lib/python${PY_VER}/site-packages/:${PYTHONPATH}"
    ;;
  darwin*)
		# Get the SDK
		CMAKE_PLATFORM_FLAGS+=(-DCMAKE_OSX_SYSROOT:PATH="${CONDA_BUILD_SYSROOT}")
		CMAKE_PLATFORM_FLAGS+=(-DCMAKE_SKIP_BUILD_RPATH:BOOL=OFF)
		CMAKE_PLATFORM_FLAGS+=(-DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON)
		CMAKE_PLATFORM_FLAGS+=(-DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOl=ON)

		# Environment variables for nosetests
		export DYLD_FALLBACK_LIBRARY_PATH="${PREFIX}/lib/:${DYLD_FALLBACK_LIBRARY_PATH}"
		export PYTHONPATH="${PREFIX}/lib/python${PY_VER}/site-packages/:${PYTHONPATH}"
    ;;
  *)
esac

# CMake
cmake .. \
    -G "Ninja" \
    -DCMAKE_BUILD_TYPE=$BUILD_CONFIG \
    -DCMAKE_PREFIX_PATH:PATH="${PREFIX}" \
    -DCMAKE_INSTALL_PREFIX:PATH="${PREFIX}" \
    -DCMAKE_INSTALL_RPATH:PATH="${PREFIX}/lib" \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DBOOST_ROOT:PATH="${PREFIX}" \
    -DCMAKE_MODULE_PATH:PATH="${SRC_DIR}/cmake/modules" \
    -DENABLE_TESTING:BOOL=ON \
    -DVTKBONE_WRAP_PYTHON=ON \
    -DVTK_DIR:PATH="${BUILD}/lib/cmake/vtk-9.0" \
    -DPython3_EXECUTABLE:FILEPATH="$(which python)" \
    -DPython3_LIBRARY:FILEPATH="${PYTHON_LIBRARY}" \
    -DPython3_INCLUDE_DIR:PATH="${PYTHON_INCLUDE_DIR}" \
    -DPython3_ROOT_DIR:PATH="${PREFIX}" \
    -DPython3_FIND_STRATEGY="LOCATION" \
    -DPYTHON_SITE_PACKAGES:PATH="${PYTHON_SITE_PACKAGES}" \
    "${CMAKE_PLATFORM_FLAGS[@]}"

# Compile and install
ninja install -v

# Print site_packages folder
echo "PYTHONPATH: ${PYTHONPATH}"
echo "PREFIX: ${PREFIX}"
echo "PYTHON_SITE_PACKAGES: ${PREFIX}/lib/python${PY_VER}/site-packages"
echo "PYTHON_INCLUDE_DIR: ${PYTHON_INCLUDE_DIR}"

# Run tests
ctest --output-on-failure
# nosetests ${SRC_DIR}/Testing/Python

# rename vtkbone.cpython-*.so to vtkbone.so
site_packages_dir="${PREFIX}/lib/python${PY_VER}/site-packages/vtkbone"
ls -l ${site_packages_dir}
cd "$site_packages_dir"

for f in vtkbone.cpython-*.so; do
    if [[ -f "$f" && ! -f vtkbone.so ]]; then
        mv "$f" vtkbone.so
        echo "Renamed $f to vtkbone.so"
        ls -l ${site_packages_dir}
    fi
done
