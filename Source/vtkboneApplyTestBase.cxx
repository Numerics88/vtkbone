#include "vtkboneApplyTestBase.h"
#include "vtkboneNodeSetsByGeometry.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkInformationStringKey.h"
#include "vtkBox.h"
#include "vtkExtractGeometry.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkboneMaterialTable.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkboneSelectionUtilities.h"
#include "vtkSmartPointer.h"
#include "vtkboneMacros.h"
#include <cassert>

vtkStandardNewMacro(vtkboneApplyTestBase);

const int VoxelLocalIdTestAxisX[8] = {0, 2, 4, 6, 1, 3, 5, 7};
const int VoxelLocalIdTestAxisY[8] = {0, 4, 1, 5, 2, 6, 3, 7};

//----------------------------------------------------------------------------
vtkboneApplyTestBase::vtkboneApplyTestBase()
  :
  TopConstraintSpecificMaterial (-1),
  BottomConstraintSpecificMaterial (-1),
  UnevenTopSurface (0),
  UseTopSurfaceMaximumDepth (0),
  TopSurfaceMaximumDepth (0),
  UnevenBottomSurface (0),
  UseBottomSurfaceMaximumDepth (0),
  BottomSurfaceMaximumDepth (0),
  TestAxis (2)
  {
  }

//----------------------------------------------------------------------------
vtkboneApplyTestBase::~vtkboneApplyTestBase()
  {
  }

//----------------------------------------------------------------------------
void vtkboneApplyTestBase::PrintParameters (ostream& os, vtkIndent indent)
{
  os << indent << "TopConstraintSpecificMaterial: " << this->TopConstraintSpecificMaterial << "\n";
  os << indent << "BottomConstraintSpecificMaterial: " << this->BottomConstraintSpecificMaterial << "\n";
  os << indent << "UnevenTopSurface: " << this->UnevenTopSurface << "\n";
  if (UnevenTopSurface)
    {
    os << indent << "UseTopSurfaceMaximumDepth: " << this->UseTopSurfaceMaximumDepth << "\n";
    if (UseTopSurfaceMaximumDepth)
      {
      os << indent << "TopSurfaceMaximumDepth: " << this->TopSurfaceMaximumDepth << "\n";
      }
    }
  os << indent << "UnevenBottomSurface: " << this->UnevenBottomSurface << "\n";
  if (UnevenBottomSurface)
    {
    os << indent << "UseBottomSurfaceMaximumDepth: " << this->UseBottomSurfaceMaximumDepth << "\n";
    if (UseBottomSurfaceMaximumDepth)
      {
      os << indent << "BottomSurfaceMaximumDepth: " << this->BottomSurfaceMaximumDepth << "\n";
      }
    }
  os << indent << "TestAxis: " << this->TestAxis << "\n";
}

//----------------------------------------------------------------------------
void vtkboneApplyTestBase::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  this->PrintParameters(os,indent);
}

//----------------------------------------------------------------------------
int vtkboneApplyTestBase::DataFrameSense(int testFrameSense)
  {
  // The -2 comes from having a default testing axis of 2.
  int new_sense = (testFrameSense + this->TestAxis - 2) % 3;
  return (new_sense < 0) ? (3+new_sense) : new_sense;
  }

//----------------------------------------------------------------------------
int vtkboneApplyTestBase::TestFrameSense(int dataFrameSense)
  {
  // The +2 comes from having a default testing axis of 2.
  int new_sense = (dataFrameSense - this->TestAxis + 2) % 3;
  return (new_sense < 0) ? (3+new_sense) : new_sense;
  }

//----------------------------------------------------------------------------
int vtkboneApplyTestBase::DataFramePolarity(int testFrameSense, int polarity)
  {
  return polarity;
  }

//----------------------------------------------------------------------------
double vtkboneApplyTestBase::TestFrameBound
  (
  double bounds[6],
  int testFrameSense,
  int polarity
  )
  {
  int dfSense = this->DataFrameSense(testFrameSense);
  int dfPolarity = this->DataFramePolarity(testFrameSense, polarity);
  return bounds[2*dfSense + dfPolarity];
  }

//----------------------------------------------------------------------------
int vtkboneApplyTestBase::GetVoxelLocalId(int testFrameLocalId)
{
  if (this->TestAxis == 2)
    {
    return testFrameLocalId;
    }
  else if (this->TestAxis == 0)
    {
    return VoxelLocalIdTestAxisX[testFrameLocalId];
    }
  else if (this->TestAxis == 1)
    {
    return VoxelLocalIdTestAxisY[testFrameLocalId];
    }
  else
    {
    std::cerr << "WARNING: Invalid value for TestAxis\n";
    return -1;
    }
}


