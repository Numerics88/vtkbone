#include "vtkboneApplyBendingTest.h"
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
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationStringVectorKey.h"
#include "vtkDataSetAttributes.h"
#include "vtkSmartPointer.h"
#include <sstream>

vtkStandardNewMacro(vtkboneApplyBendingTest);


//----------------------------------------------------------------------------
vtkboneApplyBendingTest::vtkboneApplyBendingTest()
  :
  NeutralAxisAngle (1.5707963267948966),   // pi/2
  BendingAngle (0.017453292519943295)  // One degree.
  {
  this->NeutralAxisOrigin[0] = 0;
  this->NeutralAxisOrigin[1] = 0;
  }


//----------------------------------------------------------------------------
vtkboneApplyBendingTest::~vtkboneApplyBendingTest()
  {
  }


//----------------------------------------------------------------------------
void vtkboneApplyBendingTest::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  this->PrintParameters(os,indent);
  }

//----------------------------------------------------------------------------
void vtkboneApplyBendingTest::PrintParameters (ostream& os, vtkIndent indent)
  {
  os << indent << "NeutralAxisOrigin: " << this->NeutralAxisOrigin[0] << ", "
                                        << this->NeutralAxisOrigin[1] << "\n";
  os << indent << "NeutralAxisAngle: " << this->NeutralAxisAngle << "\n";
  os << indent << "BendingAngle: " << this->BendingAngle << "\n";
  }


//----------------------------------------------------------------------------
int vtkboneApplyBendingTest::RequestData
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
          (this->AddConvergenceSet(output) == VTK_OK) &&
          (this->AddPostProcessingSets(output) == VTK_OK) &&
          (this->AddInformation(output) == VTK_OK));
  }

