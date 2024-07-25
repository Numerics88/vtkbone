/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneNodeSetsByGeometry
    @brief   various algorithms to select nodes sets
 from a mesh by geometry.


 These static methods are typically used to find nodes for boundary
 conditions for finite element models (vtkboneFiniteElementModel).

 Note that in the future these static methods may be replaced by filter
 classes.

    @sa
 vtkboneFiniteElementModel vtkboneFiniteElementModelGenerator
*/

#ifndef __vtkboneNodeSetsByGeometry_h
#define __vtkboneNodeSetsByGeometry_h

#include "vtkObject.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkUnstructuredGrid;
class vtkboneFiniteElementModel;
class vtkDataSet;
class vtkIdTypeArray;
class vtkPolyData;
class vtkIdList;
class vtkPoints;

class VTKBONE_EXPORT vtkboneNodeSetsByGeometry : public vtkObject
{
public:
  static vtkboneNodeSetsByGeometry *New();
  vtkTypeMacro(vtkboneNodeSetsByGeometry,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /*! Determine the bounds of model, either for the whole model, or for the
      SpecificMaterial defined (i.e., representing endcaps). */
  static void DetermineMaterialBounds(vtkUnstructuredGrid *geometry,
                                      double bounds[6],
                                      int specificMaterial = -1);
  //@}

  //@{
  /*! Given a data set and a list of point IDs, returns a reduced list of
      point IDs corresponding only to points for which at least one
      neighbouring cell has the specified scalar value. */
  static int FilterPointListByCellScalar(vtkIdTypeArray *output_ids,
                                          vtkIdTypeArray *input_ids,
                                          vtkDataSet *data,
                                          int targetCellScalar);
  //@}

  //@{
  /*! Function returns the node ids of all points in the unstructured grid
      that lie on the defined plane.  The plane is defined by two
      parameters: axis and val. The plane intersects the coordinate axis
      given by axis (=0,1,2) at val (a float). For example, to find all
      nodes lying on the bottom Z face of a cube, set axis=2 and val = 0.0
      (or whatever the minimum Z value is in the data set). */
  static int FindNodesOnPlane(
    int axis,
    float val,
    vtkIdTypeArray *ids,
    vtkUnstructuredGrid *ug,
    int specificMaterial = -1);
  //@}

  //@{
  /*! Calls FindNodesOnPlane then adds the resulting set to model. */
  static int AddNodesOnPlane(
    int axis,
    float val,
    const char* name,
    vtkboneFiniteElementModel* model,
    int specificMaterial = -1);
  //@}

  //@{
  /*! Calls AddNodesOnPlane and also adds the associated elements. */
  static int AddNodesAndElementsOnPlane(
    int axis,
    float val,
    const char* name,
    vtkboneFiniteElementModel* model,
    int specificMaterial = -1);
  //@}

  //@{
  /*! Function returns the node ids of all points in the unstructured grid
      that lie on the defined straight line. This algorithm is a
      modification of vtkboneNodeSetsByGeometry::FindNodesOnPlane. It is
      based on two planes intersecting each other. */
  static int FindNodesIntersectingTwoPlanes(
    int axis1,
    float val1,
    int axis2,
    float val2,
    vtkIdTypeArray* ids,
    vtkUnstructuredGrid* ug,
    int specificMaterial = -1);
  //@}

  //@{
  /*! Calls FindNodesIntersectingTwoPlanes and adds the resulting node set
      to model. */
  static int AddNodesIntersectingTwoPlanes(
    int axis1,
    float val1,
    int axis2,
    float val2,
    const char* name,
    vtkboneFiniteElementModel* model,
    int specificMaterial = -1);
  //@}

  //@{
  /*! Calls AddNodesIntersectingTwoPlanes and also adds the associated
      elements set. */
  static int AddNodesAndElementsIntersectingTwoPlanes(
    int axis1,
    float val1,
    int axis2,
    float val2,
    const char* name,
    vtkboneFiniteElementModel* model,
    int specificMaterial = -1);
  //@}

  //@{
  /*! Function returns the node ids of all points in the unstructured grid
      that lie on the defined point. This algorithm is a modification of
      vtkboneNodeSetsByGeometry::FindNodesOnPlane. It is based on three
      planes intersecting each other. */
  static int FindNodesIntersectingThreePlanes(
    int axisA,
    float valA,
    int axisB,
    float valB,
    int axisC,
    float valC,
    vtkIdTypeArray *ids,
    vtkUnstructuredGrid *ug,
    int specificMaterial = -1);
  //@}

  //@{
  /*! Calls FindNodesIntersectingThreePlanes and adds the resulting node
      set to model. */
  static int AddNodesIntersectingThreePlanes(
    int axisA,
    float valA,
    int axisB,
    float valB,
    int axisC,
    float valC,
    const char* name,
    vtkboneFiniteElementModel* model,
    int specificMaterial = -1);
  //@}

  //@{
  /*! Calls AddNodesIntersectingThreePlanes and also adds the associated
      elements. */
  static int AddNodesAndElementsIntersectingThreePlanes(
    int axisA,
    float valA,
    int axisB,
    float valB,
    int axisC,
    float valC,
    const char* name,
    vtkboneFiniteElementModel* model,
    int specificMaterial = -1);
  //@}

  //@{
  /*! Function returns the node ids of all points that you would be able to
      see if you looked down upon the object from the specified dirction.
      This is useful for specifying which nodes should have forces applied
      to them if the surface is uneven. normalVector is the vector from the
      object towards the viewer. If the input object contains a PedigreeIds
      array, those values will be returned in visibleNodesIds; otherwise
      the point ids of the input object will be used. */
  static int FindNodesOnVisibleSurface(
    vtkIdTypeArray *visibleNodesIds,
    vtkUnstructuredGrid *ug,
    double normalVector[3],
    int specificMaterial = -1);
  //@}

  //@{
  /*! Calls FindNodesOnSurface and adds the resulting node set to model. */
  static int AddNodesOnVisibleSurface(
    const char* name,
    vtkboneFiniteElementModel* model,
    double normalVector[3],
    int specificMaterial = -1);
  //@}

  //@{
  /*! Calls AddNodesOnVisibleSurface and also adds the associated elements. */
  static int AddNodesAndElementsOnVisibleSurface(
    const char* name,
    vtkboneFiniteElementModel* model,
    double normalVector[3],
    int specificMaterial = -1);
  //@}

protected:
  vtkboneNodeSetsByGeometry() {};
  ~vtkboneNodeSetsByGeometry() {};

private:
  vtkboneNodeSetsByGeometry(const vtkboneNodeSetsByGeometry&); // Not implemented
  void operator=(const vtkboneNodeSetsByGeometry&); // Not implemented
};

#endif
