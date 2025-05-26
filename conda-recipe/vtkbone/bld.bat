@echo on

:: Create build directory
mkdir build
cd build
set BUILD_CONFIG=Release

@REM Print environment variables
echo DEBUG Printing environment variables
echo BUILD_PREFIX=%BUILD_PREFIX%
echo BUILD_CONFIG=%BUILD_CONFIG%
echo PREFIX=%PREFIX%
echo PYTHON=%PYTHON%
echo PYTHON_LIBRARY=%PYTHON_LIBRARY%
echo PYTHON_INCLUDE_DIR=%PYTHON_INCLUDE_DIR%
echo PYTHONPATH=%PYTHONPATH%
echo SRC_DIR=%SRC_DIR%
echo LIBRARY_PREFIX=%LIBRARY_PREFIX%

:: Debug: Find python file path:
echo DEBUG Python executable: %PYTHON%

:: CMake
cmake .. ^
	-G "Ninja" ^
	-DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
	-DCMAKE_PREFIX_PATH:PATH="%PREFIX%" ^
	-DCMAKE_INSTALL_PREFIX:PATH="%LIBRARY_PREFIX%" ^
	-DCMAKE_INSTALL_RPATH:PATH="%PREFIX%\\Lib" ^
	-DBUILD_SHARED_LIBS:BOOL=ON ^
	-DCMAKE_MODULE_PATH:PATH="%SRC_DIR%\\cmake\\modules" ^
	-DENABLE_TESTING:BOOL=ON ^
	-DVTKBONE_WRAP_PYTHON:BOOL=ON ^
	-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS:BOOL=OFF ^
	-DPython3_EXECUTABLE:FILEPATH="%PYTHON%"^
	-DPython3_ROOT_DIR:PATH="%PREFIX%" ^
	-DPython3_FIND_STRATEGY="LOCATION" ^
	-DPython3_INCLUDE_DIR:PATH="%PREFIX%\\include" ^
	-DPython3_LIBRARY="%PREFIX%\libs\python3.lib"

@REM if errorlevel 1 exit 1

:: Compile and install
ninja install -v
@REM if errorlevel 1 exit 1

:: Set environemnt variables for nosetests
set PATH=%PATH%;%PREFIX%\\Library\\lib;%PREFIX%\\Library\\bin
set PYTHONPATH=%PYTHONPATH%;%PREFIX%\\Lib\\site-packages

:: Run tests
::    Note that for >=py3.8, DLL look up no longer goes through `PATH`.
::    This is a hack to make nose run by adding dll's inside python
@REM python %RECIPE_DIR%/test_windows.py %SRC_DIR%/Testing/Python

if errorlevel 1 exit 1
