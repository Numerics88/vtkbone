@echo on

:: Create build directory
mkdir build
cd build
set BUILD_CONFIG=Release

:: Set Python paths explicitly to use conda's Python
set PYTHON_EXECUTABLE=%PREFIX%\python.exe
set PYTHON_INCLUDE_DIR=%PREFIX%\include
set PYTHON_LIBRARY=%PREFIX%\libs\python%CONDA_PY%.lib

:: CMake
cmake .. ^
	-G "Ninja" ^
	-DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
	-DCMAKE_INSTALL_PREFIX:PATH="%LIBRARY_PREFIX%" ^
	-DBOOST_ROOT:PATH="%PREFIX%" ^
	-DBUILD_SHARED_LIBS:BOOL=ON ^
	-DENABLE_TESTING:BOOL=ON ^
	-DVTKBONE_WRAP_PYTHON:BOOL=ON ^
	-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS:BOOL=OFF ^
	-DPython3_EXECUTABLE:FILEPATH="%PYTHON_EXECUTABLE%" ^
	-DPython3_INCLUDE_DIRS:PATH="%PYTHON_INCLUDE_DIR%" ^
	-DPython3_LIBRARY:PATH="%PYTHON_LIBRARY%"

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
if not exist "%PREFIX%\Lib\site-packages\vtkbone\vtkbone.pyd" (
	echo "Error: vtkbone.pyd was not moved correctly."
	exit /b 1
) else (
	echo "vtkbone.pyd moved successfully."
)

if errorlevel 1 exit 1

:: Use CTEST to run the tests
ctest --output-on-failure

if errorlevel 1 exit 1
