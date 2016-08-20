#include "vtkboneSolverParameters.h"
#include "vtkObjectFactory.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationStringVectorKey.h"
#include "vtkboneMacros.h"

vtkStandardNewMacro (vtkboneSolverParameters);

vtkInformationKeyMacro (vtkboneSolverParameters, MAXIMUM_ITERATIONS, Integer);
vtkInformationKeyMacro (vtkboneSolverParameters, CONVERGENCE_TOLERANCE, Double);
vtkInformationKeyMacro (vtkboneSolverParameters, MAXIMUM_PLASTIC_ITERATIONS, Integer);
vtkInformationKeyMacro (vtkboneSolverParameters, PLASTIC_CONVERGENCE_TOLERANCE, Double);
vtkInformationKeyMacro (vtkboneSolverParameters, POST_PROCESSING_NODE_SETS, StringVector);
vtkInformationKeyMacro (vtkboneSolverParameters, POST_PROCESSING_ELEMENT_SETS, StringVector);
vtkInformationKeyMacro (vtkboneSolverParameters, ROTATION_CENTER, DoubleVector);

//----------------------------------------------------------------------------
vtkboneSolverParameters::vtkboneSolverParameters()
{
}

//----------------------------------------------------------------------------
vtkboneSolverParameters::~vtkboneSolverParameters()
{
}
