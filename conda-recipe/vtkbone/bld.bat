@echo on

:: Create build directory
mkdir build
cd build
set BUILD_CONFIG=Release

:: CMake
cmake .. ^
	-G "Ninja" ^
	-DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
	-DCMAKE_PREFIX_PATH:PATH="%PREFIX%" ^
	-DCMAKE_INSTALL_PREFIX:PATH="%LIBRARY_PREFIX%" ^
	-DCMAKE_INSTALL_RPATH:PATH="%PREFIX%\\lib" ^
	-DENABLE_TESTING:BOOL=ON ^
	-DVTKBONE_WRAP_PYTHON:BOOL=ON ^
	-DCMAKE_MODULE_PATH:PATH="%SRC_DIR%\\cmake\\modules" ^
	-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS:BOOL=OFF ^
	-DBUILD_SHARED_LIBS:BOOL=ON
	-DPython3_EXECUTABLE:FILEPATH="%PYTHON%" ^

if errorlevel 1 exit 1

:: Compile and install
ninja install -v
if errorlevel 1 exit 1

:: Set environemnt variables for nosetests
set PATH=%PATH%;%PREFIX%\\Library\\lib;%PREFIX%\\Library\\bin
set PYTHONPATH=%PYTHONPATH%;%PREFIX%\\Lib\\site-packages

:: Run tests
::    Note that for >=py3.8, DLL look up no longer goes through `PATH`.
::    This is a hack to make nose run by adding dll's inside python
@REM python %RECIPE_DIR%/test_windows.py %SRC_DIR%/Testing/Python

if errorlevel 1 exit 1
