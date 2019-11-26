/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneOrientationFilter - select polygons that face a specified direction.
//
// .SECTION Description
//
// This filter passes through only polygons in a vtkPolyData that have a
// positive dot product with a specified normal vector.
//
// The original Cell and Point Ids can be traced back by the
// CellData/DataPointData attribute PedigreeIds arrays.
//
// This filter will use the Normals CellData array of the input data if
// it exists, otherwise it will create it and attach it to the input
// DataSet CellData.  (i.e. This filter might modify its input.  This
// is done for efficiency to avoid unnecessarily recalculating normals.)
// .SECTION See Also
// vtkboneSelectVisiblePoints

#ifndef __vtkboneOrientationFilter_h
#define __vtkboneOrientationFilter_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneOrientationFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkboneOrientationFilter* New();
  vtkTypeMacro(vtkboneOrientationFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set get the normal vector from the object towards the viewer.
  // Default is 0,0,1 .
  vtkSetVector3Macro(NormalVector, double);
  vtkGetVector3Macro(NormalVector, double);

protected:
  vtkboneOrientationFilter();
  ~vtkboneOrientationFilter();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  double NormalVector[3];

private:
  vtkboneOrientationFilter(const vtkboneOrientationFilter&);  // Not implemented.
  void operator=(const vtkboneOrientationFilter&);  // Not implemented.
};

#endif
