@echo on

:: Create build directory
mkdir build
cd build
set BUILD_CONFIG=Release

:: CMake
cmake .. ^
	-G "Ninja" ^
	-DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
	-DCMAKE_INSTALL_PREFIX:PATH="%LIBRARY_PREFIX%" ^
	-DBOOST_ROOT:PATH="%PREFIX%" ^
	-DENABLE_TESTING:BOOL=ON ^
	-DCMAKE_MODULE_PATH:PATH="%SRC_DIR%\cmake\modules" ^
	-DBUILD_SHARED_LIBS:BOOL=ON
if errorlevel 1 exit 1

:: Compile and install
ninja install
if errorlevel 1 exit 1

:: Run tests
::ctest -V
::if errorlevel 1 exit 1
