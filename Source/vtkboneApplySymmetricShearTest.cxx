#include "vtkboneApplySymmetricShearTest.h"
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

vtkStandardNewMacro(vtkboneApplySymmetricShearTest);


//----------------------------------------------------------------------------
vtkboneApplySymmetricShearTest::vtkboneApplySymmetricShearTest()
  :
  ShearStrain (0.01),
  ConfineSidesVertically (1),
  ConfineTopAndBottomVertically (1)
  {}


//----------------------------------------------------------------------------
vtkboneApplySymmetricShearTest::~vtkboneApplySymmetricShearTest()
  {}


//----------------------------------------------------------------------------
void vtkboneApplySymmetricShearTest::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  this->PrintParameters(os,indent);
}

//----------------------------------------------------------------------------
void vtkboneApplySymmetricShearTest::PrintParameters (ostream& os, vtkIndent indent)
{
  os << indent << "ShearStrain: " << this->ShearStrain << "\n";
  os << indent << "ConfineSidesVertically: " << this->ConfineSidesVertically << "\n";
  os << indent << "ConfineTopAndBottomVertically: " << this->ConfineTopAndBottomVertically << "\n";
}


//----------------------------------------------------------------------------
int vtkboneApplySymmetricShearTest::RequestData
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

  return ((this->AddSideConstraints(output) == VTK_OK) &&
          (this->AddTopAndBottomConstraints(output) == VTK_OK) &&
          (this->AddConvergenceSet (output) == VTK_OK) &&
          (this->AddPostProcessingSets(output) == VTK_OK) &&
          (this->AddInformation(output) == VTK_OK));
}

