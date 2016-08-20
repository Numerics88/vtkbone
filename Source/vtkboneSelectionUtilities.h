/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneSelectionUtilities - Useful functions for creating
// and modifying sets of Points and/or Cells.
//
// .SECTION See Also
// vtkSelection vtkSelectionNode vtkConvertSelection

#ifndef __vtkboneSelectionUtilities_h
#define __vtkboneSelectionUtilities_h

#include "vtkObject.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkDataSet;
class vtkUnstructuredGrid;
class vtkPolyData;
class vtkIdTypeArray;
class vtkSelection;
class vtkImplicitFunction;

class VTKBONE_EXPORT vtkboneSelectionUtilities : public vtkObject
{
public:
  static vtkboneSelectionUtilities* New();
  vtkTypeMacro(vtkboneSelectionUtilities, vtkObject);

  // Description:
  // Given a list of point Ids, create a corresponding vtkSelection object.
  static void PointSelectionFromIds(
      vtkSelection* selection,
      const vtkDataSet* data,
      const vtkIdTypeArray* ids);

  // Description:
  // Given a list of cell Ids, create a corresponding vtkSelection object.
  static void CellSelectionFromIds(
      vtkSelection* selection,
      const vtkDataSet* data,
      const vtkIdTypeArray* ids);

  // Description:
  // Given any type of selection returns the cell Ids of all the cells
  // containing and nodes in the selection.
  // If the PedigreeIds attribute array is present on CellData, then those
  // Ids will be used.
  // This somewhat mirrors static methods in vtkConvertSelection, but those
  // don't work for containing cell selections.
  // Furthermore, using the VTK containing cells selections is slow.
  // This method is much faster.
  static void GetContainingCells(
      vtkSelection* selection,
      vtkDataSet* data,
      vtkIdTypeArray* cellIds);
  static void GetContainingCellsFromUnstructuredGrid(
      vtkSelection* selection,
      vtkUnstructuredGrid* data,
      vtkIdTypeArray* cellIds);
  static void GetContainingCellsFromPolyData(
      vtkSelection* selection,
      vtkPolyData* data,
      vtkIdTypeArray* cellIds);
  static void GetContainingCellsFromGeneric(
      vtkSelection* selection,
      vtkDataSet* data,
      vtkIdTypeArray* cellIds);

  // Description:
  // Given any type of selection, converts it to a "containing cells"
  // selection.
  // This somewhat mirrors static methods in vtkConvertSelection, but those
  // don't work for containing cell selections.
  static void ConvertToContainingCellsSelection(
      vtkSelection* selectionIn,
      vtkDataSet* data,
      vtkSelection* selectionOut);

  // Description:
  // Add an array as the PedigreeIds attribute array of the PointData.
  // This array has initially values equal to indices.
  // It is often used in often used in
  // VTK to trace back original Node Ids after selecting a subset created
  // as a new object.  VTK filters typically will copy through PointData
  // for each new point. If the named array already exists, it is not
  // replaced unless replace is set to 1.
  static void AddPointPedigreeIdsArray(vtkDataSet* data, int replace=0);

  // Description:
  // Add an array as the PedigreeIds attribute array of the CellData.
  // This array has initially values equal to indices.
  // It is often used in often used in
  // VTK to trace back original Cell Ids after selecting a subset created
  // as a new object.  VTK filters typically will copy through CellData
  // for each new cell.  If the named array already exists, it is not
  // replaced unless replace is set to 1.
  static void AddCellPedigreeIdsArray(vtkDataSet* data, int replace=0);

  static int ExtractPointsAsPolyData(
      vtkIdTypeArray* ids,
      vtkDataSet* data,
      vtkPolyData* out);

  static int ExtractPointsAsPolyData(
    vtkSelection* selection,
    vtkDataSet* data,
    vtkPolyData* out);

protected:
  vtkboneSelectionUtilities() {};
  ~vtkboneSelectionUtilities() {};

private:
  vtkboneSelectionUtilities(const vtkboneSelectionUtilities&);  // Not implemented.
  void operator=(const vtkboneSelectionUtilities&);  // Not implemented.
};

#endif