//----------------------------------------------------------------------------
int vtkboneApplyBendingTest::AddTopAndBottomConstraints
  (
  vtkboneFiniteElementModel* model
  )
  {
  double bounds[6];
  model->GetBounds(bounds);

  // constrain bottom and top surfaces to move only in Z
  model->ApplyBoundaryCondition(
      "face_z0", this->DataFrameSense(0), 0, "bottom_fixed");
  model->ApplyBoundaryCondition(
      "face_z0", this->DataFrameSense(1), 0, "bottom_fixed");
  model->ApplyBoundaryCondition(
      "face_z1", this->DataFrameSense(0), 0, "top_fixed");
  model->ApplyBoundaryCondition(
      "face_z1", this->DataFrameSense(1), 0, "top_fixed");

   double nao[2];   // Neutral Axis origin
   nao[0] = this->NeutralAxisOrigin[0];
   nao[1] = this->NeutralAxisOrigin[1];
   // cout << "nao: " << nao[0] << ", " << nao[1] << "\n";
   double nau[2];   // Neutral Axis unit vector
   nau[0] = cos(this->NeutralAxisAngle);
   nau[1] = sin(this->NeutralAxisAngle);
   // cout << "nau: " << nau[0] << ", " << nau[1] << "\n";
   double deflectionSlope = tan(this->BendingAngle/2.0);

    // Top nodes bending constraint
    { // scope
    vtkSmartPointer<vtkIdTypeArray> nodes = vtkSmartPointer<vtkIdTypeArray>::New();
    vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
    senses->SetName("SENSE");
    vtkSmartPointer<vtkDoubleArray> values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    vtkIdTypeArray* faceTopNodes = model->GetNodeSet("face_z1");
    for (vtkIdType i=0; i < faceTopNodes->GetNumberOfTuples(); i++)
      {
      vtkIdType iNode = faceTopNodes->GetValue(i);
      double* ptDataFrame = model->GetPoint(iNode);
      double pt[2];
      pt[0] = ptDataFrame[this->DataFrameSense(0)];
      pt[1] = ptDataFrame[this->DataFrameSense(1)];
      // cout << "pt: " << pt[0] << ", " << pt[1] << "\n";
      nodes->InsertNextValue (iNode);
      senses->InsertNextValue (this->DataFrameSense(2));
      double dot_product = (pt[0]-nao[0])*nau[0] + (pt[1]-nao[1])*nau[1];
      double projectionOntoLine[2];
      projectionOntoLine[0] = nao[0] + dot_product*nau[0];
      projectionOntoLine[1] = nao[1] + dot_product*nau[1];
      // cout << "projectionOntoLine: " << projectionOntoLine[0] << ", " << projectionOntoLine[1] << "\n";
      double normalVector[2];
      normalVector[0] = pt[0] - projectionOntoLine[0];
      normalVector[1] = pt[1] - projectionOntoLine[1];
      // cout << "normalVector: " << normalVector[0] << ", " << normalVector[1] << "\n";
      // z component of the cross-product
      double distanceFromNeutralAxis = normalVector[0]*nau[1]
                                     - normalVector[1]*nau[0];
      // cout << "distanceFromNeutralAxis: " << distanceFromNeutralAxis << "\n";
      values->InsertNextValue(deflectionSlope*distanceFromNeutralAxis);
      }
    model->ApplyBoundaryCondition(nodes, senses, values, "top_displacement");
    } // scope

    // Bottom nodes bending constraint
    { // scope
    vtkSmartPointer<vtkIdTypeArray> nodes = vtkSmartPointer<vtkIdTypeArray>::New();
    vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
    senses->SetName("SENSE");
    vtkSmartPointer<vtkDoubleArray> values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    vtkIdTypeArray* faceTopNodes = model->GetNodeSet("face_z0");
    for (vtkIdType i=0; i < faceTopNodes->GetNumberOfTuples(); i++)
      {
      vtkIdType iNode = faceTopNodes->GetValue(i);
      double* ptDataFrame = model->GetPoint(iNode);
      double pt[2];
      pt[0] = ptDataFrame[this->DataFrameSense(0)];
      pt[1] = ptDataFrame[this->DataFrameSense(1)];
      // cout << "pt: " << pt[0] << ", " << pt[1] << "\n";
      nodes->InsertNextValue (iNode);
      senses->InsertNextValue (this->DataFrameSense(2));
      double dot_product = (pt[0]-nao[0])*nau[0] + (pt[1]-nao[1])*nau[1];
      double projectionOntoLine[2];
      projectionOntoLine[0] = nao[0] + dot_product*nau[0];
      projectionOntoLine[1] = nao[1] + dot_product*nau[1];
      // cout << "projectionOntoLine: " << projectionOntoLine[0] << ", " << projectionOntoLine[1] << "\n";
      double normalVector[2];
      normalVector[0] = pt[0] - projectionOntoLine[0];
      normalVector[1] = pt[1] - projectionOntoLine[1];
      // cout << "normalVector: " << normalVector[0] << ", " << normalVector[1] << "\n";
      // z component of the cross-product
      double distanceFromNeutralAxis = normalVector[0]*nau[1]
                                     - normalVector[1]*nau[0];
      // cout << "distanceFromNeutralAxis: " << distanceFromNeutralAxis << "\n";
      values->InsertNextValue(-deflectionSlope*distanceFromNeutralAxis);
      }
    model->ApplyBoundaryCondition(nodes, senses, values, "bottom_displacement");
    } // scope
  
  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneApplyBendingTest::AddConvergenceSet
  (
  vtkboneFiniteElementModel* model
  )
  {
  return model->ConvergenceSetFromConstraint("top_displacement");
  }

//----------------------------------------------------------------------------
int vtkboneApplyBendingTest::AddPostProcessingSets
  (
  vtkboneFiniteElementModel* model
  )
  {
  vtkInformation* info = model->GetInformation();
  vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append(info, "face_z1");
  vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append(info, "face_z0");
  vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append(info, "face_z1");
  vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append(info, "face_z0");
  double rotationCenter[3];
  for (int i=0; i<3; ++i)
    {
    int testFrameSense = this->TestFrameSense(i);
    if (testFrameSense < 2)
      { rotationCenter[i] = this->NeutralAxisOrigin[testFrameSense]; }
    else
      {
      double bounds[6];
      model->GetBounds(bounds);
      rotationCenter[i] = (bounds[2*i] + bounds[2*i+1])/2;
      }
    }
  vtkboneSolverParameters::ROTATION_CENTER()->Set(info, rotationCenter, 3);
  return VTK_OK;
  }


//----------------------------------------------------------------------------
int vtkboneApplyBendingTest::AddInformation
  (
  vtkboneFiniteElementModel* model
  )
  {
	std::string history = std::string("Model created by vtkboneApplyBendingTest version ")
      + vtkboneVersion::GetVTKBONEVersion() + " .";	   
  model->AppendHistory(history.c_str());

  std::ostringstream comments;
  comments << "vtkboneApplyBendingTest settings:\n\n";
  vtkIndent indent;
  vtkboneApplyTestBase::PrintParameters(comments, indent);
  this->PrintParameters(comments, indent);
  model->AppendLog(comments.str().c_str());
  
  return VTK_OK;
  }

