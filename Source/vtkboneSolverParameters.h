/*=========================================================================

  Copyright (c) 2010-2025, Numerics88 Solutions.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! @class   vtkboneSolverParameters
    @brief   Description of a finite element test.


 This class is not intended to be instantiated.  It merely exists to
 provide somewhere to define a number of InformationKeys which are of
 use in specifying parameters for the solver.

    @sa
 vtkboneFiniteElementModel vtkboneFiniteElementModel
*/

#ifndef __vtkboneSolverParameters_h
#define __vtkboneSolverParameters_h

#include "vtkObject.h"
#include "vtkboneWin32Header.h"

// Forward declarations
class vtkInformationIntegerKey;
class vtkInformationDoubleKey;
class vtkInformationDoubleVectorKey;
class vtkInformationStringKey;
class vtkInformationStringVectorKey;

class VTKBONE_EXPORT vtkboneSolverParameters : public vtkObject
{
public:
  static vtkboneSolverParameters* New();
  vtkTypeMacro(vtkboneSolverParameters, vtkObject);

  static vtkInformationIntegerKey* MAXIMUM_ITERATIONS();
  static vtkInformationDoubleKey* CONVERGENCE_TOLERANCE();
  static vtkInformationIntegerKey* MAXIMUM_PLASTIC_ITERATIONS();
  static vtkInformationDoubleKey* PLASTIC_CONVERGENCE_TOLERANCE();
  static vtkInformationStringVectorKey* POST_PROCESSING_NODE_SETS();
  static vtkInformationStringVectorKey* POST_PROCESSING_ELEMENT_SETS();
  static vtkInformationDoubleVectorKey* ROTATION_CENTER();

protected:
  vtkboneSolverParameters();
  ~vtkboneSolverParameters();

private:
  vtkboneSolverParameters(const vtkboneSolverParameters&);  // Not implemented.
  void operator=(const vtkboneSolverParameters&);  // Not implemented.
};

#endif
