
# Create build directory
mkdir build
cd build
BUILD_CONFIG=Release

# Specify Python
case $(uname | tr '[:upper:]' '[:lower:]') in
  linux*)
		export PYTHON_LIBRARY="${PREFIX}/lib/libpython${PY_VER}.so"
		export PYTHON_INCLUDE_DIR="${PREFIX}/include/python${PY_VER}"
    ;;
  darwin*)
		export PYTHON_LIBRARY="${PREFIX}/lib/libpython${PY_VER}.dylib"
		export PYTHON_INCLUDE_DIR="${PREFIX}/include/python${PY_VER}"
    ;;
  *)
esac

# C/CXX Flags
export CFLAGS="-fPIC"
export CXXFLAGS="-fPIC"

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
	-DENABLE_TESTING:BOOL=ON

# Compile and install
ninja install -v

# Run tests
ctest -V
