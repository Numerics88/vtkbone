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

if errorlevel 1 exit 1

:: Compile and install
ninja install -v
if errorlevel 1 exit 1

:: Manually copy the python module to the site-packages directory
:: This is necessary because the vtkboneWrapper CMake install does not handle this correctly
if exist "%LIBRARY_PREFIX%\bin\Lib\site-packages\vtkbone\vtkbone.pyd" (
    if not exist "%PREFIX%\Lib\site-packages\vtkbone" (
        mkdir "%PREFIX%\Lib\site-packages\vtkbone"
    )
    move "%LIBRARY_PREFIX%\bin\Lib\site-packages\vtkbone\*" "%PREFIX%\Lib\site-packages\vtkbone\"
    rmdir /s /q "%LIBRARY_PREFIX%\bin\Lib\site-packages"
)

:: Verify that the files were moved correctly
if not exist "%LIBRARY_PREFIX%\lib\site-packages\vtkbone\vtkbone.pyd" (
	echo "Error: vtkbone.pyd was not moved correctly."
	exit /b 1
) else (
	echo "vtkbone.pyd moved successfully."
)
:: Run tests

:: OLD NOTES:
::    Note that for >=py3.8, DLL look up no longer goes through `PATH`.
::    This is a hack to make nose run by adding dll's inside python
:: python %RECIPE_DIR%/test_windows.py %SRC_DIR%/Testing/Python

if errorlevel 1 exit 1
