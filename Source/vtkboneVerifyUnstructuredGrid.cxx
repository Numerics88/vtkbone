#include "vtkboneVerifyUnstructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkObjectFactory.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkCell.h"
#include "vtkIdList.h"
#include "vtkMath.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkboneVerifyUnstructuredGrid);

//----------------------------------------------------------------------------
vtkboneVerifyUnstructuredGrid::vtkboneVerifyUnstructuredGrid()
:
  Tolerance (1E-5)
{
}

//----------------------------------------------------------------------------
vtkboneVerifyUnstructuredGrid::~vtkboneVerifyUnstructuredGrid()
{
}

//----------------------------------------------------------------------------
void vtkboneVerifyUnstructuredGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Tolerance:" << this->Tolerance << "\n";
}

//----------------------------------------------------------------------------
int vtkboneVerifyUnstructuredGrid::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkUnstructuredGrid* input = vtkUnstructuredGrid::GetData(inputVector[0]);
  vtkUnstructuredGrid* output = vtkUnstructuredGrid::GetData(outputVector);
  
  if (!input || !output)
    {
    vtkErrorMacro (<<"Wrong or not enough inputs/outputs");
    return 0;
    }

  vtkDebugMacro(<<"Verifying vtkUnstructuredGrid");

  // We don't actually modify anything, so the output is a copy of the input
  output->ShallowCopy(input);

  vtkIdType nPoints = input->GetNumberOfPoints();
  vtkIdType nCells = input->GetNumberOfCells();
  
  vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
  for (vtkIdType cellId=0; cellId<nCells; cellId++)
    {
    input->GetCellPoints(cellId, pointIds);
    for (int p=0; p < pointIds->GetNumberOfIds(); p++)
      {
      vtkIdType pointId = pointIds->GetId(p);
      if (pointId < 0 || pointId >= nPoints)
        {
        vtkErrorMacro(<<"Invalid point index for cell " << cellId);
        return 0;
        }
      }
    
    //>>> TO DO: Option to check this plus provide list of valid cells types.
    if (input->GetCellType(cellId) != VTK_VOXEL)
      {
      vtkErrorMacro(<<"Disallowed cell type for cell " << cellId);
      return 0;
      }

    // Note: return of 0 means failure, but vtkErrorMacro already called,
    // so just return 0 again.
    if (input->GetCellType(cellId) == VTK_VOXEL)
      {
      if (!CheckVoxelTopology(input, cellId))
        {
        return 0;
        }
      }
    }

  vtkCellData* cData = input->GetCellData();
  for (int i=0; i < cData->GetNumberOfArrays(); i++)
    {
    if (cData->GetArray(i)->GetNumberOfTuples() != nCells)
      {
      vtkErrorMacro(<<"Cell array " << i << " has an incorrect size");
      return 0;
      }
    }

  vtkPointData* pData = input->GetPointData();
  for (int i=0; i < pData->GetNumberOfArrays(); i++)
    {
    if (pData->GetArray(i)->GetNumberOfTuples() != nPoints)
      {
      vtkErrorMacro(<<"Point array " << i << " has an incorrect size");
      return 0;
      }
    }

  return 1;  
}

//----------------------------------------------------------------------------
int vtkboneVerifyUnstructuredGrid::CheckVoxelTopology(
  vtkUnstructuredGrid* ug,
  vtkIdType cellId)
{
  if (ug->GetCellType(cellId) != VTK_VOXEL)
    {
    vtkErrorMacro(<<"Wrong cell type");
    return 0;
    }

  // Extract 2D array of point coordinates for this cell
  vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
  ug->GetCellPoints(cellId, pointIds);
  double p[8][3];
  for (int i=0; i<8; i++)
    {
    double* coord = ug->GetPoint(pointIds->GetId(i));
    p[i][0] = coord[0];
    p[i][1] = coord[1];
    p[i][2] = coord[2];
    }

  double a[3];
  double b[3];
  double c[3];

  // Calculate where p3 should be
  vtkMath::Subtract(p[1], p[0], b);
  vtkMath::Add(b, p[2], a);
  if (!this->VectorsEqual(p[3], a))
    {
    vtkErrorMacro(<<"Inconsistent voxel coordinates for cell " << cellId);
    return 0;
    }

  // Ensure that p4 is in direction of (p1-p0) X (p2-p0)
  vtkMath::Subtract(p[1], p[0], a);
  vtkMath::Subtract(p[2], p[0], b);
  vtkMath::Cross(a,b,c);
  vtkMath::Subtract(p[4], p[0], a);
  if (!this->VectorsAligned(c, a))
    {
    vtkErrorMacro(<<"Inconsistent voxel coordinates for cell " << cellId);
    return 0;
    }
  
  // Calculate where p5 should be
  vtkMath::Subtract(p[1], p[0], b);
  vtkMath::Add(b, p[4], a);
  if (!this->VectorsEqual(p[5], a))
    {
    vtkErrorMacro(<<"Inconsistent voxel coordinates for cell " << cellId);
    return 0;
    }

  // Calculate where p6 should be
  vtkMath::Subtract(p[2], p[0], b);
  vtkMath::Add(b, p[4], a);
  if (!this->VectorsEqual(p[6], a))
    {
    vtkErrorMacro(<<"Inconsistent voxel coordinates for cell " << cellId);
    return 0;
    }

  // Calculate where p7 should be
  vtkMath::Subtract(p[1], p[0], b);
  vtkMath::Add(b, p[6], a);
  if (!this->VectorsEqual(p[7], a))
    {
    vtkErrorMacro(<<"Inconsistent voxel coordinates for cell " << cellId);
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneVerifyUnstructuredGrid::VectorsEqual (double a[3], double b[3])
{
  if (fabs(a[0] - b[0]) > this->Tolerance)
    return 0;
  if (fabs(a[1] - b[1]) > this->Tolerance)
    return 0;
  if (fabs(a[2] - b[2]) > this->Tolerance)
    return 0;
  return 1;
}

//----------------------------------------------------------------------------
int vtkboneVerifyUnstructuredGrid::VectorsAligned (double a[3], double b[3])
{
  double len_a_sqr = vtkMath::Dot(a,a);
  double len_b_sqr = vtkMath::Dot(b,b);

  // c is difference
  double c[3];
  vtkMath::Subtract(a,b,c);
  double len_c_sqr = vtkMath::Dot(c,c);

  double tol_sqr = this->Tolerance * this->Tolerance;
  if (len_c_sqr < tol_sqr)
    {
    return 1;
    }

  double c_parallel[3];
  // Project c onto line given by longer vector
  if (len_a_sqr > len_b_sqr)
    {
    c_parallel[0] = a[0];
    c_parallel[1] = a[1];
    c_parallel[2] = a[2];
    vtkMath::MultiplyScalar(c_parallel, vtkMath::Dot(a,c) / len_a_sqr);
    }
  else
    {
    c[0] = -c[0];
    c[1] = -c[1];
    c[2] = -c[2];
    c_parallel[0] = b[0];
    c_parallel[1] = b[1];
    c_parallel[2] = b[2];
    vtkMath::MultiplyScalar(c_parallel, vtkMath::Dot(b,c) / len_b_sqr);
    }
  // Difference is c projected onto perpendicular plane
  double c_perpendicular[3];
  vtkMath::Subtract(c, c_parallel, c_perpendicular);
  double len_c_perpendicular_sqr = vtkMath::Dot(c_perpendicular,c_perpendicular);
  if (len_c_perpendicular_sqr > tol_sqr)
    {
    return 0;
    }
  return 1;
}
