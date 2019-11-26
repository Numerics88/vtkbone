/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneImageConnectivityMap - mark regions of image data based on geometric connectivity
//
// .SECTION Description
//
// This filter generates a mask based on connectivity of the scalar values of
// the image.  Connectivity is considered only across faces, not across edges
// or corners.
//
// This filter accepts image scalars on either the cells or the points; the
// output will have scalars with the same association.
//
// The output scalar data type is always vtkUnsignedInt.

#ifndef __vtkboneImageConnectivityMap_h
#define __vtkboneImageConnectivityMap_h

#include "vtkSimpleImageToImageFilter.h"
#include "vtkboneWin32Header.h"

// forward declarations
class vtkUnsignedIntArray;


class VTKBONE_EXPORT vtkboneImageConnectivityMap : public vtkSimpleImageToImageFilter
{
public:
  static vtkboneImageConnectivityMap* New();
  vtkTypeMacro(vtkboneImageConnectivityMap,
                       vtkSimpleImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Return the number of identified regions.
  vtkGetMacro(NumberOfRegions, unsigned int);

protected:
  vtkboneImageConnectivityMap();
  ~vtkboneImageConnectivityMap();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);

  virtual void SimpleExecute(vtkImageData*, vtkImageData*);

  //BTX
  template <typename TArray>
  int GenerateConnectivityMap(vtkDataArray* in_data_arg,
                              int* dims,
                              vtkUnsignedIntArray* cmap);

  template <typename TArray>
  int MarkComponent(TArray* in_data_arg,
                    int* dims,
                    vtkUnsignedIntArray* cmap,
                    int seed_i,
                    int seed_j,
                    int seed_k,
                    unsigned int component);
  //ETX

  unsigned int NumberOfRegions;

private:
  // Prevent compiler from making default versions of these.
  vtkboneImageConnectivityMap(const vtkboneImageConnectivityMap&);
  void operator=(const vtkboneImageConnectivityMap&);
};

#endif
