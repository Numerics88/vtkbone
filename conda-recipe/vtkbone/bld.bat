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
	-DBUILD_SHARED_LIBS:BOOL=ON ^
	-DENABLE_TESTING:BOOL=ON ^
	-DVTKBONE_WRAP_PYTHON:BOOL=ON ^
	-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS:BOOL=OFF

@REM -G "Ninja" ^
@REM 	-DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
@REM 	-DCMAKE_INSTALL_PREFIX:PATH="%LIBRARY_PREFIX%" ^
@REM 	-DBOOST_ROOT:PATH="%PREFIX%" ^
@REM 	-DENABLE_TESTING:BOOL=ON ^
@REM     -DPYTHON_INCLUDE_PATH:PATH="%PREFIX%\\include" ^
@REM     -DPYTHON_LIBRARY:FILEPATH="%PREFIX%\\libs\\python%PY_VER%.lib" ^
@REM 	-DPython_ROOT_DIR:PATH="${PREFIX}" ^
@REM 	-DVTKBONE_PYTHON_VERSION:STRING="%PY_VER%" ^
@REM 	-DCMAKE_MODULE_PATH:PATH="%SRC_DIR%\cmake\modules" ^
@REM 	-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS:BOOL=OFF ^
@REM 	-DBUILD_SHARED_LIBS:BOOL=ON

if errorlevel 1 exit 1

:: Compile and install
ninja install -v
:: if errorlevel 1 exit 1

:: Run tests
::    Note that for >=py3.8, DLL look up no longer goes through `PATH`.
::    This is a hack to make nose run by adding dll's inside python
:: python %RECIPE_DIR%/test_windows.py %SRC_DIR%/Testing/Python

if errorlevel 1 exit 1
