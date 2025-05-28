@echo on

:: Create build directory
mkdir build
cd build
set BUILD_CONFIG=Release

:: Remove MinGW from PATH to force MSVC usage
set "PATH=%PATH:C:\mingw64\bin;=%"
:: Set PATHS:
set PATH=%PATH%;%PREFIX%;%PREFIX%\\Scripts;%PREFIX%\\Library;%PREFIX%\\Library\\bin;%PREFIX%\\Lib;%PREFIX%\\include;;%PREFIX%\\Lib\\site-packages;%PREFIX%\\libs
:: Set environemnt variables for nosetests
set PATH=%PATH%;%PREFIX%\\Library\\lib;%PREFIX%\\Library\\bin
set PYTHONPATH=%PYTHONPATH%;%PREFIX%\\Lib\\site-packages;%PREFIX%\\libs

where python
where cmake
where ninja

:: print Python Library directory contents
echo %PREFIX%
dir "%PREFIX%\\Lib" /b /s
dir "%PREFIX%\\libs" /b /s

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
	-DPython3_EXECUTABLE:FILEPATH="%PYTHON%"
	@REM -DPython3_ROOT_DIR:PATH="%PREFIX%" ^
	@REM -DPython3_LIBRARY="%PREFIX%\\python3.dll" ^
    @REM -DPython3_INCLUDE_DIR="%PREFIX%\\Include"
	:: -DPython3_LIBRARY:FILEPATH="%PREFIX%\\libs\\python3.lib" ^

:: Print out build logs

type "%PREFIX%\\..\\work\\build\\CMakeFiles\\CMakeOutput.log"

if errorlevel 1 exit 1



:: Compile and install
ninja install -v
:: if errorlevel 1 exit 1

:: Run tests
::    Note that for >=py3.8, DLL look up no longer goes through `PATH`.
::    This is a hack to make nose run by adding dll's inside python
:: python %RECIPE_DIR%/test_windows.py %SRC_DIR%/Testing/Python

if errorlevel 1 exit 1
