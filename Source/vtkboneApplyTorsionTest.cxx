#include "vtkboneApplyTorsionTest.h"
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

vtkStandardNewMacro(vtkboneApplyTorsionTest);


//----------------------------------------------------------------------------
vtkboneApplyTorsionTest::vtkboneApplyTorsionTest()
  :
  TwistAngle (0.017453292519943295)  // One degree.
  {
  this->TwistAxisOrigin[0] = 0;
  this->TwistAxisOrigin[1] = 0;
  }


//----------------------------------------------------------------------------
vtkboneApplyTorsionTest::~vtkboneApplyTorsionTest()
  {
  }


//----------------------------------------------------------------------------
void vtkboneApplyTorsionTest::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  this->PrintParameters(os,indent);
  }

//----------------------------------------------------------------------------
void vtkboneApplyTorsionTest::PrintParameters (ostream& os, vtkIndent indent)
  {
  os << indent << "TwistAxisOrigin: " << this->TwistAxisOrigin[0] << ", "
                                        << this->TwistAxisOrigin[1] << "\n";
  os << indent << "TwistAngle: " << this->TwistAngle << "\n";
  }


//----------------------------------------------------------------------------
int vtkboneApplyTorsionTest::RequestData
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
int vtkboneApplyTorsionTest::AddTopAndBottomConstraints
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

   double tao[2];   // Torsion Axis origin
   tao[0] = this->TwistAxisOrigin[0];
   tao[1] = this->TwistAxisOrigin[1];
   // 2D Rotation matrix
   double R00 = cos(this->TwistAngle);
   double R01 = -sin(this->TwistAngle);
   double R10 = -R01;
   double R11 = R00;

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
      double s[2];
      s[0] = ptDataFrame[this->DataFrameSense(0)] - tao[0];
      s[1] = ptDataFrame[this->DataFrameSense(1)] - tao[1];
      double new_s[2];
      new_s[0] = R00*s[0] + R01*s[1];
      new_s[1] = R10*s[0] + R11*s[1];
      nodes->InsertNextValue(iNode);
      senses->InsertNextValue(this->DataFrameSense(0));
      values->InsertNextValue(new_s[0] - s[0]);
      nodes->InsertNextValue(iNode);
      senses->InsertNextValue(this->DataFrameSense(1));
      values->InsertNextValue(new_s[1] - s[1]);
      }
    model->ApplyBoundaryCondition(nodes, senses, values, "top_displacement");
    } // scope

  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneApplyTorsionTest::AddConvergenceSet
  (
  vtkboneFiniteElementModel* model
  )
  {
  return model->ConvergenceSetFromConstraint(
           model->GetConstraints()->GetItem("top_displacement"));
  }

//----------------------------------------------------------------------------
int vtkboneApplyTorsionTest::AddPostProcessingSets
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
      { rotationCenter[i] = this->TwistAxisOrigin[testFrameSense]; }
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
int vtkboneApplyTorsionTest::AddInformation
  (
  vtkboneFiniteElementModel* model
  )
  {
  std::string history = std::string("Model created by vtkboneApplyTorsionTest version ")
      + vtkboneVersion::GetVTKBONEVersion() + " .";
  model->AppendHistory(history.c_str());

  std::ostringstream comments;
  comments << "vtkboneApplyTorsionTest settings:\n\n";
  vtkIndent indent;
  vtkboneApplyTestBase::PrintParameters(comments, indent);
  this->PrintParameters(comments, indent);
  model->AppendLog(comments.str().c_str());
  
  return VTK_OK;
  }

