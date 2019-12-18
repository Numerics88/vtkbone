#include "vtkboneImageToMesh.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkUnstructuredGrid.h"
#include "vtkObjectFactory.h"
#include "vtkDoubleArray.h"
#include "vtkIntArray.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellLinks.h"
#include "vtkPointData.h"
#include "vtkDataSet.h"
#include "vtkExtractUnstructuredGrid.h"
#include "vtkMergePoints.h"
#include "vtkCellTypes.h"
#include "vtkImageData.h"
#include "vtkDelaunay3D.h"
#include "vtkPolyData.h"
#include "vtkMath.h"
#include "vtkSmartPointer.h"
#include <boost/format.hpp>

vtkStandardNewMacro(vtkboneImageToMesh);

//-----------------------------------------------------------------------
vtkboneImageToMesh::vtkboneImageToMesh()
{
}

//----------------------------------------------------------------------------
vtkboneImageToMesh::~vtkboneImageToMesh()
{
}

//----------------------------------------------------------------------------
void vtkboneImageToMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
int vtkboneImageToMesh::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info )
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}

//----------------------------------------------------------------------------
int vtkboneImageToMesh::GetInputAsCellScalars
(
  vtkImageData* input,
  vtkImageData* inputAsCellScalars
)
{
  inputAsCellScalars->Initialize();

  if (input->GetCellData()->GetScalars())
    {
    if (input->GetPointData()->GetScalars())
      {
      vtkWarningMacro(<< "Image data has both Point and Cell scalars.  Using"
                         " the Cell scalar data.\n");
      }
    // Scalar data is on the cells, which we want.
    inputAsCellScalars->ShallowCopy(input);
    }
  else
    {
    // Create a new vtkImageData object with the scalar data on the cells.
    if (!input->GetPointData()->GetScalars())
      {
      vtkErrorMacro(<< "Image data has no data.\n");
      return 0;
      }
    int dims[3];
    int extents[6];
    double origin[3];
    double spacing[3];
    input->GetDimensions(dims);
    input->GetExtent(extents);
    input->GetOrigin(origin);
    input->GetSpacing(spacing);
    inputAsCellScalars->SetSpacing(spacing);
    inputAsCellScalars->SetDimensions(dims[0]+1, dims[1]+1, dims[2]+1);
    inputAsCellScalars->SetOrigin(origin[0] + (extents[0] - 0.5) * spacing[0],
                                  origin[1] + (extents[2] - 0.5) * spacing[1],
                                  origin[2] + (extents[4] - 0.5) * spacing[2]);
    inputAsCellScalars->GetCellData()->SetScalars(input->GetPointData()->GetScalars());
    }

    return 1;
}

