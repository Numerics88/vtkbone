/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneDecimateImage
    @brief   Reduces the linear dimension by 2 in such a way that values in the input are not interpolated.


 Reduces the linear dimension of an image by a factor 2 (thus 8 for the number of voxels),
 in such a way that the resulting image can used to generate an FE model
 approximately equivalent to one generated from the original image.
 Since the values in the image are interpreted as material IDs, they can not be
 averaged. Instead, the maximum value over every 2x2x2 block of voxels is taken
 as the value of the output voxel.

 If the image has any odd dimensions, it is padded out to an even dimension by duplicating
 the outer-most layer of voxels.

 In general, it is preferable to use vtkboneCoarsenModel after generating a FE model,
 as that class can interpolate material properties, as it has access to the
 material definitions.

    @sa
 vtkboneCoarsenModel vtkboneInterpolateCoarseSolution
*/

#ifndef __vtkboneDecimateImage_h
#define __vtkboneDecimateImage_h

#include "vtkImageAlgorithm.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneDecimateImage : public vtkImageAlgorithm
{
public:
  static vtkboneDecimateImage *New();
  vtkTypeMacro(vtkboneDecimateImage,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkboneDecimateImage() {}
  ~vtkboneDecimateImage() {}

  virtual int RequestInformation(vtkInformation *, vtkInformationVector **,
                                 vtkInformationVector *) override;
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **,
                                  vtkInformationVector *) override;
  virtual int RequestData(vtkInformation *, vtkInformationVector **,
                          vtkInformationVector *) override;

  virtual int SimpleExecute(vtkImageData* input, vtkImageData* output);

private:
  vtkboneDecimateImage(const vtkboneDecimateImage&);  // Not implemented.
  void operator=(const vtkboneDecimateImage&);  // Not implemented.
};

#endif
