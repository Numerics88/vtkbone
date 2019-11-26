#include "vtkboneTensorOfInertia.h"
#include "vtkObjectFactory.h"
#include "vtkCellArray.h"
#include "vtkCell.h"
#include "vtkDataObject.h"
#include "vtkIdList.h"
#include "vtkMath.h"
#include "vtkTensor.h"
#include "vtkMatrix3x3.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"

template <typename T> inline T sqr(T x) {return x*x;}

vtkStandardNewMacro(vtkboneTensorOfInertia);

//----------------------------------------------------------------------------
// Constructs with initial 0 values.
vtkboneTensorOfInertia::vtkboneTensorOfInertia()
{
  this->UseSpecificValue = 0;
  this->UseThresholds = 0;
  this->SpecificValue = 0;
  this->LowerThreshold = VTK_DOUBLE_MIN;
  this->UpperThreshold = VTK_DOUBLE_MAX;

  this->Count = 0;
  this->Volume = 0;
  this->Mass = 0;
  this->CenterOfMass[0] = 0;
  this->CenterOfMass[1] = 0;
  this->CenterOfMass[2] = 0;
  this->TensorOfInertiaAboutOrigin = vtkTensor::New();
  this->TensorOfInertia = vtkTensor::New();
  this->Eigenvectors = vtkMatrix3x3::New();

  this->PrincipalMoments[0] = 0;
  this->PrincipalMoments[1] = 0;
  this->PrincipalMoments[2] = 0;

  this->PrincipalAxisClosestToX[0] = 0;
  this->PrincipalAxisClosestToX[1] = 0;
  this->PrincipalAxisClosestToX[2] = 0;
  this->PrincipalAxisClosestToY[0] = 0;
  this->PrincipalAxisClosestToY[1] = 0;
  this->PrincipalAxisClosestToY[2] = 0;
  this->PrincipalAxisClosestToZ[0] = 0;
  this->PrincipalAxisClosestToZ[1] = 0;
  this->PrincipalAxisClosestToZ[2] = 0;

  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(0);
}

//----------------------------------------------------------------------------
int vtkboneTensorOfInertia::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//----------------------------------------------------------------------------
// Destroy any allocated memory.
vtkboneTensorOfInertia::~vtkboneTensorOfInertia()
{
  if (TensorOfInertia)
    {
    TensorOfInertia->Delete();
    TensorOfInertia = NULL;
    }
  if (TensorOfInertiaAboutOrigin)
    {
    TensorOfInertiaAboutOrigin->Delete();
    TensorOfInertiaAboutOrigin = NULL;
    }
  if (Eigenvectors)
    {
    Eigenvectors->Delete();
    Eigenvectors = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkboneTensorOfInertia::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "UseSpecificValue: " << this->UseSpecificValue << "\n";
  os << indent << "UseThresholds:" << this->UseThresholds << "\n";
  os << indent << "SpecificValue:" << this->SpecificValue << "\n";
  os << indent << "LowerThreshold:" << this->LowerThreshold << "\n";
  os << indent << "UpperThreshold:" << this->UpperThreshold << "\n";

  // The rest of the values only meaningful if filter has been run already.
  os << indent << "Volume: " << this->GetVolume () << "\n";
  os << indent << "Center of Mass: "
    << this->CenterOfMass[0] << ", "
    << this->CenterOfMass[1] << ", "
    << this->CenterOfMass[2] << "\n";
  os << indent << "Moment of Inertia:\n";
  this->TensorOfInertia->PrintSelf (os, indent);
  os << indent << "Moment of Inertia about origin:\n";
  this->TensorOfInertiaAboutOrigin->PrintSelf (os, indent);
}

//----------------------------------------------------------------------------
int vtkboneTensorOfInertia::ProcessRequest(vtkInformation* request,
                                          vtkInformationVector** inputVector,
                                          vtkInformationVector* outputVector)
{
  // generate the data
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
    return this->RequestData(request, inputVector, outputVector);
    }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkboneTensorOfInertia::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* vtkNotUsed( outputVector ))
{
  vtkInformation *inInfo =
    inputVector[0]->GetInformationObject(0);

  vtkImageData *image = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (image)
    { return this->ProcessImage (image); }

  vtkUnstructuredGrid *grid = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (grid)
    { return this->ProcessUnstructuredGrid (grid); }

  vtkErrorMacro( << "vtkboneTensorOfInertia can't handle this kind of input.");
  return VTK_ERROR;
}