//----------------------------------------------------------------------------
// NOTE: Assumption is that input has data on the Cells.  This corresponds
//       more logically with FE elements.  Use the method GetInputAsCellScalars
//       to ensure that the input to this function satisfies this assumption.
int vtkboneImageToMesh::GenerateHexahedrons
(
  vtkImageData* input,
  vtkUnstructuredGrid* output
)
{

  vtkDebugMacro(<<"Generating hexahedrons...");

  vtkDataArray* inputScalars = input->GetCellData()->GetScalars();
  if (inputScalars == NULL)
    {
    vtkErrorMacro(<< "No data found for input Image.");
    return 0;
    }
  if (inputScalars->GetNumberOfTuples() != input->GetNumberOfCells())
    {
    vtkErrorMacro(<< "Inconsistent number of cell data values.");
    return 0;
    }

  vtkIdType numInputCells = input->GetNumberOfCells();
  vtkIdType numInputPoints = input->GetNumberOfPoints();

  // Create a map of new Cells to old Cells
  vtkSmartPointer<vtkIdTypeArray> reverseCellMap = vtkSmartPointer<vtkIdTypeArray>::New();
  reverseCellMap->Allocate(numInputCells);   // maximum possible
  reverseCellMap->SetNumberOfComponents(1);
  for (int oldId=0; oldId < numInputCells; oldId++)
    {
    if (inputScalars->GetTuple1(oldId) != 0)
      {
      reverseCellMap->InsertNextValue(oldId);
      }
    }
  vtkIdType numOutputCells = reverseCellMap->GetNumberOfTuples();

  // Create a map of old Points to new Points.
  vtkSmartPointer<vtkIdTypeArray> pointMap = vtkSmartPointer<vtkIdTypeArray>::New();
  pointMap->SetNumberOfComponents(1);
  pointMap->SetNumberOfTuples(numInputPoints);
  for (vtkIdType i=0; i < numInputPoints; i++)
    {
    pointMap->SetValue(i, -1);   // -1 indicates old point not used.
    }
  // First step is just to flag points that are used in output.
  {  // scope
  vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
  for (int newCellId=0; newCellId < numOutputCells; newCellId++)
    {
    input->GetCellPoints(reverseCellMap->GetValue(newCellId), pointIds);
    for (int p=0; p < pointIds->GetNumberOfIds(); p++)
      {
      pointMap->SetValue(pointIds->GetId(p), 1);   // 1 indicates old point is used.
      }
    }
  }
  // Now go through and generate actual new Ids for new points (replacing all
  // the 1's).
  vtkIdType numOutputPoints = 0;
  for (vtkIdType i=0; i<pointMap->GetNumberOfTuples(); i++)
    {
    if (pointMap->GetValue(i) != -1)
      {
      pointMap->SetValue(i, numOutputPoints);
      numOutputPoints++;
      }
    }

  // Generate the node coordinate list.
  vtkSmartPointer<vtkDoubleArray> pointCoord = vtkSmartPointer<vtkDoubleArray>::New();
  pointCoord->SetNumberOfComponents(3);
  pointCoord->SetNumberOfTuples(numOutputPoints);
  for (vtkIdType oldId=0; oldId < numInputPoints; oldId++)
    {
    vtkIdType newId = pointMap->GetValue(oldId);
    if (newId != -1)
      {
      pointCoord->SetTuple(newId, input->GetPoint(oldId));
      }
    }

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetData(pointCoord);
  output->SetPoints(points);

  // Generate Cell Array (i.e. list of point Ids associated with cells)
  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  // The following allocation is exact
  const int pointsPerCell = 8;
  cells->Allocate(cells->EstimateSize(numOutputCells, pointsPerCell));
  {  // scope
  vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
  for (vtkIdType newCellId=0; newCellId < numOutputCells; newCellId++)
    {
    input->GetCellPoints(reverseCellMap->GetValue(newCellId), pointIds);
    for (int p=0; p < pointIds->GetNumberOfIds(); p++)
      {
      pointIds->SetId(p, pointMap->GetValue(pointIds->GetId(p)));
      }
    cells->InsertNextCell(pointIds);
    }
  }
  output->SetCells(VTK_VOXEL, cells);

  // Generate Cell scalars
  vtkSmartPointer<vtkDataArray> outputScalars = vtkSmartPointer<vtkDataArray>::Take(
                vtkDataArray::CreateDataArray(inputScalars->GetDataType()));
  outputScalars->SetNumberOfTuples(numOutputCells);
  outputScalars->SetName("MaterialID");
  for (vtkIdType newCellId=0; newCellId < numOutputCells; newCellId++)
    {
    outputScalars->SetTuple(newCellId, reverseCellMap->GetValue(newCellId), inputScalars);
    }
  output->GetCellData()->SetScalars(outputScalars);

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneImageToMesh::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkImageData* unmodifiedInput = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkImageData> input = vtkSmartPointer<vtkImageData>::New();
  this->GetInputAsCellScalars(unmodifiedInput, input);

  // Initialize and checks
  int dims[3];
  double origin[3];
  double spacing[3];
  input->GetDimensions(dims);
  input->GetOrigin(origin);
  input->GetSpacing(spacing);

  if ((dims[0]<1) || (dims[1]<1) || (dims[2]<1))
    {
    vtkErrorMacro(<<"This filter requires 3D input data.");
    return 0;
    }
  if ((spacing[0]<=0.0) || (spacing[1]<=0.0) || (spacing[2]<=0.0))
    {
    vtkErrorMacro(<<"Spacing must be greater than or equal to 0.0.");
    return 0;
    }

  vtkDebugMacro(<<"\n"
                <<"  Dimensions:    " << dims[0]      << ", " << dims[1]    << ", " << dims[2]    << "\n"
                <<"  Origin:      " << origin[0]    << ", " << origin[1]  << ", " << origin[2]  << "\n"
                <<"  Spacing:    " << spacing[0]    << ", " << spacing[1] << ", " << spacing[2] << "\n"
                );

  this->GenerateHexahedrons(input, output);

  vtkDebugMacro(<<"\n  Generated :\n"
                << "    " << output->GetNumberOfPoints() <<" points\n"
                << "    " << output->GetNumberOfCells() <<" cells\n"
                );

  return 1;
}

//-----------------------------------------------------------------------
void vtkboneImageToMesh::Report(ostream& s)
{
  using boost::format;

  vtkIdType numPoints = this->GetOutput()->GetNumberOfPoints();
  vtkIdType numCells = this->GetOutput()->GetNumberOfCells();

  s << "!-----------------------------------------------------------------------------\n";
  s << (format(" %-30s\n") % "vtkboneImageToMesh").str();
  s << "\n";
  s << (format("   %-30s%-12d\n") % "Points generated" % numPoints).str();
  s << (format("   %-30s%-12d\n") % "Cells generated" % numCells).str();
  s << "\n";
}
