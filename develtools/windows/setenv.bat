@echo off
REM This is an example setenv.bat for running the faim software from the build
REM directory.
REM
REM To use:
REM  - copy it to your build directory
REM  - modify the paths below to correspond to your system
REM  - run "setenv"

REM If VTK is not already on the path, then you will need a line like the following.
REM If using Anaconda Python, this is unnecessary.
REM call "C:\Users\Eric\Install\VTK-6.3.0\setenv.bat"

REM Note that SCRIPT_DIR seems to end with a trailing \
set SCRIPT_DIR=%~dp0

echo Setting VTKBONE_DIR=%SCRIPT_DIR%
set VTKBONE_DIR=%SCRIPT_DIR%

echo Adding to PATH: %VTKBONE_DIR%Release
set PATH=%VTKBONE_DIR%Release;%PATH%

echo Adding to PYTHONPATH: %VTKBONE_DIR%Wrapping\Python;%VTKBONE_DIR%Release
set PYTHONPATH=%VTKBONE_DIR%Wrapping\Python;%VTKBONE_DIR%Release;%PYTHONPATH%
