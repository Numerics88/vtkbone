/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkbonepQCTReader
    @brief   read Stratec pQCT files.

 vtkbonepQCTReader is a source object that reads Stratec pQCT files.

 vtkbonepQCTReader creates vtkImageData datasets.

 The result is a 2D slice dataset.

 This reader can, as a user-controlled option, put the data on the PointData
 scalars or on the CellData scalars.  The Cell scalars are more natural
 for Finite Element models,
 as the points represent the center value of voxels.  In particular, for
 using the threshold filter for example, it is typically necessary to put
 the data on the Cell scalars.  However, vtkImageData is easier to use with
 the data on the Points (many of its methods assume this).  This reader
 therefore supports both.  The origin is shifted by 1/2 a voxel relative to
 the Offset if the data is placed on Points.  (i.e. the points are located
 where cell centers would be if the data were placed on the cells).
 This is so that rendering will be in consistent coordinate frames.  Note
 that most VTK filters which take vtkImageData as input expect the data
 to be on the Points; VTKBONE filters generally accept input images with
 either on the Points or on the Cells.
*/

#ifndef __vtkbonepQCTReader_h
#define __vtkbonepQCTReader_h

#include "vtkImageAlgorithm.h"
#ifdef N88_BUILD_AS_PARAVIEW_PLUGIN
#define VTKBONE_EXPORT VTK_EXPORT
#else
#include "vtkboneWin32Header.h"
#endif

// Forward declarations
class vtkImageData;
class vtkMath;

class VTKBONE_EXPORT vtkbonepQCTReader : public vtkImageAlgorithm
{
public:
  static vtkbonepQCTReader *New();
  vtkTypeMacro(vtkbonepQCTReader, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /*! Set/Get the name of the file to read. */
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  //@}

  //@{
  /*! Set/Get flag to specify whether data should be on cells or on points. */
  vtkSetMacro(DataOnCells, int);
  vtkGetMacro(DataOnCells, int);
  vtkBooleanMacro(DataOnCells, int);
  //@}

  //@{
  /*! Was there an error on the last read performed? */
  vtkGetMacro(Error, int);
  //@}

  //@{
  /*! Get pQCT Dimension. */
  vtkGetVector3Macro(Dimension, int);
  //@}

  //@{
  /*! Get pQCT Position. */
  vtkGetVector3Macro(Position, int);
  //@}

  //@{
  /*! Get pQCT Element Size.  Units are mm. */
  vtkGetVector3Macro(ElementSize, double);
  //@}


protected:
  vtkbonepQCTReader();
  ~vtkbonepQCTReader();

  int RequestInformation(vtkInformation* request,
                         vtkInformationVector** inputVector,
                         vtkInformationVector* outputVector) override;
  int RequestData(vtkInformation*,
                  vtkInformationVector**,
                  vtkInformationVector*) override;

  char *FileName;
  int DataOnCells;  // Flag to put data on cells instead of points.

  int Dimension[3];
  int Position[3];
  double ElementSize[3];

  int Error;

private:
  // Prevent compiler from making default versions of these.
  vtkbonepQCTReader(const vtkbonepQCTReader&);
  void operator=(const vtkbonepQCTReader&);
};

#endif /* __vtkbonepQCTReader_h */
