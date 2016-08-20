C:\Users\Eric\Install\VTK-6.3.0\setenv.ps1

Write-Host "Setting VTKBONE_DIR = $PSScriptRoot"
$env:VTKBONE_DIR = $PSScriptRoot

Write-Host "Adding to PATH: $env:VTKBONE_DIR\Release"
$env:PATH = "$env:VTKBONE_DIR\Release;$env:PATH"

Write-Host "Adding to PYTHONPATH: $env:VTKBONE_DIR\lib\Release"
Write-Host "Adding to PYTHONPATH: $env:VTKBONE_DIR\Wrapping\Python"
$env:PYTHONPATH = "$env:VTKBONE_DIR\Wrapping\Python;$env:VTKBONE_DIR\Release;$env:PYTHONPATH"
