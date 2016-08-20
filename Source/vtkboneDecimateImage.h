/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

#ifndef __vtkboneDecimateImage_h
#define __vtkboneDecimateImage_h

#include "vtkImageAlgorithm.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneDecimateImage : public vtkImageAlgorithm
{
public:
  static vtkboneDecimateImage *New();
  vtkTypeMacro(vtkboneDecimateImage,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkboneDecimateImage() {}
  ~vtkboneDecimateImage() {}

  virtual int RequestInformation(vtkInformation *, vtkInformationVector **,
                                 vtkInformationVector *);
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **,
                                  vtkInformationVector *);
  virtual int RequestData(vtkInformation *, vtkInformationVector **,
                          vtkInformationVector *);

  virtual int SimpleExecute(vtkImageData* input, vtkImageData* output);

private:
  vtkboneDecimateImage(const vtkboneDecimateImage&);  // Not implemented.
  void operator=(const vtkboneDecimateImage&);  // Not implemented.
};

#endif
