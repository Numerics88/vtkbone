/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneErrorWarningObserver
    @brief   a simple observer to catch VTK warnings and errors.


 This class takes no action except to save the description of the error
 so that it can be retreived later.
*/

#ifndef __vtkboneErrorWarningObserver_h
#define __vtkboneErrorWarningObserver_h

#include "vtkCommand.h"
#include "vtkboneWin32Header.h"
#include "vtkboneMacros.h"

class VTKBONE_EXPORT vtkboneErrorWarningObserver : public vtkCommand
{
public:
  static vtkboneErrorWarningObserver *New()
    {return new vtkboneErrorWarningObserver;};

  // Method needs to exist for vtkSetStringMacro.
  void Modified() {}

  virtual void Execute(vtkObject* caller, unsigned long eventId, void* callData);

  //@{
  /*! Set/Get error descriptions. */
  vtkboneSetStringMacro(ErrorDescriptions);
  vtkboneGetStringMacro(ErrorDescriptions);
  virtual void AppendErrorDescriptions(const char* newMsg);
  //@}

  //@{
  /*! Set/Get warning descriptions. */
  vtkboneSetStringMacro(WarningDescriptions);
  vtkboneGetStringMacro(WarningDescriptions);
  virtual void AppendWarningDescriptions(const char* newMsg);
  //@}

  /*! Returns 1 if error occurred. */
  virtual int ErrorOccurred();

  /*! Returns 1 if warning occurred. */
  virtual int WarningOccurred();

  /*! Clears stored error and warnings. */
  virtual void Reset();

  // Dummy method required by Set/Get string macros
  virtual int GetDebug() {return 0;}

protected:
  vtkboneErrorWarningObserver();
  ~vtkboneErrorWarningObserver();

  char* ErrorDescriptions;
  char* WarningDescriptions;

private:
  vtkboneErrorWarningObserver(const vtkboneErrorWarningObserver&);  // Not implemented.
  void operator=(const vtkboneErrorWarningObserver&);  // Not implemented.
};

#endif
