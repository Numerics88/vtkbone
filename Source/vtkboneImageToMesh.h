/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneImageToMesh - takes a vtkImageData as input and outputs a
//  vtkUnstructuredGrid of hexahedron cells.
//
// .SECTION Description
// vtkboneImageToMesh creates a solid model of hexahedrons (actually of
// type VTK_VOXEL).
//
// It is possible to pass the scalar values associated with each
// image point. Only non-zero scalar values from the input image
// points are used to generate cells.
//
// The input may have the scalar data either on the Points or on the Cells.
// For Point scalar data, each point will be at the center of a
// hexahedron.
// For Cell scalar data, each cell maps to a hexahedron.
//
// The scalars on the output data will be named "MaterialID".
//
// The following diagram shows the topology of the VTK_VOXEL type.
//
// @verbatim
//           6---------7
//         / |       / |
//        /  |      /  |
//       4---------5   |
//       |   |     |   |
//       |   2-----|---3
//       |  /      |  /
//       |/        | /     z  y
//       0---------1       | /
//                         |/
//                         o--->x
// @endverbatim
//
// The numbering of both the nodes and the elements of the output is sorted
// with increasing X,Y,Z values, with X being the fastest changing coordinate
// and Z the slowest.


#ifndef __vtkboneImageToMesh_h
#define __vtkboneImageToMesh_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkboneWin32Header.h"

// forward declarations
class vtkImageData;

class VTKBONE_EXPORT vtkboneImageToMesh : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkboneImageToMesh* New();
  vtkTypeMacro(vtkboneImageToMesh, vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void Report(ostream& s);

protected:
  vtkboneImageToMesh();
  ~vtkboneImageToMesh();

  virtual int FillInputPortInformation(int port, vtkInformation* info);

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  // Description:
  // Returns the input if the input has the scalar data on the points; otherwise
  // returns a new vtkImageData object that has the scalar data on the
  // points.
  int GetInputAsCellScalars(vtkImageData* input,
                            vtkImageData* inputAsPointScalars);

  int GenerateHexahedrons(vtkImageData* input,
                          vtkUnstructuredGrid* output);

private:
  // Prevent compiler from making default versions of these.
  vtkboneImageToMesh(const vtkboneImageToMesh&);
  void operator=(const vtkboneImageToMesh&);
};


//-----------------------------------------------------------------------
// Namespace for printing more concise reports for classes.
// This is consistent with the calling format in vtkbonePrettyReports.h
//
namespace vtkbonePrettyReports
{
  //-----------------------------------------------------------------------
  inline void Print(std::ostream& s, vtkboneImageToMesh* obj)
  {
    obj->Report(s);
  }
}  // namespace vtkbonePrettyReports

#endif
