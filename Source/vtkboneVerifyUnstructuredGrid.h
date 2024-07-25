/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneVerifyUnstructuredGrid
    @brief   verifies the consistency of
  vtkUnstructuredGrid objects.


  This object will generate a VTK error if the input fails a consistency
  tests.  The input is passed through unmodified to the output.
  At the moment, only datasets consisting entirely of cells of VTK_VOXEL
  can be checked.  This will be generalized to other cell types in future.

  The tests are:
    1. Check that point and cell Ids are within range.
    2. Check the the topology of the cells (ie. the list of point Ids for
       each cell) is consistent with the cell type.
    3. Check that all attribute arrays have the correct length.
*/

#ifndef __vtkboneVerifyUnstructuredGrid_h
#define __vtkboneVerifyUnstructuredGrid_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkCell;

class VTKBONE_EXPORT vtkboneVerifyUnstructuredGrid : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkboneVerifyUnstructuredGrid* New();
  vtkTypeMacro(vtkboneVerifyUnstructuredGrid, vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /*! Set/Get the point coordinate tolerance distance.	This tolerance is
      used when verifying cell topology. */
  vtkSetMacro(Tolerance, double);
  vtkGetMacro(Tolerance, double);
  //@}

protected:
  vtkboneVerifyUnstructuredGrid();
  ~vtkboneVerifyUnstructuredGrid();

  virtual int RequestData(vtkInformation *request,
                          vtkInformationVector **inputVector,
                          vtkInformationVector *outputVector) override;

  int CheckVoxelTopology(vtkUnstructuredGrid* ug, vtkIdType cellId);

  int VectorsEqual(double a[3], double b[3]);

  int VectorsAligned(double a[3], double b[3]);

  double Tolerance;
};

#endif
