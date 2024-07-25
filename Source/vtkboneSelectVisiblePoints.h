/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneSelectVisiblePoints
    @brief   Selects points that are not obscured
 by a set of Polygons.


 This filter takes two inputs.  Input 0 is a vtkPolyData that represents
 polygons (a "surface") that might potentially obscure points.  Input 1 is
 any type of vtkDataSet, that contains the Points to be tested for
 visibility (the Cells of input 1 are ignored).  The output is a vtkPolyData
 object contains all the visible cells, represented as a set of VTK_VERTEX.
 The original Point Ids can be traced back by the PointData attribute PedigreeIds
 array, it is exists.  Otherwise a PedigreeIds attribute array will be
 generated for this purpose.

    @sa
 vtkSelection vtkSelectVisiblePoints
*/

#ifndef __vtkboneSelectVisiblePoints_h
#define __vtkboneSelectVisiblePoints_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkboneWin32Header.h"

class VTKBONE_EXPORT vtkboneSelectVisiblePoints : public vtkPolyDataAlgorithm
{
public:
  static vtkboneSelectVisiblePoints* New();
  vtkTypeMacro(vtkboneSelectVisiblePoints, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /*! Sets the tolerance for calculating intersection.	If any polygon lies
      within Tolerance of the ray of a point, that point is considered
      hidden. */
  vtkSetMacro(Tolerance, float);
  vtkGetMacro(Tolerance, float);
  //@}

  //@{
  /*! Set get the normal vector from the object towards the viewer. Default
      is 0,0,1 . */
  vtkSetVector3Macro(NormalVector, double);
  vtkGetVector3Macro(NormalVector, double);
  //@}

  //@{
  /*! Find the distance from the point P along the vector V to the bounding
      box surface. */
  static double LineBoundsIntersection(
    double P[3],
    double V[3],
    double bounds[6]);
  //@}

protected:
  vtkboneSelectVisiblePoints();
  ~vtkboneSelectVisiblePoints();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;

  virtual int FillInputPortInformation(int port, vtkInformation* info) override;

  float Tolerance;
  double NormalVector[3];

private:
  vtkboneSelectVisiblePoints(const vtkboneSelectVisiblePoints&);  // Not implemented.
  void operator=(const vtkboneSelectVisiblePoints&);  // Not implemented.
};

#endif