//----------------------------------------------------------------------------
int vtkboneApplySymmetricShearTest::AddSideConstraints
  (
  vtkboneFiniteElementModel* model
  )
{
  double bounds[6];
  model->GetBounds(bounds);

  double dx_factor = (0.5*this->ShearStrain);
  double dy_factor = (0.5*this->ShearStrain);

  // face 00
  {  // scope
    vtkIdTypeArray* singleIds = model->GetNodeSet("face_x0");
    vtkIdType n = singleIds->GetNumberOfTuples();
    vtkSmartPointer<vtkIdTypeArray> doubledIds = vtkSmartPointer<vtkIdTypeArray>::New();
    doubledIds->SetNumberOfValues(2*n);
    vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
    senses->SetName("SENSE");
    senses->SetNumberOfValues(2*n);
    vtkSmartPointer<vtkDoubleArray> values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    values->SetNumberOfValues(2*n);
    for (vtkIdType i=0; i<n; ++i)
    {
      vtkIdType id = singleIds->GetValue(i);
      doubledIds->SetValue(2*i, id);
      doubledIds->SetValue(2*i+1, id);
      double* dataFrameCoords = model->GetPoint(id);
      double testFrameCoords[2];
      testFrameCoords[0] = dataFrameCoords[this->DataFrameSense(0)];
      testFrameCoords[1] = dataFrameCoords[this->DataFrameSense(1)];
      senses->SetValue(2*i, this->DataFrameSense(0));
      senses->SetValue(2*i+1, this->DataFrameSense(1));
      double dx = dx_factor
                  * (testFrameCoords[1] - this->TestFrameBound(bounds,1,0));
      values->SetValue(2*i, dx);
      values->SetValue(2*i+1, 0);
    }
    vtkSmartPointer<vtkboneConstraint> constraint = vtkSmartPointer<vtkboneConstraint>::New();
    constraint->SetName("face_x0_lateral");
    constraint->SetIndices(doubledIds);
    constraint->SetConstraintType(vtkboneConstraint::DISPLACEMENT);
    constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
    constraint->GetAttributes()->AddArray(senses);
    constraint->GetAttributes()->AddArray(values);
    model->GetConstraints()->AddItem(constraint);
  }  // end scope

  // face 01
  {  // scope
    vtkIdTypeArray* singleIds = model->GetNodeSet("face_x1");
    vtkIdType n = singleIds->GetNumberOfTuples();
    vtkSmartPointer<vtkIdTypeArray> doubledIds = vtkSmartPointer<vtkIdTypeArray>::New();
    doubledIds->SetNumberOfValues(2*n);
    vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
    senses->SetName("SENSE");
    senses->SetNumberOfValues(2*n);
    vtkSmartPointer<vtkDoubleArray> values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    values->SetNumberOfValues(2*n);
    double dy = (0.5*this->ShearStrain)
       * (this->TestFrameBound(bounds,0,1) - this->TestFrameBound(bounds,0,0));
    for (vtkIdType i=0; i<n; ++i)
    {
      vtkIdType id = singleIds->GetValue(i);
      doubledIds->SetValue(2*i, id);
      doubledIds->SetValue(2*i+1, id);
      double* dataFrameCoords = model->GetPoint(id);
      double testFrameCoords[2];
      testFrameCoords[0] = dataFrameCoords[this->DataFrameSense(0)];
      testFrameCoords[1] = dataFrameCoords[this->DataFrameSense(1)];
      senses->SetValue(2*i, this->DataFrameSense(0));
      senses->SetValue(2*i+1, this->DataFrameSense(1));
      double dx = dx_factor
                  * (testFrameCoords[1] - this->TestFrameBound(bounds,1,0));
      values->SetValue(2*i, dx);
      values->SetValue(2*i+1, dy);
    }
    vtkSmartPointer<vtkboneConstraint> constraint = vtkSmartPointer<vtkboneConstraint>::New();
    constraint->SetName("face_x1_lateral");
    constraint->SetIndices(doubledIds);
    constraint->SetConstraintType(vtkboneConstraint::DISPLACEMENT);
    constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
    constraint->GetAttributes()->AddArray(senses);
    constraint->GetAttributes()->AddArray(values);
    model->GetConstraints()->AddItem(constraint);
  }  // end scope

  // face 10
  {  // scope
    vtkIdTypeArray* singleIds = model->GetNodeSet("face_y0");
    vtkIdType n = singleIds->GetNumberOfTuples();
    vtkSmartPointer<vtkIdTypeArray> doubledIds = vtkSmartPointer<vtkIdTypeArray>::New();
    doubledIds->SetNumberOfValues(2*n);
    vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
    senses->SetName("SENSE");
    senses->SetNumberOfValues(2*n);
    vtkSmartPointer<vtkDoubleArray> values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    values->SetNumberOfValues(2*n);
    for (vtkIdType i=0; i<n; ++i)
    {
      vtkIdType id = singleIds->GetValue(i);
      doubledIds->SetValue(2*i, id);
      doubledIds->SetValue(2*i+1, id);
      double* dataFrameCoords = model->GetPoint(id);
      double testFrameCoords[2];
      testFrameCoords[0] = dataFrameCoords[this->DataFrameSense(0)];
      testFrameCoords[1] = dataFrameCoords[this->DataFrameSense(1)];
      senses->SetValue(2*i, this->DataFrameSense(0));
      senses->SetValue(2*i+1, this->DataFrameSense(1));
      double dy = dy_factor
                  * (testFrameCoords[0] - this->TestFrameBound(bounds,0,0));
      values->SetValue(2*i, 0);
      values->SetValue(2*i+1, dy);
    }
    vtkSmartPointer<vtkboneConstraint> constraint = vtkSmartPointer<vtkboneConstraint>::New();
    constraint->SetName("face_y0_lateral");
    constraint->SetIndices(doubledIds);
    constraint->SetConstraintType(vtkboneConstraint::DISPLACEMENT);
    constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
    constraint->GetAttributes()->AddArray(senses);
    constraint->GetAttributes()->AddArray(values);
    model->GetConstraints()->AddItem(constraint);
  }  // end scope

  // face 11
  {  // scope
    vtkIdTypeArray* singleIds = model->GetNodeSet("face_y1");
    vtkIdType n = singleIds->GetNumberOfTuples();
    vtkSmartPointer<vtkIdTypeArray> doubledIds = vtkSmartPointer<vtkIdTypeArray>::New();
    doubledIds->SetNumberOfValues(2*n);
    vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
    senses->SetName("SENSE");
    senses->SetNumberOfValues(2*n);
    vtkSmartPointer<vtkDoubleArray> values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    values->SetNumberOfValues(2*n);
    double dx = (0.5*this->ShearStrain)
       * (this->TestFrameBound(bounds,1,1) - this->TestFrameBound(bounds,1,0));
    for (vtkIdType i=0; i<n; ++i)
    {
      vtkIdType id = singleIds->GetValue(i);
      doubledIds->SetValue(2*i, id);
      doubledIds->SetValue(2*i+1, id);
      double* dataFrameCoords = model->GetPoint(id);
      double testFrameCoords[2];
      testFrameCoords[0] = dataFrameCoords[this->DataFrameSense(0)];
      testFrameCoords[1] = dataFrameCoords[this->DataFrameSense(1)];
      senses->SetValue(2*i, this->DataFrameSense(0));
      senses->SetValue(2*i+1, this->DataFrameSense(1));
      double dy = dy_factor
                  * (testFrameCoords[0] - this->TestFrameBound(bounds,0,0));
      values->SetValue(2*i, dx);
      values->SetValue(2*i+1, dy);
    }
    vtkSmartPointer<vtkboneConstraint> constraint = vtkSmartPointer<vtkboneConstraint>::New();
    constraint->SetName("face_y1_lateral");
    constraint->SetIndices(doubledIds);
    constraint->SetConstraintType(vtkboneConstraint::DISPLACEMENT);
    constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
    constraint->GetAttributes()->AddArray(senses);
    constraint->GetAttributes()->AddArray(values);
    model->GetConstraints()->AddItem(constraint);
  }  // end scope

  if (this->ConfineSidesVertically)
  {
    model->ApplyBoundaryCondition(
      "face_x0", this->DataFrameSense(2), 0, "face_x0_vertical");
    model->ApplyBoundaryCondition(
      "face_x1", this->DataFrameSense(2), 0, "face_x1_vertical");
    model->ApplyBoundaryCondition(
      "face_y0", this->DataFrameSense(2), 0, "face_y0_vertical");
    model->ApplyBoundaryCondition(
      "face_y1", this->DataFrameSense(2), 0, "face_y1_vertical");
  }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneApplySymmetricShearTest::AddTopAndBottomConstraints
  (
  vtkboneFiniteElementModel* model
  )
{
  if (this->ConfineTopAndBottomVertically)
  {
    model->ApplyBoundaryCondition(
      "face_z0", this->DataFrameSense(2), 0, "face_z0_vertical");
    model->ApplyBoundaryCondition(
      "face_z1", this->DataFrameSense(2), 0, "face_z1_vertical");
  }
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneApplySymmetricShearTest::AddConvergenceSet
  (
  vtkboneFiniteElementModel* model
  )
{
  return model->ConvergenceSetFromConstraint("face_y0_lateral");
}

//----------------------------------------------------------------------------
int vtkboneApplySymmetricShearTest::AddPostProcessingSets
  (
  vtkboneFiniteElementModel* model
  )
{
  vtkInformation* info = model->GetInformation();
  vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append(info, "face_x0");
  vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append(info, "face_x1");
  vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append(info, "face_y0");
  vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append(info, "face_y1");
  vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append(info, "face_x0");
  vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append(info, "face_x1");
  vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append(info, "face_y0");
  vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append(info, "face_y1");
  return VTK_OK;
}


//----------------------------------------------------------------------------
int vtkboneApplySymmetricShearTest::AddInformation
  (
  vtkboneFiniteElementModel* model
  )
{
  std::string history = std::string("Model created by vtkboneApplySymmetricShearTest version ")
      + vtkboneVersion::GetVTKBONEVersion() + " .";
  model->AppendHistory(history.c_str());

  std::ostringstream comments;
  comments << "vtkboneApplySymmetricShearTest settings:\n\n";
  vtkIndent indent;
  vtkboneApplyTestBase::PrintParameters(comments, indent);
  this->PrintParameters(comments, indent);
  model->AppendLog(comments.str().c_str());

  return VTK_OK;
}