//----------------------------------------------------------------------------
int vtkboneTensorOfInertia::ProcessImage(
  vtkImageData* image)
{
  vtkDataArray* scalars = image->GetPointData()->GetScalars();
  int scalarsOnImageCells = 0;
  if (image->GetCellData()->GetScalars())
    {
    if (scalars)
      {
      vtkWarningMacro(<<"Image data has both point and cell scalars");
      }
    scalars = image->GetCellData()->GetScalars();
    scalarsOnImageCells = 1;
    }

  if (scalars->GetNumberOfComponents() != 1)
    {
    vtkErrorMacro(<<"Those aren't very scalar scalars");
    return VTK_ERROR;
    }

  vtkIdType numScalars = scalars->GetNumberOfTuples();
  if (numScalars < 1)
    {
    vtkErrorMacro( << "No data to measure...!");
    return VTK_ERROR;
    }

  int extent[6];
  image->GetExtent(extent);
  vtkIdType dims[3];
  if (scalarsOnImageCells)
    {
    dims[0] = extent[1] - extent[0];
    dims[1] = extent[3] - extent[2];
    dims[2] = extent[5] - extent[4];
    }
  else
    {
    dims[0] = extent[1] - extent[0] + 1;
    dims[1] = extent[3] - extent[2] + 1;
    dims[2] = extent[5] - extent[4] + 1;
    }
  assert (numScalars == dims[0]*dims[1]*dims[2]);
  double spacing[3];
  image->GetSpacing(spacing);
  double origin[3];
  image->GetOrigin(origin);
  // In vtkImageData, origin is always w.r.t. the points.
  if (scalarsOnImageCells)
    {
    origin[0] += 0.5*spacing[0];
    origin[1] += 0.5*spacing[1];
    origin[2] += 0.5*spacing[2];
    }

  //>>> TO DO : In principle, can do this all in one pass, but that is not
  //            working correctly.

  this->Count = 0;
  double sum_x = 0.0;
  double sum_y = 0.0;
  double sum_z = 0.0;
  vtkIdType cellId = 0;
  for (int k=0; k < dims[2]; k++)
    for (int j=0; j < dims[1]; j++)
      for (int i=0; i < dims[0]; i++)
        {
        double val = scalars->GetTuple1(cellId);
        int flag = 1;
        if ((val == 0) ||
            (UseSpecificValue && (int(val) != SpecificValue)) ||
            (UseThresholds && (val < LowerThreshold || val > UpperThreshold)))
          {flag = 0;}
        if (flag)
          {
          this->Count++;
          double x = origin[0] + i*spacing[0];
          double y = origin[1] + j*spacing[1];
          double z = origin[2] + k*spacing[2];
          sum_x += x;
          sum_y += y;
          sum_z += z;
          }
        cellId++;
        }

  double voxelVolume = spacing[0]*spacing[1]*spacing[2];

  this->Volume = this->Count * voxelVolume;
  this->Mass = this->Volume;  // For now, unit density.

  this->CenterOfMass[0] = sum_x / this->Count;
  this->CenterOfMass[1] = sum_y / this->Count;
  this->CenterOfMass[2] = sum_z / this->Count;

  double sum_Ixx = 0.0;
  double sum_Iyy = 0.0;
  double sum_Izz = 0.0;
  double sum_Ixy = 0.0;
  double sum_Iyz = 0.0;
  double sum_Izx = 0.0;
  // Shift to center of mass origin
  double com_origin[3];
  com_origin[0] = origin[0] - this->CenterOfMass[0];
  com_origin[1] = origin[1] - this->CenterOfMass[1];
  com_origin[2] = origin[2] - this->CenterOfMass[2];
  cellId = 0;
  for (int k=0; k < dims[2]; k++)
    for (int j=0; j < dims[1]; j++)
      for (int i=0; i < dims[0]; i++)
        {
        double val = scalars->GetTuple1(cellId);
        int flag = 1;
        if ((val == 0) ||
            (UseSpecificValue && (int(val) != SpecificValue)) ||
            (UseThresholds && (val < LowerThreshold || val > UpperThreshold)))
          {flag = 0;}
        if (flag)
          {
          double x = com_origin[0] + i*spacing[0];
          double y = com_origin[1] + j*spacing[1];
          double z = com_origin[2] + k*spacing[2];
          sum_Ixx += y*y + z*z;
          sum_Iyy += z*z + x*x;
          sum_Izz += x*x + y*y;
          sum_Ixy -= x*y;
          sum_Iyz -= y*z;
          sum_Izx -= z*x;
          }
        cellId++;
        }

  sum_Ixx *= voxelVolume;
  sum_Iyy *= voxelVolume;
  sum_Izz *= voxelVolume;
  sum_Ixy *= voxelVolume;
  sum_Iyz *= voxelVolume;
  sum_Izx *= voxelVolume;

  this->TensorOfInertia->Initialize();
  this->TensorOfInertia->SetComponent (0, 0, sum_Ixx);
  this->TensorOfInertia->SetComponent (1, 1, sum_Iyy);
  this->TensorOfInertia->SetComponent (2, 2, sum_Izz);
  this->TensorOfInertia->SetComponent (0, 1, sum_Ixy);
  this->TensorOfInertia->SetComponent (1, 0, sum_Ixy);
  this->TensorOfInertia->SetComponent (1, 2, sum_Iyz);
  this->TensorOfInertia->SetComponent (2, 1, sum_Iyz);
  this->TensorOfInertia->SetComponent (2, 0, sum_Izx);
  this->TensorOfInertia->SetComponent (0, 2, sum_Izx);

  TranslateTensorOfInertiaFromCOM (
    this->TensorOfInertia,
    this->Mass,
    this->CenterOfMass,
    this->TensorOfInertiaAboutOrigin);

  // Ew! VTK should really provide an interface that doesn't force
  // a reinterpret cast.
  vtkMath::Diagonalize3x3  (
    reinterpret_cast<double(*)[3]>(this->TensorOfInertia->T),
    this->PrincipalMoments,
    reinterpret_cast<double(*)[3]>(this->Eigenvectors->GetData()));

  this->PrincipalAxisClosestToX[0] = this->Eigenvectors->GetElement(0,0);
  this->PrincipalAxisClosestToX[1] = this->Eigenvectors->GetElement(1,0);
  this->PrincipalAxisClosestToX[2] = this->Eigenvectors->GetElement(2,0);
  this->PrincipalAxisClosestToY[0] = this->Eigenvectors->GetElement(0,1);
  this->PrincipalAxisClosestToY[1] = this->Eigenvectors->GetElement(1,1);
  this->PrincipalAxisClosestToY[2] = this->Eigenvectors->GetElement(2,1);
  this->PrincipalAxisClosestToZ[0] = this->Eigenvectors->GetElement(0,2);
  this->PrincipalAxisClosestToZ[1] = this->Eigenvectors->GetElement(1,2);
  this->PrincipalAxisClosestToZ[2] = this->Eigenvectors->GetElement(2,2);

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneTensorOfInertia::ProcessUnstructuredGrid(
  vtkUnstructuredGrid* grid)
{
  if (grid->GetNumberOfCells() == 0)
    { return VTK_OK; }
  this->Count = grid->GetNumberOfCells();

  vtkCellArray* cells = grid->GetCells();
  vtkPoints* points = grid->GetPoints();

  //>>> TO DO : In principle, can do this all in one pass, but that is not
  //            working correctly.

  this->Volume = 0;
  cells->InitTraversal();
  vtkIdType npts = 0;
  vtkIdType* pts = NULL;
  vtkIdType pts1[8];
  vtkIdType cellid = 0;
  double sum_centers[3] = {0, 0, 0};
  while (cells->GetNextCell(npts, pts))
    {
    switch (grid->GetCellType(cellid))
      {
      case VTK_VOXEL:
        this->Volume += (points->GetPoint(pts[1])[0] - points->GetPoint(pts[0])[0])
                      * (points->GetPoint(pts[2])[1] - points->GetPoint(pts[0])[1])
                      * (points->GetPoint(pts[4])[2] - points->GetPoint(pts[0])[2]);
        sum_centers[0] += (points->GetPoint(pts[1])[0] + points->GetPoint(pts[0])[0])/2;
        sum_centers[1] += (points->GetPoint(pts[2])[1] + points->GetPoint(pts[0])[1])/2;
        sum_centers[2] += (points->GetPoint(pts[4])[2] + points->GetPoint(pts[0])[2])/2;
        break;
      default:
        vtkErrorMacro(<<"Unsupported Element Type.");
        return VTK_ERROR;
      }
    ++cellid;
    }

  this->CenterOfMass[0] = sum_centers[0] / this->Count;
  this->CenterOfMass[1] = sum_centers[1] / this->Count;
  this->CenterOfMass[2] = sum_centers[2] / this->Count;

  this->Mass = this->Volume;  // For now, unit density.

  double sum_Ixx = 0.0;
  double sum_Iyy = 0.0;
  double sum_Izz = 0.0;
  double sum_Ixy = 0.0;
  double sum_Iyz = 0.0;
  double sum_Izx = 0.0;
  cells->InitTraversal();
  cellid = 0;
  while (cells->GetNextCell(npts, pts))
    {
    switch (grid->GetCellType(cellid))
      {
      case VTK_VOXEL:
        { // scope for temp variable definition within switch
        double volume = (points->GetPoint(pts[1])[0] - points->GetPoint(pts[0])[0])
                      * (points->GetPoint(pts[2])[1] - points->GetPoint(pts[0])[1])
                      * (points->GetPoint(pts[4])[2] - points->GetPoint(pts[0])[2]);
        double x = (points->GetPoint(pts[1])[0] + points->GetPoint(pts[0])[0])/2
                 - this->CenterOfMass[0];
        double y = (points->GetPoint(pts[2])[1] + points->GetPoint(pts[0])[1])/2
                 - this->CenterOfMass[1];
        double z = (points->GetPoint(pts[4])[2] + points->GetPoint(pts[0])[2])/2
                 - this->CenterOfMass[2];
        sum_Ixx += volume * (y*y + z*z);
        sum_Iyy += volume * (z*z + x*x);
        sum_Izz += volume * (x*x + y*y);
        sum_Ixy -= volume * (x*y);
        sum_Iyz -= volume * (y*z);
        sum_Izx -= volume * (z*x);
        }
        break;
      default:
        vtkErrorMacro(<<"Unsupported Element Type.");
        return VTK_ERROR;
      }
    ++cellid;
    }

  this->TensorOfInertia->Initialize();
  this->TensorOfInertia->SetComponent (0, 0, sum_Ixx);
  this->TensorOfInertia->SetComponent (1, 1, sum_Iyy);
  this->TensorOfInertia->SetComponent (2, 2, sum_Izz);
  this->TensorOfInertia->SetComponent (0, 1, sum_Ixy);
  this->TensorOfInertia->SetComponent (1, 0, sum_Ixy);
  this->TensorOfInertia->SetComponent (1, 2, sum_Iyz);
  this->TensorOfInertia->SetComponent (2, 1, sum_Iyz);
  this->TensorOfInertia->SetComponent (2, 0, sum_Izx);
  this->TensorOfInertia->SetComponent (0, 2, sum_Izx);

  TranslateTensorOfInertiaFromCOM (
    this->TensorOfInertia,
    this->Mass,
    this->CenterOfMass,
    this->TensorOfInertiaAboutOrigin);

  // Ew! VTK should really provide an interface that doesn't force
  // a reinterpret cast.
  vtkMath::Diagonalize3x3  (
    reinterpret_cast<double(*)[3]>(this->TensorOfInertia->T),
    this->PrincipalMoments,
    reinterpret_cast<double(*)[3]>(this->Eigenvectors->GetData()));

  this->PrincipalAxisClosestToX[0] = this->Eigenvectors->GetElement(0,0);
  this->PrincipalAxisClosestToX[1] = this->Eigenvectors->GetElement(1,0);
  this->PrincipalAxisClosestToX[2] = this->Eigenvectors->GetElement(2,0);
  this->PrincipalAxisClosestToY[0] = this->Eigenvectors->GetElement(0,1);
  this->PrincipalAxisClosestToY[1] = this->Eigenvectors->GetElement(1,1);
  this->PrincipalAxisClosestToY[2] = this->Eigenvectors->GetElement(2,1);
  this->PrincipalAxisClosestToZ[0] = this->Eigenvectors->GetElement(0,2);
  this->PrincipalAxisClosestToZ[1] = this->Eigenvectors->GetElement(1,2);
  this->PrincipalAxisClosestToZ[2] = this->Eigenvectors->GetElement(2,2);

  return VTK_OK;
}

//----------------------------------------------------------------------------
void vtkboneTensorOfInertia::GetTensorOfInertia (vtkTensor* MOI)
{
  MOI->DeepCopy(this->TensorOfInertia);
}

//----------------------------------------------------------------------------
void vtkboneTensorOfInertia::GetTensorOfInertiaAboutOrigin (vtkTensor* MOI)
{
  MOI->DeepCopy(this->TensorOfInertiaAboutOrigin);
}

//----------------------------------------------------------------------------
void vtkboneTensorOfInertia::GetEigenvectors (vtkMatrix3x3* A)
{
  A->DeepCopy(this->Eigenvectors);
}

//----------------------------------------------------------------------------
void vtkboneTensorOfInertia::TranslateTensorOfInertiaFromCOM
(
  vtkTensor* T0,
  double m,
  double r[3],
  vtkTensor* T
)
{
  T->Initialize();
  T->SetComponent (0,0, T0->GetComponent(0,0) + m*(r[1]*r[1] + r[2]*r[2]));
  T->SetComponent (1,1, T0->GetComponent(1,1) + m*(r[2]*r[2] + r[0]*r[0]));
  T->SetComponent (2,2, T0->GetComponent(2,2) + m*(r[0]*r[0] + r[1]*r[1]));
  T->SetComponent (0,1, T0->GetComponent(0,1) - m*r[0]*r[1]);
  T->SetComponent (1,0, T->GetComponent(0,1));
  T->SetComponent (1,2, T0->GetComponent(1,2) - m*r[1]*r[2]);
  T->SetComponent (2,1, T->GetComponent(1,2));
  T->SetComponent (2,0, T0->GetComponent(2,0) - m*r[2]*r[0]);
  T->SetComponent (0,2, T->GetComponent(2,0));  
}

//----------------------------------------------------------------------------
void vtkboneTensorOfInertia::TranslateTensorOfInertiaToCOM
(
  vtkTensor* T,
  double m,
  double r[3],
  vtkTensor* T0
)
{
  T0->Initialize();
  T0->SetComponent (0,0, T->GetComponent(0,0) - m*(r[1]*r[1] + r[2]*r[2]));
  T0->SetComponent (1,1, T->GetComponent(1,1) - m*(r[2]*r[2] + r[0]*r[0]));
  T0->SetComponent (2,2, T->GetComponent(2,2) - m*(r[0]*r[0] + r[1]*r[1]));
  T0->SetComponent (0,1, T->GetComponent(0,1) + m*r[0]*r[1]);
  T0->SetComponent (1,0, T->GetComponent(0,1));
  T0->SetComponent (1,2, T->GetComponent(1,2) + m*r[1]*r[2]);
  T0->SetComponent (2,1, T->GetComponent(1,2));
  T0->SetComponent (2,0, T->GetComponent(2,0) + m*r[2]*r[0]);
  T0->SetComponent (0,2, T->GetComponent(2,0));  
}

//----------------------------------------------------------------------------
void vtkboneTensorOfInertia::TranslateTensorOfInertia
(
  vtkTensor* T1,
  double r1[3],
  double m,
  double r2[3],
  vtkTensor* T2
)
{
  vtkSmartPointer<vtkTensor> T0 = vtkSmartPointer<vtkTensor>::New();
  TranslateTensorOfInertiaToCOM (T1, m, r1, T0);
  TranslateTensorOfInertiaFromCOM (T0, m, r2, T2);
}

//----------------------------------------------------------------------------
void vtkboneTensorOfInertia::SetInputData(vtkDataObject* input)
{
  this->SetInputData(0, input);
}

//----------------------------------------------------------------------------
void vtkboneTensorOfInertia::SetInputData(int index, vtkDataObject* input)
{
  this->SetInputDataInternal(index, input);
}
