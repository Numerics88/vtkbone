/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneMaterialTableAlgorithm - Superclass for algorithms that
//  produce only vtkboneMaterialTable as output

#ifndef __vtkboneMaterialTableAlgorithm_h
#define __vtkboneMaterialTableAlgorithm_h

#include "vtkAlgorithm.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkDataSet;
class vtkboneMaterialTable;

class VTKBONE_EXPORT vtkboneMaterialTableAlgorithm : public vtkAlgorithm
{
public:
  static vtkboneMaterialTableAlgorithm *New();
  vtkTypeMacro(vtkboneMaterialTableAlgorithm,vtkAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Get the output data object for a port on this algorithm.
  vtkboneMaterialTable* GetOutput();
  vtkboneMaterialTable* GetOutput(int);
  virtual void SetOutput(vtkDataObject* d);

  // Description:
  // see vtkAlgorithm for details
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*) override;

  // this method is not recommended for use, but lots of old style filters
  // use it
  vtkDataObject *GetInput(int port);
  vtkDataObject *GetInput() { return this->GetInput(0); };
  vtkboneMaterialTable *GetMaterialTableInput(int port);

  // Description:
  // Assign a data object as input. Note that this method does not
  // establish a pipeline connection. Use SetInputConnection() to
  // setup a pipeline connection.
  void SetInputData(vtkDataObject *);
  void SetInputData(int, vtkDataObject*);

  // Description:
  // Assign a data object as input. Note that this method does not
  // establish a pipeline connection. Use SetInputConnection() to
  // setup a pipeline connection.
  void AddInputData(vtkDataObject *);
  void AddInputData(int, vtkDataObject*);

protected:
  vtkboneMaterialTableAlgorithm();
  ~vtkboneMaterialTableAlgorithm();

  // convenience method
  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestDataObject(vtkInformation*,
                                vtkInformationVector**,
                                vtkInformationVector*);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);

  // Description:
  // This method is the old style execute method
  virtual void ExecuteData(vtkDataObject *output);
  virtual void Execute();

  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info) override;
  virtual int FillInputPortInformation(int port, vtkInformation* info) override;

private:
  vtkboneMaterialTableAlgorithm(const vtkboneMaterialTableAlgorithm&);  // Not implemented.
  void operator=(const vtkboneMaterialTableAlgorithm&);  // Not implemented.
};

#endif
