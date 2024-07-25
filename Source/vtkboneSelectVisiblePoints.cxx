#include "vtkboneSelectVisiblePoints.h"
#include "vtkIdTypeArray.h"
#include "vtkModifiedBSPTree.h"
#include "vtkSelection.h"
#include "vtkboneSelectionUtilities.h"
#include "vtkExtractSelection.h"
#include "vtkGeometryFilter.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkSmartPointer.h"
#include "vtkInformationVector.h"
#include "vtkExecutive.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkObjectFactory.h"
#include <limits>
#include <algorithm>

// Uncomment to generate debugging output for FindVisiblePoints
//#define TRACE_INTERSECTION_LINES

#ifdef TRACE_INTERSECTION_LINES
#include "vtkXMLPolyDataWriter.h"
#endif

vtkStandardNewMacro (vtkboneSelectVisiblePoints);

//----------------------------------------------------------------------------
vtkboneSelectVisiblePoints::vtkboneSelectVisiblePoints()
{
  this->Tolerance = 1E-4;
  this->NormalVector[0] = 0;
  this->NormalVector[1] = 0;
  this->NormalVector[2] = 1;
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkboneSelectVisiblePoints::~vtkboneSelectVisiblePoints()
{
}

//----------------------------------------------------------------------------
void vtkboneSelectVisiblePoints::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Tolerance: " << Tolerance << endl;
  os << indent << "NormalVector: "
    << NormalVector[0] << ", "
    << NormalVector[1] << ", "
    << NormalVector[2] << "\n";
}

