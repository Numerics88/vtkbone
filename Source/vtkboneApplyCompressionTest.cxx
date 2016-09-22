#include "vtkboneApplyCompressionTest.h"
#include "vtkboneSolverParameters.h"
#include "vtkboneConstraintCollection.h"
#include "vtkboneVersion.h"
#include "vtkObjectFactory.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkIdList.h"
#include "vtkboneNodeSetsByGeometry.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationStringVectorKey.h"
#include "vtkSmartPointer.h"
#include <sstream>
#include <limits>

vtkStandardNewMacro(vtkboneApplyCompressionTest);

template <typename T> inline T sqr(T x) {return x*x;}

//----------------------------------------------------------------------------
vtkboneApplyCompressionTest::vtkboneApplyCompressionTest()
  :
  AppliedStrain (-0.01),
  AppliedDisplacement (0.0),
  TopSurfaceContactFriction (0),
  BottomSurfaceContactFriction (0),
  ConfineSides (0),
  Pin (0),
  PinCellClosestToXYCenter (1),
  PinCellId (0)
  {
  }


//----------------------------------------------------------------------------
vtkboneApplyCompressionTest::~vtkboneApplyCompressionTest()
  {
  }


//----------------------------------------------------------------------------
void vtkboneApplyCompressionTest::PrintParameters (ostream& os, vtkIndent indent)
  {
  os << indent << "AppliedStrain: " << this->AppliedStrain << "\n";
  os << indent << "AppliedDisplacement: " << this->AppliedDisplacement << "\n";
  os << indent << "TopSurfaceContactFriction: " << this->TopSurfaceContactFriction << "\n";
  os << indent << "BottomSurfaceContactFriction: " << this->BottomSurfaceContactFriction << "\n";
  os << indent << "ConfineSides: " << this->ConfineSides << "\n";
  os << indent << "Pin: " << this->Pin << "\n";
  os << indent << "PinCellClosestToXYCenter: " << this->PinCellClosestToXYCenter << "\n";
  os << indent << "PinCellId: " << this->PinCellId << "\n";
  }


//----------------------------------------------------------------------------
void vtkboneApplyCompressionTest::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  this->PrintParameters(os, indent);
  }


//----------------------------------------------------------------------------
int vtkboneApplyCompressionTest::RequestData
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
          (this->AddSideConstraints(output) == VTK_OK) &&
          (this->AddConvergenceSet(output) == VTK_OK) &&
          (this->AddPostProcessingSets(output) == VTK_OK) &&
          (this->AddInformation(output) == VTK_OK));
  }

//----------------------------------------------------------------------------
int vtkboneApplyCompressionTest::PinElement
  (
  vtkboneFiniteElementModel* model
  )
  {
  if (this->PinCellClosestToXYCenter)
    {
    this->PinCellId = this->GetCellClosestToXYCenter(model);
    }
  // Get cell 0
  vtkSmartPointer<vtkIdList> cell0PointList = vtkSmartPointer<vtkIdList>::New();
  model->GetCellPoints(this->PinCellId, cell0PointList);
  model->ApplyBoundaryCondition(
      cell0PointList->GetId(this->GetVoxelLocalId(0)),
      this->DataFrameSense(0),
      0,
      "pinned_nodes");
  model->ApplyBoundaryCondition(
      cell0PointList->GetId(this->GetVoxelLocalId(0)),
      this->DataFrameSense(1),
      0,
      "pinned_nodes");
  // Point 1 of cell 0, which is along the x axis in the Test Frame,
  // is constrained to no motion in y direction (in the Test Frame) 
  // to prevent rotation.
  model->ApplyBoundaryCondition(
      cell0PointList->GetId(this->GetVoxelLocalId(1)),
      this->DataFrameSense(1),
      0,
      "pinned_nodes");
  return VTK_OK;
  }

