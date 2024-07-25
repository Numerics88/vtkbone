#include "vtkboneApplyDirectionalShearTest.h"
#include "vtkboneSolverParameters.h"
#include "vtkboneConstraintCollection.h"
#include "vtkboneNodeSetsByGeometry.h"
#include "vtkboneVersion.h"
#include "vtkObjectFactory.h"
#include "vtkCharArray.h"
#include "vtkDoubleArray.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkIdList.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationStringVectorKey.h"
#include "vtkDataSetAttributes.h"
#include "vtkSmartPointer.h"
#include <sstream>

vtkStandardNewMacro(vtkboneApplyDirectionalShearTest);


//----------------------------------------------------------------------------
vtkboneApplyDirectionalShearTest::vtkboneApplyDirectionalShearTest()
  :
  ScaleShearToHeight (1)
{
  this->ShearVector[0] = 0.01;
  this->ShearVector[1] = 0;
}


//----------------------------------------------------------------------------
vtkboneApplyDirectionalShearTest::~vtkboneApplyDirectionalShearTest()
{
}


//----------------------------------------------------------------------------
void vtkboneApplyDirectionalShearTest::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  this->PrintParameters(os,indent);
}

//----------------------------------------------------------------------------
void vtkboneApplyDirectionalShearTest::PrintParameters (ostream& os, vtkIndent indent)
{
  os << indent << "ShearVector: " << this->ShearVector[0] << ", "
                                        << this->ShearVector[1] << "\n";
  os << indent << "ScaleShearToHeight: " << this->ScaleShearToHeight << "\n";
}


//----------------------------------------------------------------------------
int vtkboneApplyDirectionalShearTest::RequestData
  (
    vtkInformation * request,
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector
  )
{
  vtkboneApplyTestBase::RequestData(request, inputVector, outputVector);

  // Only need output object: vtkboneApplyTestBase has already copied the input
  // object to the output object (and added stuff).
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkboneFiniteElementModel *output = vtkboneFiniteElementModel::SafeDownCast(
                            outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
  {
    vtkErrorMacro("No output object.");
    return 0;
  }

  return ((this->AddTopAndBottomConstraints(output) == VTK_OK) &&
          (this->AddConvergenceSet (output) == VTK_OK) &&
          (this->AddPostProcessingSets(output) == VTK_OK) &&
          (this->AddInformation(output) == VTK_OK));
}

//----------------------------------------------------------------------------
int vtkboneApplyDirectionalShearTest::AddTopAndBottomConstraints
  (
  vtkboneFiniteElementModel* model
  )
{
  double bounds[6];
  model->GetBounds(bounds);

  // Fixed constraints on bottom in all directions
  model->FixNodes("face_z0", "bottom_fixed");

  // Fix top nodes in z direction
  model->ApplyBoundaryCondition(
      "face_z1", this->DataFrameSense(2), 0, "top_fixed");

  double disp[2];
  disp[0] = this->ShearVector[0];
  disp[1] = this->ShearVector[1];
  if (this->ScaleShearToHeight)
  {
    double h = this->TestFrameBound(bounds,2,1) - this->TestFrameBound(bounds,2,0);
    disp[0] *= h;
    disp[1] *= h;
  }

  model->ApplyBoundaryCondition(
      "face_z1", this->DataFrameSense(0), disp[0], "top_displacement");
  model->ApplyBoundaryCondition(
      "face_z1", this->DataFrameSense(1), disp[1], "top_displacement");

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneApplyDirectionalShearTest::AddConvergenceSet
  (
  vtkboneFiniteElementModel* model
  )
{
  return model->ConvergenceSetFromConstraint("top_displacement");
}

//----------------------------------------------------------------------------
int vtkboneApplyDirectionalShearTest::AddPostProcessingSets
  (
  vtkboneFiniteElementModel* model
  )
{
  vtkInformation* info = model->GetInformation();
  vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append(info, "face_z1");
  vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append(info, "face_z0");
  vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append(info, "face_z1");
  vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append(info, "face_z0");
  return VTK_OK;
}


//----------------------------------------------------------------------------
int vtkboneApplyDirectionalShearTest::AddInformation
  (
  vtkboneFiniteElementModel* model
  )
{
  std::string history = std::string("Model created by vtkboneApplyDirectionalShearTest version ")
      + vtkboneVersion::GetVTKBONEVersion() + " .";
  model->AppendHistory(history.c_str());

  std::ostringstream comments;
  comments << "vtkboneApplyDirectionalShearTest settings:\n\n";
  vtkIndent indent;
  vtkboneApplyTestBase::PrintParameters(comments, indent);
  this->PrintParameters(comments, indent);
  model->AppendLog(comments.str().c_str());

  return VTK_OK;
}

