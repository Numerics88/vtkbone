# vtkbone
VTK classes for processing of finite element models derived from micro-CT.
vtkbone was formerly vtkn88.

[![Conda Build And Publish](https://github.com/Numerics88/vtkbone/actions/workflows/build-publish-anaconda.yml/badge.svg?branch=master)](https://github.com/Numerics88/vtkbone/actions/workflows/build-publish-anaconda.yml)
[![Anaconda-Server Badge](https://anaconda.org/numerics88/vtkbone/badges/version.svg)](https://anaconda.org/numerics88/vtkbone)
[![Anaconda-Server Badge](https://anaconda.org/numerics88/vtkbone/badges/platforms.svg)](https://anaconda.org/numerics88/vtkbone)

## Documentation

The API documentation for vtkbone can be found on-line at https://bonelab.github.io/n88/documentation/vtkbone/2.0/. Or, you can generate it yourself from the
source code using DOxygen.

A manual exists for the Faim FE package that includes a chapter on using vtkbone,
and a chapter with tutorials. It can be found at http://numerics88.com/documentation/ .

## Compiling and linking

vtkbone requires the following:

  * CMake: www.cmake.org
  * Boost: www.boost.org
  * n88util: https://github.com/Numerics88/n88util
  * AimIO: https://github.com/Numerics88/AimIO
  * pQCTIO: https://github.com/Numerics88/pQCTIO
  * Google test: https://github.com/google/googletest

To build and run the tests with cmake, on linux or macOS, something like the
following sequence of commands is required:

```sh
mkdir build
cd build
ccmake ..
make
ctest -V
```

On Windows the procedure is a rather different: refer to CMake documentation.

## Use vtkbone in a Conda environment

It might be convenient to use `vtkbone` in a conda environment instead of building it yourself.

```commandline
# Setup the environment called bl
conda create --name bl --channel numerics88 python=3 vtkbone

# Activate the environemnt
conda activate bl

# Try reading an AIM file
python
>>> import vtkbone
>>> dir(vtkbone)
>>> reader = vtkbone.vtkboneAIMReader()
>>> reader.SetFileName("test25a.aim")
>>> reader.Update()
>>> print(reader.GetDimension())
(25, 25, 25)
```

## Authors and Contributors

vtkbone is maintained and supported by Numerics88 Solutions (http://numerics88.com). 
It was originally developed at the University of Calgary by Eric Nodwell and 
Steven K. Boyd (skboyd@ucalgary.ca).

## Licence

vtkbone is licensed under a MIT-style open source license. See the file LICENSE.