//----------------------------------------------------------------------------
vtkIdType vtkboneApplyCompressionTest::GetCellClosestToXYCenter
  (
  vtkboneFiniteElementModel* model
  )
  {
  double bounds[6];
    // x,y,z are in Test Frame.
  model->GetBounds(bounds);
  double x_center = (TestFrameBound(bounds, 0, 0) + TestFrameBound(bounds, 0, 1))/2;
  double y_center = (TestFrameBound(bounds, 1, 0) + TestFrameBound(bounds, 1, 1))/2;
  double z_center = (TestFrameBound(bounds, 2, 0) + TestFrameBound(bounds, 2, 1))/2;
  double r2_closest = std::numeric_limits<double>::max();
  double z_min = std::numeric_limits<double>::max();
  double bestCellId = 0;
  for (vtkIdType id=0; id<model->GetNumberOfCells(); id++)
    {
    double cellBounds[6];
    model->GetCellBounds(id, cellBounds);
    double x = (TestFrameBound(cellBounds, 0, 0) + TestFrameBound(cellBounds, 0, 1))/2;
    double y = (TestFrameBound(cellBounds, 1, 0) + TestFrameBound(cellBounds, 1, 1))/2;
    double z = (TestFrameBound(cellBounds, 2, 0) + TestFrameBound(cellBounds, 2, 1))/2;
    double r2 = sqr(x-x_center) + sqr(y-y_center);
    if ((r2 == r2_closest && z < z_min) || (r2 < r2_closest))
      {
      bestCellId = id;
      r2_closest = r2;
      z_min = z;
      }
    }
  return bestCellId;
  }

//----------------------------------------------------------------------------
int vtkboneApplyCompressionTest::AddTopAndBottomConstraints
  (
  vtkboneFiniteElementModel* model
  )
  {
  double bounds[6];
  model->GetBounds(bounds);

  if (this->BottomSurfaceContactFriction)
    {
    // Fixed constraints on bottom in all directions
    model->FixNodes("face_z0", "bottom_fixed");
    }
  else
    {
    // Fixed constraints on bottom in z direction
    model->ApplyBoundaryCondition(
        "face_z0", this->DataFrameSense(2), 0, "bottom_fixed");
    }

  // Need pin if no fixed nodes in x,y directions.
  // Also fix another point in same cell in one direction only to prevent rotation.
  if (this->Pin &&
      !this->BottomSurfaceContactFriction &&
      !this->TopSurfaceContactFriction &&
      !this->ConfineSides)
    {
    this->PinElement(model);
    }

  if (this->TopSurfaceContactFriction)
    {
    model->ApplyBoundaryCondition(
        "face_z1", this->DataFrameSense(0), 0, "top_fixed");
    model->ApplyBoundaryCondition(
        "face_z1", this->DataFrameSense(1), 0, "top_fixed");
    }

  double displacement;
  if (this->AppliedDisplacement == 0)
    {
    displacement = (this->TestFrameBound(bounds,2,1) - this->TestFrameBound(bounds,2,0))
                     * this->AppliedStrain;
    }
  else
    {
    displacement = this->AppliedDisplacement;
    }
  model->ApplyBoundaryCondition(
      "face_z1", this->DataFrameSense(2), displacement, "top_displacement");

   return VTK_OK;
   }

//----------------------------------------------------------------------------
int vtkboneApplyCompressionTest::AddSideConstraints
  (
  vtkboneFiniteElementModel* model
  )
  {
  if (!this->ConfineSides)
    { return VTK_OK; }

  model->ApplyBoundaryCondition(
      "face_x0", this->DataFrameSense(0), 0, "face_x0_fixed");
  model->ApplyBoundaryCondition(
      "face_x1", this->DataFrameSense(0), 0, "face_x1_fixed");
  model->ApplyBoundaryCondition(
      "face_y0", this->DataFrameSense(1), 0, "face_y0_fixed");
  model->ApplyBoundaryCondition(
      "face_y1", this->DataFrameSense(1), 0, "face_y1_fixed");
  
  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneApplyCompressionTest::AddConvergenceSet
  (
  vtkboneFiniteElementModel* model
  )
  {
  return model->ConvergenceSetFromConstraint("top_displacement");
  }

//----------------------------------------------------------------------------
int vtkboneApplyCompressionTest::AddPostProcessingSets
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
int vtkboneApplyCompressionTest::AddInformation
  (
  vtkboneFiniteElementModel* model
  )
  {
  std::string history = std::string("Model created by vtkboneApplyCompressionTest version ")
      + vtkboneVersion::GetVTKBONEVersion() + " .";
  model->AppendHistory(history.c_str());

  std::ostringstream comments;
  comments << "vtkboneApplyCompressionTest settings:\n\n";
  vtkIndent indent;
  vtkboneApplyTestBase::PrintParameters(comments, indent);
  this->PrintParameters(comments, indent);
  model->AppendLog(comments.str().c_str());
  
  return VTK_OK;
  }

