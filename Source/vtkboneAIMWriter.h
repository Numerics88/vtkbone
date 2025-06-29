/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneAIMWriter
    @brief   writes Scanco .aim files.

 vtkboneAIMWriter is a writer object that writes Scanco .aim files.

 Accepts vtkImageData with data either on the Points or on the Cells.
 If on the Cells, the Offset of the input image must give the coordinates
 of the corner of voxel 0; if on the Points, the Offset of the input image
 must give the coordinates of the center of voxel 0.

 The center of Cell 0,0,0 in the input vtkImageData is written as the
 "pos" of the AIM.  Note that the AIM can only store integral pixel offsets
 for "pos".
*/

#ifndef __vtkboneAIMWriter_h
#define __vtkboneAIMWriter_h

#include "vtkWriter.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkImageData;

class VTKBONE_EXPORT vtkboneAIMWriter : public vtkWriter
{
public:
  static vtkboneAIMWriter *New();
  vtkTypeMacro(vtkboneAIMWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /*! Get the input to this writer. */
  vtkImageData* GetInput();
  vtkImageData* GetInput(int port);
  //@}

  //@{
  /*! Specify file name of data file to write. */
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  //@}

  //@{
  /*! Set/get the processing log string. */
  vtkSetStringMacro(ProcessingLog);
  vtkGetStringMacro(ProcessingLog);
  //@}

  /*! Appends a string to the processing log. */
  void AppendToLog(const char* textToAppend);

  //@{
  /*! This adds the basic information for an AIM to the start of the
      processing log if this is turned on.  Use this to create new AIMs. 
      When it is off, then anything set by the user with the
      "ProcessingLog" string is simply applied to the AIM log. Default is
      On. */
  vtkSetMacro(NewProcessingLog, int);
  vtkGetMacro(NewProcessingLog, int);
  vtkBooleanMacro(NewProcessingLog, int);
  //@}

protected:
  vtkboneAIMWriter();
  ~vtkboneAIMWriter();

  /*! Writes the AIM. */
  void WriteData() override;

  char *FileName;
  char *ProcessingLog;
  int CompressData;
  int NewProcessingLog;

  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

private:
  // Prevent compiler from making default versions of these.
  vtkboneAIMWriter(const vtkboneAIMWriter&);
  void operator=(const vtkboneAIMWriter&);

};

#endif

