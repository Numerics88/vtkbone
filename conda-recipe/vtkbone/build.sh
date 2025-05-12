set -x

# Deactivate any existing conda environment to avoid mismatches with build config
# Without this, the build fails for MacOS-13 runner
# conda deactivate

# Create build directory
mkdir -p build
cd build
BUILD_CONFIG=Release

# Specify Python
# PYTHON_INCLUDE_DIR=$(python -c 'import sysconfig;print("{0}".format(sysconfig.get_path("platinclude")))')
# PYTHON_LIBRARY=$(python -c 'import sysconfig;print("{0}/{1}".format(*map(sysconfig.get_config_var, ("LIBDIR", "LDLIBRARY"))))')

# OS specifics
declare -a CMAKE_PLATFORM_FLAGS
case $(uname | tr '[:upper:]' '[:lower:]') in
  linux*)
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
	-DPython_FIND_STRATEGY="LOCATION" \
	-DPython_ROOT_DIR:PATH="${PREFIX}" \
	-DPYTHON_LIBRARY:PATH="${PYTHON_LIBRARY}" \
	-DPYTHON_INCLUDE_DIR:PATH="${PYTHON_INCLUDE_DIR}" \
	"${CMAKE_PLATFORM_FLAGS[@]}"


echo "Macosx deployment target: ${MACOSX_DEPLOYMENT_TARGET}"
# Compile and install
ninja install -v

# conda activate

# Run tests
nosetests ${SRC_DIR}/Testing/Python