//----------------------------------------------------------------------------
int vtkboneSelectVisiblePoints::FillInputPortInformation
(
  int port,
  vtkInformation* info
)
{
  if (port==0)
  {
    info->Set (vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  }
  else
  {
    info->Set (vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  }
  return 1;
}

//----------------------------------------------------------------------------
int vtkboneSelectVisiblePoints::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkInformation* inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation* inInfo1 = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* surface = vtkPolyData::SafeDownCast (inInfo0->Get (vtkDataObject::DATA_OBJECT()));
  vtkDataSet* pointDataSet = vtkDataSet::SafeDownCast (inInfo1->Get (vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast (outInfo->Get (vtkDataObject::DATA_OBJECT()));

  if (!surface || !pointDataSet || !output)
  {
    vtkErrorMacro (<<"Wrong or not enough inputs/outputs");
    return 0;
  }

  vtkIdType numCandidatePoints = pointDataSet->GetNumberOfPoints();
  vtkSmartPointer<vtkIdTypeArray> visiblePointList = vtkSmartPointer<vtkIdTypeArray>::New();
  if (numCandidatePoints == 0)
  {
    return 1;
  }
  // Over-allocate.  Does no harm on 64 bit systems.
  visiblePointList->Allocate (numCandidatePoints);

  surface->ComputeBounds();
  double bounds[6];
  surface->GetBounds(bounds);

  double maxDim = std::max(std::max(bounds[1]-bounds[0],bounds[3]-bounds[2]),
                          bounds[5]-bounds[4]);
  double pointOffset = 1.01 * Tolerance;

  // ModifiedBSPTree is best locator for ray casting, which we are doing
  vtkSmartPointer<vtkModifiedBSPTree> locator = vtkSmartPointer<vtkModifiedBSPTree>::New();
  locator->SetDataSet(surface);
  locator->BuildLocator();

#ifdef TRACE_INTERSECTION_LINES
  vtkSmartPointer<vtkPoints> visibleIntersectionLineEnds = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> visibleLinesCells = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> hiddenIntersectionLineEnds = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> hiddenLinesCells = vtkSmartPointer<vtkCellArray>::New();
#endif

  // here we form a line using the point in question and a point just outside the z-bounds
  // of the image but in the same x and y position.  The locator checks to see if anything
  // intersects this line.  If not, the point is added to the list.
  for (vtkIdType id = 0; id < numCandidatePoints; id++)
  {
    double p0[3];
    pointDataSet->GetPoint(id,p0);
    double s = LineBoundsIntersection (p0, NormalVector, bounds);
    if (s<0) continue;  // Point is outside surface bounds and direction points away
    // Shift by pointOffset away from point
    // This will prevent us from finding an intersection with a surface
    // right at the point.  Also will guarantee that p1 is outside the bounds,
    // so no ambiguity about finding intersection with surface at bounds.
    p0[0] += pointOffset * NormalVector[0];
    p0[1] += pointOffset * NormalVector[1];
    p0[2] += pointOffset * NormalVector[2];
    double p1[3];
    p1[0] = p0[0] + s * NormalVector[0];
    p1[1] = p0[1] + s * NormalVector[1];
    p1[2] = p0[2] + s * NormalVector[2];
    double t;
    double x[3];
    double pcoords[3];
    int subId;
    if (locator->IntersectWithLine(p0,p1,Tolerance,t,x,pcoords,subId) == 0)
    {
      visiblePointList->InsertNextValue (id);
#ifdef TRACE_INTERSECTION_LINES
      vtkIdType begin = visibleIntersectionLineEnds->InsertNextPoint(p0);
      vtkIdType end = visibleIntersectionLineEnds->InsertNextPoint(p1);
      visibleLinesCells->InsertNextCell(2);
      visibleLinesCells->InsertCellPoint(begin);
      visibleLinesCells->InsertCellPoint(end);
#endif
    }
#ifdef TRACE_INTERSECTION_LINES
    else
    {
      vtkIdType begin = hiddenIntersectionLineEnds->InsertNextPoint(p0);
      vtkIdType end = hiddenIntersectionLineEnds->InsertNextPoint(p1);
      hiddenLinesCells->InsertNextCell(2);
      hiddenLinesCells->InsertCellPoint(begin);
      hiddenLinesCells->InsertCellPoint(end);
    }
#endif
  }

#ifdef TRACE_INTERSECTION_LINES
  vtkSmartPointer<vtkPolyData> visibleLines = vtkSmartPointer<vtkPolyData>::New();
  visibleLines->SetPoints(visibleIntersectionLineEnds);
  visibleLines->SetLines(visibleLinesCells);
  vtkSmartPointer<vtkXMLPolyDataWriter> visibleLinesWriter =
                              vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  const char* visibleLinesFile = "visibleLines.vtp";
  visibleLinesWriter->SetFileName(visibleLinesFile);
  visibleLinesWriter->SetInput(visibleLines);
  cout << "FindVisiblePoints: Writing visible lines to " << visibleLinesFile << "\n";
  visibleLinesWriter->Write();

  vtkSmartPointer<vtkPolyData> hiddenLines = vtkSmartPointer<vtkPolyData>::New();
  hiddenLines->SetPoints(hiddenIntersectionLineEnds);
  hiddenLines->SetLines(hiddenLinesCells);
  vtkSmartPointer<vtkXMLPolyDataWriter> hiddenLinesWriter =
                              vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  const char* hiddenLinesFile = "hiddenLines.vtp";
  hiddenLinesWriter->SetFileName(hiddenLinesFile);
  hiddenLinesWriter->SetInput(hiddenLines);
  cout << "FindVisiblePoints: Writing hidden lines to " << hiddenLinesFile << "\n";
  hiddenLinesWriter->Write();
#endif

  // Extract all the marked Points as a Vertex set.
  vtkSmartPointer<vtkSelection> selection = vtkSmartPointer<vtkSelection>::New();
  vtkboneSelectionUtilities::PointSelectionFromIds (selection, pointDataSet, visiblePointList);

  vtkSmartPointer<vtkExtractSelection> extractor = vtkSmartPointer<vtkExtractSelection>::New();
  extractor->SetInputData (0, pointDataSet);
  extractor->SetInputData (1, selection);
  extractor->Update();

  // We need this because the output of vtkExtractSelection is vtkUnstructuredGrid,
  // however, we need to return a vtkPolyData.
  vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
  geometryFilter->SetInputData (extractor->GetOutput());
  geometryFilter->Update();

  output->ShallowCopy (geometryFilter->GetOutput());

  vtkPointData* pointData = output->GetPointData();
  if (pointData->GetPedigreeIds() == NULL)
  {
    // It shouldn't be possible for vtkOriginalPointIds not to exist, as
    // it is added by vtkExtractSelection.  However, if for some reason
    // it doesn't, then PedigreeIds will remain NULL.
    pointData->SetPedigreeIds(pointData->GetArray("vtkOriginalPointIds"));
  }

  return 1;
}

//----------------------------------------------------------------------------
double vtkboneSelectVisiblePoints::LineBoundsIntersection
(
  double P[3],
  double V[3],
  double bounds[6]
)
{
  double s;
  if (V[0] > 0)
  {
    s = (bounds[1] - P[0])/V[0];
  }
  else if (V[0] < 0)
  {
    s = (bounds[0] - P[0])/V[0];
  }
  else
  {
    s = std::numeric_limits<double>::infinity();
  }
  if (V[1] > 0)
  {
    s = std::min(s, (bounds[3] - P[1])/V[1]);
  }
  else if (V[1] < 0)
  {
    s = std::min(s, (bounds[2] - P[1])/V[1]);
  }
  if (V[2] > 0)
  {
    s = std::min(s, (bounds[5] - P[2])/V[2]);
  }
  else if (V[2] < 0)
  {
    s = std::min(s, (bounds[4] - P[2])/V[2]);
  }
  return s;
}

