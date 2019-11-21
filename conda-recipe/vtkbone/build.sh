set -x

# Create build directory
mkdir build
cd build
BUILD_CONFIG=Release

# Specify Python
case $(uname | tr '[:upper:]' '[:lower:]') in
  linux*)
		export PYTHON_LIBRARY="${PREFIX}/lib/libpython${PY_VER}.so"
		export PYTHON_INCLUDE_DIR="${PREFIX}/include/python${PY_VER}"

		# CMAKE_PLATFORM_FLAGS+=(-DCMAKE_FIND_ROOT_PATH="${PREFIX};${BUILD_PREFIX}/${HOST}/sysroot")
    ;;
  darwin*)
		export PYTHON_LIBRARY="${PREFIX}/lib/libpython${PY_VER}.dylib"
		export PYTHON_INCLUDE_DIR="${PREFIX}/include/python${PY_VER}"

		# Get the SDK
		CMAKE_PLATFORM_FLAGS+=(-DCMAKE_OSX_SYSROOT="${CONDA_BUILD_SYSROOT}")
		export DYLD_FALLBACK_LIBRARY_PATH="${BUILD_PREFIX}/lib/:${DYLD_LIBRARY_PATH}"
    ;;
  *)
esac

echo $LD_LIBRARY_PATH
echo $PATH
conda env list


# CMake
cmake .. \
	-G "Ninja" \
	-DCMAKE_BUILD_TYPE=$BUILD_CONFIG \
	-DCMAKE_PREFIX_PATH:PATH="${PREFIX}" \
	-DCMAKE_INSTALL_PREFIX:PATH="${PREFIX}" \
	-DCMAKE_INSTALL_RPATH:PATH="${PREFIX}/lib" \
	-DBUILD_SHARED_LIBS:BOOL=ON \
	-DBOOST_ROOT:PATH="${PREFIX}" \
	-DPYTHON_LIBRARY:FILEPATH="${PYTHON_LIBRARY}" \
	-DPYTHON_INCLUDE_DIR:PATH="${PYTHON_INCLUDE_DIR}" \
	-DENABLE_TESTING:BOOL=ON \
	"${CMAKE_PLATFORM_FLAGS[@]}"

# Compile and install
ninja install -v

# Run tests
ctest -V
