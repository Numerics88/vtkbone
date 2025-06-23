/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneFaimVersion5InputWriter
    @brief   writes faim finite element input file.

 Takes as input a vtkboneFiniteElementModel and writes as output a file suitable for
 input to the faim finite element solver.

 Note that the output file format is 1-indexed.  (vtkboneFiniteElementModel is 0-indexed.)

 Note that the topology of hexahedral cells is different for faim than it
 is for vtkboneFiniteElementModel.  This is automatically translated.  Refer
 to vtkboneFiniteElementModel for the topology of its elements.  The figure
 below shows the topology of the hexahedral elements as written to the faim
 data file (shown 1-indexed because faim is 1-indexed).

 @verbatim
           6---------7
          /|        /|
         / |       / |
        /  |      /  |
       2---------3   |
       |   |     |   |
       |   5-----|---8
       |  /      |  /
       |/        | /     z  y
       1---------4/      | /
                         |/
                         .--->x
 @endverbatim

    @sa
 vtkboneFiniteElementModel vtkboneFiniteElementModelGenerator
*/

#ifndef __vtkboneFaimVersion5InputWriter_h
#define __vtkboneFaimVersion5InputWriter_h

#include "vtkWriter.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkPoints;
class vtkboneFiniteElementModel;
class vtkDoubleArray;
class vtkIdList;
class vtkIdTypeArray;
class vtkCharArray;
class vtkboneConstraint;

class VTKBONE_EXPORT vtkboneFaimVersion5InputWriter : public vtkWriter
{
public:
  static vtkboneFaimVersion5InputWriter *New();
  vtkTypeMacro(vtkboneFaimVersion5InputWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /*! Specify file name of file to write. */
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  //@}

  //@{
  /*! Set the tolerance for distinguishing between non-zero Displacement
      contraints and Fixed constraints. Default is 1E-8 . */
  vtkSetMacro(DisplacementTolerance, double);
  vtkGetMacro(DisplacementTolerance, double);
  //@}

protected:
  vtkboneFaimVersion5InputWriter();
  ~vtkboneFaimVersion5InputWriter();

  void WriteData() override;

  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  int WriteNodes(ostream *fp, vtkboneFiniteElementModel* model);
  int WriteElements(ostream *fp, vtkboneFiniteElementModel* model);
  int WriteMeshOutput(ostream *fp, vtkboneFiniteElementModel* model);

  int VerifyConstraint(vtkboneConstraint* constraint);

  int WriteFixedConstraints(ostream *fp, vtkboneFiniteElementModel* model);
  int WriteForceConstraints(ostream *fp, vtkboneFiniteElementModel* model);
  int WriteDisplacementConstraints(ostream *fp, vtkboneFiniteElementModel* model);
  int WriteNodeSet(ostream *fp, vtkboneFiniteElementModel* model, const char* setName);
  int WriteContainingElementSet(ostream *fp, vtkboneFiniteElementModel* model, const char* setName);

  char* FileName;
  double DisplacementTolerance;

private:
  vtkboneFaimVersion5InputWriter(const vtkboneFaimVersion5InputWriter&); // Not implemented
  void operator=(const vtkboneFaimVersion5InputWriter&); // Not implemented
};

#endif

