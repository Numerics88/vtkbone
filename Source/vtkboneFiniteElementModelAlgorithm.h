/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneFiniteElementModelAlgorithm
    @brief   Superclass for algorithms that
  produce only vtkboneFiniteElementModel as output
*/

#ifndef __vtkboneFiniteElementModelAlgorithm_h
#define __vtkboneFiniteElementModelAlgorithm_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkDataSet;
class vtkboneFiniteElementModel;

class VTKBONE_EXPORT vtkboneFiniteElementModelAlgorithm : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkboneFiniteElementModelAlgorithm *New();
  vtkTypeMacro(vtkboneFiniteElementModelAlgorithm,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /*! Get the output data object for a port on this algorithm. */
  vtkboneFiniteElementModel* GetOutput();
  vtkboneFiniteElementModel* GetOutput(int);
  virtual void SetOutput(vtkDataObject* d) override;
  //@}

  //@{
  /*! see vtkAlgorithm for details */
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*) override;
  //@}

  // this method is not recommended for use, but lots of old style filters
  // use it
  vtkDataObject *GetInput(int port);
  vtkDataObject *GetInput() { return this->GetInput(0); };
  vtkboneFiniteElementModel *GetFiniteElementModelInput(int port);

  //@{
  /*! Assign a data object as input. Note that this method does not
      establish a pipeline connection. Use SetInputConnection() to setup a
      pipeline connection. */
  void SetInputData(vtkDataObject *);
  void SetInputData(int, vtkDataObject*);
  //@}

  //@{
  /*! Assign a data object as input. Note that this method does not
      establish a pipeline connection. Use SetInputConnection() to setup a
      pipeline connection. */
  void AddInputData(vtkDataObject *);
  void AddInputData(int, vtkDataObject*);
  //@}

protected:
  vtkboneFiniteElementModelAlgorithm();
  ~vtkboneFiniteElementModelAlgorithm();

  // convenience method
  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector) override;

  //@{
  /*! This is called by the superclass. This is the method you should
      override. */
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;
  //@}


  //@{
  /*! This is called by the superclass. This is the method you should
      override. */
  virtual int RequestDataObject(vtkInformation*,
                                vtkInformationVector**,
                                vtkInformationVector*);
  //@}

  //@{
  /*! This is called by the superclass. This is the method you should
      override. */
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*) override;
  //@}

  //@{
  /*! This method is the old style execute method */
  virtual void ExecuteData(vtkDataObject *output);
  virtual void Execute();
  //@}

  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info) override;
  virtual int FillInputPortInformation(int port, vtkInformation* info) override;

private:
  vtkboneFiniteElementModelAlgorithm(const vtkboneFiniteElementModelAlgorithm&);  // Not implemented.
  void operator=(const vtkboneFiniteElementModelAlgorithm&);  // Not implemented.
};

#endif