//----------------------------------------------------------------------------
// Reminder: child classes may override this if they want different sets.
int vtkboneApplyTestBase::AddSets
  (
  vtkboneFiniteElementModel* model
  )
  {
  if ((this->AddDataFrameZFacesSets(model) == VTK_OK) &&
      (this->AddDataFrameXFacesSets(model) == VTK_OK) &&
      (this->AddDataFrameYFacesSets(model) == VTK_OK))
    { return VTK_OK; }
  else
    { return VTK_ERROR; }
  }

//----------------------------------------------------------------------------
int vtkboneApplyTestBase::AddDataFrameZFacesSets
  (
  vtkboneFiniteElementModel* model
  )
  {
  double bounds[6];
  model->GetBounds(bounds);

  if (this->UnevenBottomSurface)
    {
    double direction[3] = {0.0, 0.0, 0.0};  // normal vector of surface
    direction[this->DataFrameSense(2)] = this->DataFramePolarity(2,0) ? 1 : -1;
    if (this->UseBottomSurfaceMaximumDepth)
      {
      double box_bounds[6];
      model->GetBounds(box_bounds);
      int data_frame_bottom_bound_index = 2*this->DataFrameSense(2) + this->DataFramePolarity(2,0);
      int data_frame_top_bound_index = 2*this->DataFrameSense(2) + this->DataFramePolarity(2,1);
      box_bounds[data_frame_top_bound_index] = box_bounds[data_frame_bottom_bound_index]
                                             + this->BottomSurfaceMaximumDepth;
      vtkSmartPointer<vtkBox> box = vtkSmartPointer<vtkBox>::New();
      box->SetBounds(box_bounds);
      vtkSmartPointer<vtkExtractGeometry> filter = vtkSmartPointer<vtkExtractGeometry>::New();
      filter->SetImplicitFunction(box);
      filter->ExtractInsideOn();
      filter->ExtractBoundaryCellsOn();
      // Need to copy model, because using model as an input to a filter
      // causes an infinite update loop (we are currently inside a filter
      // constructing model).
      vtkSmartPointer<vtkUnstructuredGrid> geometry = vtkSmartPointer<vtkUnstructuredGrid>::New();
      geometry->ShallowCopy(model);
      filter->SetInputData(geometry);
      filter->Update();
      vtkUnstructuredGrid* depth_filtered_dataset = vtkUnstructuredGrid::SafeDownCast(filter->GetOutput());
      assert(depth_filtered_dataset);
      vtkSmartPointer<vtkIdTypeArray> visibleNodesIds = vtkSmartPointer<vtkIdTypeArray>::New();
      vtkboneNodeSetsByGeometry::FindNodesOnVisibleSurface(
        visibleNodesIds,
        depth_filtered_dataset,
        direction,
        this->BottomConstraintSpecificMaterial);
      visibleNodesIds->SetName ("face_z0");
      model->AddNodeSet(visibleNodesIds);
      vtkSmartPointer<vtkIdTypeArray> elementSet = vtkSmartPointer<vtkIdTypeArray>::New();
      model->GetAssociatedElementsFromNodeSet ("face_z0", elementSet);
      model->AddElementSet(elementSet);
      }
    else
      {
      vtkboneNodeSetsByGeometry::AddNodesAndElementsOnVisibleSurface("face_z0",
                     model, direction, this->BottomConstraintSpecificMaterial);
      }
    }
  else
    {
    vtkboneNodeSetsByGeometry::AddNodesAndElementsOnPlane(
            this->DataFrameSense(2),
            this->TestFrameBound(bounds,2,0),
            "face_z0",
            model,
            this->BottomConstraintSpecificMaterial);
    }

  if (this->UnevenTopSurface)
    {
    double direction[3] = {0.0, 0.0, 0.0};  // normal vector of surface
    direction[this->DataFrameSense(2)] = this->DataFramePolarity(2,1) ? 1 : -1;
    if (this->UseTopSurfaceMaximumDepth)
      {
      double box_bounds[6];
      model->GetBounds(box_bounds);
      int data_frame_bottom_bound_index = 2*this->DataFrameSense(2) + this->DataFramePolarity(2,0);
      int data_frame_top_bound_index = 2*this->DataFrameSense(2) + this->DataFramePolarity(2,1);
      box_bounds[data_frame_bottom_bound_index] = box_bounds[data_frame_top_bound_index]
                                              - this->TopSurfaceMaximumDepth;
      vtkSmartPointer<vtkBox> box = vtkSmartPointer<vtkBox>::New();
      box->SetBounds(box_bounds);
      vtkSmartPointer<vtkExtractGeometry> filter = vtkSmartPointer<vtkExtractGeometry>::New();
      filter->SetImplicitFunction(box);
      filter->ExtractInsideOn();
      filter->ExtractBoundaryCellsOn();
      // Need to copy model, because using model as an input to a filter
      // causes an infinite update loop (we are currently inside a filter
      // constructing model).
      vtkSmartPointer<vtkUnstructuredGrid> geometry = vtkSmartPointer<vtkUnstructuredGrid>::New();
      geometry->ShallowCopy(model);
      filter->SetInputData(geometry);
      filter->Update();
      vtkUnstructuredGrid* depth_filtered_dataset = vtkUnstructuredGrid::SafeDownCast(filter->GetOutput());
      assert(depth_filtered_dataset);
      vtkSmartPointer<vtkIdTypeArray> visibleNodesIds = vtkSmartPointer<vtkIdTypeArray>::New();
      vtkboneNodeSetsByGeometry::FindNodesOnVisibleSurface(
        visibleNodesIds,
        depth_filtered_dataset,
        direction,
        this->TopConstraintSpecificMaterial);
      visibleNodesIds->SetName ("face_z1");
      model->AddNodeSet(visibleNodesIds);
      vtkSmartPointer<vtkIdTypeArray> elementSet = vtkSmartPointer<vtkIdTypeArray>::New();
      model->GetAssociatedElementsFromNodeSet ("face_z1", elementSet);
      model->AddElementSet(elementSet);
      }
    else
      {
      vtkboneNodeSetsByGeometry::AddNodesAndElementsOnVisibleSurface("face_z1",
                        model, direction, this->TopConstraintSpecificMaterial);
      }
    }
  else
    {
    vtkboneNodeSetsByGeometry::AddNodesAndElementsOnPlane(
            this->DataFrameSense(2),
            this->TestFrameBound(bounds,2,1),
            "face_z1",
            model,
            this->TopConstraintSpecificMaterial);
    }
  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneApplyTestBase::AddDataFrameXFacesSets
  (
  vtkboneFiniteElementModel* model
  )
  {
  double bounds[6];
  model->GetBounds(bounds);

  int ok =
  (vtkboneNodeSetsByGeometry::AddNodesAndElementsOnPlane(
          this->DataFrameSense(0),
          this->TestFrameBound(bounds,0,0),
          "face_x0",
          model) == VTK_OK) &&
  (vtkboneNodeSetsByGeometry::AddNodesAndElementsOnPlane(
          this->DataFrameSense(0),
          this->TestFrameBound(bounds,0,1),
          "face_x1",
          model) == VTK_OK);
  if (ok)
    { return VTK_OK; }
  else
    { return VTK_ERROR; }
  }

//----------------------------------------------------------------------------
int vtkboneApplyTestBase::AddDataFrameYFacesSets
  (
  vtkboneFiniteElementModel* model
  )
  {
  double bounds[6];
  model->GetBounds(bounds);

  int ok =
  (vtkboneNodeSetsByGeometry::AddNodesAndElementsOnPlane(
          this->DataFrameSense(1),
          this->TestFrameBound(bounds,1,0),
          "face_y0",
          model) == VTK_OK) &&
  (vtkboneNodeSetsByGeometry::AddNodesAndElementsOnPlane(
          this->DataFrameSense(1),
          this->TestFrameBound(bounds,1,1),
          "face_y1",
          model) == VTK_OK);
  if (ok)
    { return VTK_OK; }
  else
    { return VTK_ERROR; }
  }

//----------------------------------------------------------------------------
int vtkboneApplyTestBase::RequestData
(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector
)
{
  vtkboneFiniteElementModelGenerator::RequestData(request, inputVector, outputVector);

  // Only need output object: vtkboneFiniteElementModelGenerator has already copied the input
  // object to the output object (and added stuff).
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkboneFiniteElementModel *output = vtkboneFiniteElementModel::SafeDownCast(
                            outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
    {
    vtkErrorMacro("No output object.");
    return 0;
    }

  if (output->GetNumberOfCells() == 0)
    {
    vtkWarningMacro("Zero elements on input to vtkboneApplyTestBase");
    return 0;
    }

  // Checks and setup.
  int elementType = output->GetElementType();
  if (elementType != vtkboneFiniteElementModel::N88_TETRAHEDRON &&
      elementType != vtkboneFiniteElementModel::N88_HEXAHEDRON)
    {
    vtkErrorMacro(<<"The mesher currently is designed for either a pure tetrahedron"
                  <<" or hexahedron mesh.  To make it work for mixed meshes, the "
                  <<" application of force boundary conditions would have to be modified.");
    return 0;
    }

  return (this->AddSets(output) == VTK_OK);
}
