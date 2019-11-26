#include "vtkboneNodeSetsByGeometry.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkboneSelectVisiblePoints.h"
#include "vtkboneSelectionUtilities.h"
#include "vtkboneOrientationFilter.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkIdTypeArray.h"
#include "vtkIdList.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkCell.h"
#include "vtkCellLocator.h"
#include "vtkGenericCell.h"
#include "vtkPoints.h"
#include "vtkHexahedron.h"
#include "vtkTetra.h"
#include "vtkSpatialRepresentationFilter.h"
#include "vtkMath.h"
#include "vtkPolyData.h"
#include "vtkGeometryFilter.h"
#include "vtkSelectionNode.h"
#include "vtkSelection.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkSmartPointer.h"
#include "vtkExtractSelection.h"
#include "n88util/floating_point_comparisons.hpp"
#include <limits>
#include <assert.h>

vtkStandardNewMacro(vtkboneNodeSetsByGeometry);

//----------------------------------------------------------------------------
void vtkboneNodeSetsByGeometry::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkboneNodeSetsByGeometry::DetermineMaterialBounds
(
  vtkUnstructuredGrid *geometry,
  double bounds[6],
  int specificMaterial
)
{
  using std::numeric_limits;

  if (specificMaterial == -1)
    {
    geometry->GetBounds(bounds);
    }
  else
    {
    bounds[0] =  numeric_limits<double>::infinity();
    bounds[1] = -numeric_limits<double>::infinity();
    bounds[2] =  numeric_limits<double>::infinity();
    bounds[3] = -numeric_limits<double>::infinity();
    bounds[4] =  numeric_limits<double>::infinity();
    bounds[5] = -numeric_limits<double>::infinity();
    vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
    for (vtkIdType i=0; i<geometry->GetNumberOfPoints(); i++)
      {
      geometry->GetPointCells(i,cellIds);
      for (int j=0; j<cellIds->GetNumberOfIds(); j++)
        {
        int material = geometry->GetCellData()->GetScalars()->GetTuple1(cellIds->GetId(j));
        if (material == specificMaterial)
          {
          double point[3];
          geometry->GetPoint(i,point);
          bounds[0] = std::min(bounds[0], point[0]);
          bounds[1] = std::max(bounds[1], point[0]);
          bounds[2] = std::min(bounds[2], point[1]);
          bounds[3] = std::max(bounds[3], point[1]);
          bounds[4] = std::min(bounds[4], point[2]);
          bounds[5] = std::max(bounds[5], point[2]);
          break;
          }
        }
      }
    }

  return;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::FilterPointListByCellScalar
  (
  vtkIdTypeArray *output_ids,
  vtkIdTypeArray *input_ids,
  vtkDataSet *data,
  int targetCellScalar
  )
  {
  output_ids->Initialize();
  output_ids->Allocate(input_ids->GetNumberOfTuples());
  vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
  for (vtkIdType i=0; i<input_ids->GetNumberOfTuples(); i++)
    {
    vtkIdType pointId = input_ids->GetValue(i);
    data->GetPointCells(pointId, cellIds);
    int haveMatch = 0;
    for (int j=0; j<cellIds->GetNumberOfIds(); j++)
      {
      int val = data->GetCellData()->GetScalars()->GetTuple1(cellIds->GetId(j));
      if (val == targetCellScalar)
        {
        haveMatch = 1;
        break;
        }
      }
    if (haveMatch)
      {
      output_ids->InsertNextValue(pointId);
      }
    }
  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::FindNodesOnPlane
(
  int axis,
  float val,
  vtkIdTypeArray *ids,
  vtkUnstructuredGrid *ug,
  int specificMaterial
)
{
  bonelabMisc::SanityCheck();

  vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();

  for (vtkIdType i=0; i<ug->GetNumberOfPoints(); i++) {
    if (bonelabMisc::ApproximatelyEqual(ug->GetPoint(i)[axis],val)) {
      if (specificMaterial == -1) {
        ids->InsertNextValue(i);
      } else {
        ug->GetPointCells(i,cellIds);
        for (int j=0; j<cellIds->GetNumberOfIds(); j++) {
          int material = ug->GetCellData()->GetScalars()->GetTuple1(cellIds->GetId(j));
          if (material == specificMaterial) {
            ids->InsertNextValue(i);
            break;
          }
        }
      }
    }
  }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::AddNodesOnPlane
(
  int axis,
  float val,
  const char* name,
  vtkboneFiniteElementModel *model,
  int specificMaterial
)
{
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetName (name);
  if (FindNodesOnPlane (axis, val, ids, model, specificMaterial) != VTK_OK)
    {
    return VTK_ERROR;
    }
  model->AddNodeSet (ids);
  return VTK_OK;
}


//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::AddNodesAndElementsOnPlane
(
  int axis,
  float val,
  const char* name,
  vtkboneFiniteElementModel *model,
  int specificMaterial
)
{
  if (AddNodesOnPlane (axis, val, name, model, specificMaterial) != VTK_OK)
    { return VTK_ERROR; }
  vtkSmartPointer<vtkIdTypeArray> elementSet = vtkSmartPointer<vtkIdTypeArray>::New();
  model->GetAssociatedElementsFromNodeSet (name, elementSet);
  model->AddElementSet(elementSet);
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::FindNodesIntersectingTwoPlanes
(
  int axis1,
  float val1,
  int axis2,
  float val2,
  vtkIdTypeArray *ids,
  vtkUnstructuredGrid *ug,
  int specificMaterial
)
{
  bonelabMisc::SanityCheck();

  vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();

  for (vtkIdType i=0; i<ug->GetNumberOfPoints(); i++) {
    if (bonelabMisc::ApproximatelyEqual(ug->GetPoint(i)[axis1], val1)) {
      if (bonelabMisc::ApproximatelyEqual(ug->GetPoint(i)[axis2], val2)) {
        if (specificMaterial == -1) {
          ids->InsertNextValue(i);
        } else {
          ug->GetPointCells(i,cellIds);
          for (int j=0; j<cellIds->GetNumberOfIds(); j++) {
          int material = ug->GetCellData()->GetScalars()->GetTuple1(cellIds->GetId(j));
            if (material == specificMaterial) {
              ids->InsertNextValue(i);
              break;
            }
          }
        }
      }
    }
  }
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::AddNodesIntersectingTwoPlanes
(
  int axis1,
  float val1,
  int axis2,
  float val2,
  const char* name,
  vtkboneFiniteElementModel *model,
  int specificMaterial
)
{
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetName (name);
  if (FindNodesIntersectingTwoPlanes (axis1, val1, axis2, val2, ids, model, specificMaterial) != VTK_OK)
    {
    return VTK_ERROR;
    }
  model->AddNodeSet (ids);
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::AddNodesAndElementsIntersectingTwoPlanes
(
  int axis1,
  float val1,
  int axis2,
  float val2,
  const char* name,
  vtkboneFiniteElementModel *model,
  int specificMaterial
)
{
  if (AddNodesIntersectingTwoPlanes (axis1, val1, axis2, val2, name, model, specificMaterial) != VTK_OK)
    { return VTK_ERROR; }
  vtkSmartPointer<vtkIdTypeArray> elementSet = vtkSmartPointer<vtkIdTypeArray>::New();
  model->GetAssociatedElementsFromNodeSet (name, elementSet);
  model->AddElementSet(elementSet);
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::FindNodesIntersectingThreePlanes
(
  int axisA,
  float valA,
  int axisB,
  float valB,
  int axisC,
  float valC,
  vtkIdTypeArray *ids,
  vtkUnstructuredGrid *ug,
  int specificMaterial
)
{
  bonelabMisc::SanityCheck();

  vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();

  for (vtkIdType i=0; i<ug->GetNumberOfPoints(); i++) {
    if (bonelabMisc::ApproximatelyEqual(ug->GetPoint(i)[axisA], valA)) {
      if (bonelabMisc::ApproximatelyEqual(ug->GetPoint(i)[axisB], valB)) {
        if (bonelabMisc::ApproximatelyEqual(ug->GetPoint(i)[axisC], valC)) {
          if (specificMaterial == -1) {
            ids->InsertNextValue(i);
          } else {
            ug->GetPointCells(i,cellIds);
            for (int j=0; j<cellIds->GetNumberOfIds(); j++) {
              int material = ug->GetCellData()->GetScalars()->GetTuple1(cellIds->GetId(j));
              if (material == specificMaterial) {
                ids->InsertNextValue(i);
                break;
              }
            }
          }
        }
      }
    }
  }
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::AddNodesIntersectingThreePlanes
(
  int axis1,
  float val1,
  int axis2,
  float val2,
  int axis3,
  float val3,
  const char* name,
  vtkboneFiniteElementModel *model,
  int specificMaterial
)
{
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetName (name);
  if (FindNodesIntersectingThreePlanes (axis1, val1, axis2, val2, axis3, val3, ids, model, specificMaterial) != VTK_OK)
    {
    return VTK_ERROR;
    }
  model->AddNodeSet (ids);
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::AddNodesAndElementsIntersectingThreePlanes
(
  int axis1,
  float val1,
  int axis2,
  float val2,
  int axis3,
  float val3,
  const char* name,
  vtkboneFiniteElementModel *model,
  int specificMaterial
)
{
  if (AddNodesIntersectingThreePlanes (axis1, val1, axis2, val2, axis3, val3, name, model, specificMaterial) != VTK_OK)
    { return VTK_ERROR; }
  vtkSmartPointer<vtkIdTypeArray> elementSet = vtkSmartPointer<vtkIdTypeArray>::New();
  model->GetAssociatedElementsFromNodeSet (name, elementSet);
  model->AddElementSet(elementSet);
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::FindNodesOnVisibleSurface
(
  vtkIdTypeArray *visibleNodesIds,
  vtkUnstructuredGrid *ug,
  double normalVector[3],
  int specificMaterial
)
{
  // cout << "*** vtkboneNodeSetsByGeometry::FindNodesOnVisibleSurface input\n";
  // ug->Print(cout);

  // Need to make a copy of the input, otherwise we can get into an
  // infinite update loop.
  vtkSmartPointer<vtkUnstructuredGrid> ug_copy = vtkSmartPointer<vtkUnstructuredGrid>::New();
  ug_copy->ShallowCopy(ug);
  vtkboneSelectionUtilities::AddPointPedigreeIdsArray(ug_copy);

  vtkSmartPointer<vtkGeometryFilter> surfaceExtractor = vtkSmartPointer<vtkGeometryFilter>::New();
  surfaceExtractor->SetInputData (ug_copy);
  surfaceExtractor->MergingOff();
  surfaceExtractor->Update ();
  vtkPolyData* exteriorPolys = surfaceExtractor->GetOutput();

  // {
  // vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  // writer->SetInput (exteriorPolys);
  // writer->SetFileName ("exteriorPolys.vtp");
  // writer->Write();
  // }

  vtkSmartPointer<vtkboneOrientationFilter> orientationFilter = vtkSmartPointer<vtkboneOrientationFilter>::New();
  orientationFilter->SetInputData (exteriorPolys);
  orientationFilter->SetNormalVector (normalVector);
  orientationFilter->Update();
  vtkPolyData* exteriorOrientedPolys = orientationFilter->GetOutput();

  // {
  // vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  // writer->SetInput (exteriorOrientedPolys);
  // writer->SetFileName ("exteriorOrientedPolys.vtp");
  // writer->Write();
  // }

  vtkSmartPointer<vtkDataSet> pointDataSet;
  if (specificMaterial == -1)
    {
    // We want to consider points with any material, so just use exteriorOrientedPolys.
    pointDataSet = exteriorOrientedPolys;
    }
  else
    {
    // Create Selection object to filter only those cells with the specified
    // specificMaterial .
    vtkSmartPointer<vtkSelectionNode> selectionNode = vtkSmartPointer<vtkSelectionNode>::New();
    vtkInformation* info = selectionNode->GetProperties();
    vtkSelectionNode::SOURCE()->Set(info, exteriorOrientedPolys);
    selectionNode->SetFieldType(vtkSelectionNode::CELL);
    selectionNode->SetContentType(vtkSelectionNode::THRESHOLDS);
    vtkSmartPointer<vtkIntArray> lims = vtkSmartPointer<vtkIntArray>::New();
    lims->InsertNextValue(specificMaterial);
    lims->InsertNextValue(specificMaterial);
    selectionNode->SetSelectionList(lims);
    vtkSmartPointer<vtkSelection> selection = vtkSmartPointer<vtkSelection>::New();
    selection->AddNode(selectionNode);
    // Apply the Selection
    vtkSmartPointer<vtkExtractSelection> extractor = vtkSmartPointer<vtkExtractSelection>::New();
    extractor->SetInputData (0, exteriorOrientedPolys);
    extractor->SetInputData (1, selection);
    extractor->Update();
    pointDataSet = vtkDataSet::SafeDownCast(extractor->GetOutput());  // Increments reference count.
    }

  vtkSmartPointer<vtkboneSelectVisiblePoints> visibilitySelector = vtkSmartPointer<vtkboneSelectVisiblePoints>::New();
  visibilitySelector->SetInputData (0, exteriorOrientedPolys);
  visibilitySelector->SetInputData (1, pointDataSet);
  visibilitySelector->SetNormalVector (normalVector);
  visibilitySelector->Update();
  vtkPolyData* result = visibilitySelector->GetOutput();
  if (result->GetNumberOfPoints() == 0)
    {
    // No points pass, so return an empty list.
    visibleNodesIds->Initialize();
    }
  else if (result->GetPointData()->GetPedigreeIds())
    {
    visibleNodesIds->DeepCopy (result->GetPointData()->GetPedigreeIds());
    }
  else
    {
    // Shouldn't ever get here, because the selector should always attach
    // at least a PedigreeIds array.
    cerr << "Internal Error.  No PedigreeIds found\n";
    return VTK_ERROR;
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::AddNodesOnVisibleSurface
(
  const char* name,
  vtkboneFiniteElementModel *model,
  double normalVector[3],
  int specificMaterial
)
{
  vtkSmartPointer<vtkIdTypeArray> visibleNodesIds = vtkSmartPointer<vtkIdTypeArray>::New();
  visibleNodesIds->SetName (name);
  if (FindNodesOnVisibleSurface (visibleNodesIds, model, normalVector, specificMaterial) != VTK_OK)
    {
    return VTK_ERROR;
    }

  model->AddNodeSet (visibleNodesIds);
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneNodeSetsByGeometry::AddNodesAndElementsOnVisibleSurface
(
  const char* name,
  vtkboneFiniteElementModel *model,
  double normalVector[3],
  int specificMaterial
)
{
  if (AddNodesOnVisibleSurface (name, model, normalVector, specificMaterial) != VTK_OK)
    { return VTK_ERROR; }
  vtkSmartPointer<vtkIdTypeArray> elementSet = vtkSmartPointer<vtkIdTypeArray>::New();
  model->GetAssociatedElementsFromNodeSet (name, elementSet);
  model->AddElementSet(elementSet);
  return VTK_OK;
}

