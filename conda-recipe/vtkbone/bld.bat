@echo on

mkdir build
cd build
set BUILD_CONFIG=Release

cmake .. ^
	-G "Ninja" ^
	-DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
	-DCMAKE_PREFIX_PATH="%PREFIX%" ^
	-DCMAKE_INSTALL_PREFIX="%LIBRARY_PREFIX%" ^
	-DCMAKE_INSTALL_RPATH="%PREFIX%/lib" ^
	-DBOOST_ROOT="%PREFIX%" ^
	-DENABLE_TESTING=ON ^
	-DPython_EXECUTABLE="%PYTHON%" ^
	-DVTKBONE_WRAP_PYTHON=ON ^
	-DVTKBONE_WRAP_PYTHON3=ON ^
	-DCMAKE_MODULE_PATH="%SRC_DIR%/cmake/modules" ^
	-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=OFF ^
	-DBUILD_SHARED_LIBS=ON
if errorlevel 1 exit 1

ninja install -v
if errorlevel 1 exit 1

set PATH=%PREFIX%\Library\bin;%PATH%
set PYTHONPATH=%PREFIX%\Lib\site-packages

:: Run Python tests
@REM python %RECIPE_DIR%\test_windows.py %SRC_DIR%\Testing\Python
@REM if errorlevel 1 exit 1
