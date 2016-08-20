#include "vtkboneInterpolateCoarseSolution.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkboneMacros.h"
#include "vtkSmartPointer.h"
#include "n88util/array.hpp"
#include "n88util/exception.hpp"
#include "boost/format.hpp"
#include <cmath>

#ifdef _WIN32
#if (_MSC_VER < 1800)
template <typename T> T round(T x) {return floor(x+0.5);}
#endif
#endif

using boost::format;

vtkStandardNewMacro(vtkboneInterpolateCoarseSolution);

//----------------------------------------------------------------------------
vtkboneInterpolateCoarseSolution::vtkboneInterpolateCoarseSolution()
  :
  SolutionArray (NULL)
  {
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(0);
  }

//----------------------------------------------------------------------------
vtkboneInterpolateCoarseSolution::~vtkboneInterpolateCoarseSolution()
  {
  if (this->SolutionArray) { this->SolutionArray->Delete(); }
  }

//----------------------------------------------------------------------------
void vtkboneInterpolateCoarseSolution::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os, indent);
  }

//----------------------------------------------------------------------------
int vtkboneInterpolateCoarseSolution::FillInputPortInformation(int port, vtkInformation *info)
  {
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkboneFiniteElementModel");
    }
  if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkboneFiniteElementModel");
    }
  else
    {
    return 0;
    }  
  return 1;
  }

template <typename T> bool rel_comp(const T x, const T y, const T tol)
  {
  if (x==0) return (x==y);
  T d = std::abs((x-y)/x);
  return (d < tol);
  }

//----------------------------------------------------------------------------
int vtkboneInterpolateCoarseSolution::RequestData
(
  vtkInformation *,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector
)
{
  vtkInformation* inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation* inInfo1 = inputVector[1]->GetInformationObject(0);
  vtkboneFiniteElementModel* full_model =
    vtkboneFiniteElementModel::SafeDownCast(inInfo0->Get(vtkDataObject::DATA_OBJECT()));
  vtkboneFiniteElementModel* reduced_model =
    vtkboneFiniteElementModel::SafeDownCast(inInfo1->Get(vtkDataObject::DATA_OBJECT()));
  if (!full_model || !reduced_model)
    {
    vtkErrorMacro("Missing input object.");
    return 0;
    }

  if (full_model->GetNumberOfCells() == 0 ||
  	  reduced_model->GetNumberOfCells() == 0)
    {
    vtkWarningMacro("Zero elements on input to vtkboneInterpolateCoarseSolution");
    return 0;
    }

  //   VTK_VOXEL
  //           6---------7
  //          /|        /|
  //         / |       / |
  //        /  |      /  |
  //       4---------5   |
  //       |   |     |   |
  //       |   2-----|---3
  //       |  /      |  /
  //       |/        | /     z  y
  //       0---------1/      | /
  //                         |/
  //                         .--->x

  vtkCell* cell = full_model->GetCell(0);
  if (cell->GetCellType() != VTK_VOXEL)
    {
    vtkErrorMacro(<<"Unsupported Cell Type.");
    return VTK_ERROR;
    }
  double full_spacing[3];
  full_spacing[0] = full_model->GetPoint(cell->GetPointId(1))[0] - full_model->GetPoint(cell->GetPointId(0))[0];
  full_spacing[1] = full_model->GetPoint(cell->GetPointId(2))[1] - full_model->GetPoint(cell->GetPointId(0))[1];
  full_spacing[2] = full_model->GetPoint(cell->GetPointId(4))[2] - full_model->GetPoint(cell->GetPointId(0))[2];
  // std::cout << "full spacing = "
  //           << full_spacing[0] << ", "
  //           << full_spacing[1] << ", "
  //           << full_spacing[2] << "\n";

  cell = reduced_model->GetCell(0);
  if (cell->GetCellType() != VTK_VOXEL)
    {
    vtkErrorMacro(<<"Unsupported Cell Type.");
    return VTK_ERROR;
    }
  double reduced_spacing[3];
  reduced_spacing[0] = reduced_model->GetPoint(cell->GetPointId(1))[0] - reduced_model->GetPoint(cell->GetPointId(0))[0];
  reduced_spacing[1] = reduced_model->GetPoint(cell->GetPointId(2))[1] - reduced_model->GetPoint(cell->GetPointId(0))[1];
  reduced_spacing[2] = reduced_model->GetPoint(cell->GetPointId(4))[2] - reduced_model->GetPoint(cell->GetPointId(0))[2];
  // std::cout << "reduced spacing = "
  //           << reduced_spacing[0] << ", "
  //           << reduced_spacing[1] << ", "
  //           << reduced_spacing[2] << "\n";

  double tol = 1E-4;
  n88_verbose_assert (rel_comp(2*full_spacing[0],reduced_spacing[0],tol) &&
  	                  rel_comp(2*full_spacing[1],reduced_spacing[1],tol) &&
  	                  rel_comp(2*full_spacing[2],reduced_spacing[2],tol),
    "Spacing of reduced resolution must be exactly twice that of full resolution");

  double full_bounds[6];
  full_model->GetBounds(full_bounds);
  // std::cout << "Full bounds:\n"
  //           << format("  %10.4f%10.4f\n") % full_bounds[0] % full_bounds[1]
  //           << format("  %10.4f%10.4f\n") % full_bounds[2] % full_bounds[3]
  //           << format("  %10.4f%10.4f\n") % full_bounds[4] % full_bounds[5];
  double reduced_bounds[6];
  reduced_model->GetBounds(reduced_bounds);
  // std::cout << "Reduced bounds:\n"
  //           << format("  %10.4f%10.4f\n") % reduced_bounds[0] % reduced_bounds[1]
  //           << format("  %10.4f%10.4f\n") % reduced_bounds[2] % reduced_bounds[3]
  //           << format("  %10.4f%10.4f\n") % reduced_bounds[4] % reduced_bounds[5];

  tol = 1E-2 * full_spacing[0];  // scale tolerance by spacing.
  n88_verbose_assert (full_bounds[0] >= reduced_bounds[0] - tol &&
  	                  full_bounds[1] <= reduced_bounds[1] + tol &&
  	                  full_bounds[2] >= reduced_bounds[2] - tol &&
  	                  full_bounds[3] <= reduced_bounds[3] + tol &&
  	                  full_bounds[4] >= reduced_bounds[4] - tol &&
  	                  full_bounds[5] <= reduced_bounds[5] + tol,
    "Full bounds must be interior to reduced bounds");

  vtkDataArray* reduced_data = reduced_model->GetPointData()->GetArray("Displacement");
  if (!reduced_data)
    {
    vtkErrorMacro(<<"Solution not found in reduced model.");
    return VTK_ERROR;    
    }

  // To easily index the values of the reduced model, we will put on a regular grid.
  // x fastest, z slowest
  n88::tuplet<4,size_t> grid_dims(
    (size_t)(round((reduced_bounds[5]-reduced_bounds[4])/reduced_spacing[2]) + 1),
    (size_t)(round((reduced_bounds[3]-reduced_bounds[2])/reduced_spacing[1]) + 1),
    (size_t)(round((reduced_bounds[1]-reduced_bounds[0])/reduced_spacing[0]) + 1),
    3);
  n88::array<4,float> reduced_grid (grid_dims);

  for (vtkIdType p=0; p<reduced_model->GetNumberOfPoints(); ++p)
    {
    double* coords = reduced_model->GetPoint(p);
    size_t index[3];
    index[0] = size_t(round((coords[0] - reduced_bounds[0])/reduced_spacing[0]));
    index[1] = size_t(round((coords[1] - reduced_bounds[2])/reduced_spacing[1]));
    index[2] = size_t(round((coords[2] - reduced_bounds[4])/reduced_spacing[2]));
    double* displacement = reduced_data->GetTuple3(p);
    reduced_grid(index[2],index[1],index[0],0) = displacement[0];
    reduced_grid(index[2],index[1],index[0],1) = displacement[1];
    reduced_grid(index[2],index[1],index[0],2) = displacement[2];
    }

  vtkFloatArray* full_data = vtkFloatArray::New();
  full_data->SetNumberOfComponents(3);
  full_data->SetNumberOfTuples(full_model->GetNumberOfPoints());
  full_data->SetName("Displacement");

  for (vtkIdType p=0; p<full_model->GetNumberOfPoints(); ++p)
    {
    double* coords = full_model->GetPoint(p);
    size_t index[3];
    index[0] = size_t(round((coords[0] - full_bounds[0])/full_spacing[0]));
    index[1] = size_t(round((coords[1] - full_bounds[2])/full_spacing[1]));
    index[2] = size_t(round((coords[2] - full_bounds[4])/full_spacing[2]));
    size_t odd[3];
    odd[0] = index[0] % 2;
    odd[1] = index[1] % 2;
    odd[2] = index[2] % 2;
    index[0] /= 2;
    index[1] /= 2;
    index[2] /= 2;
    n88_assert (index[0] < reduced_grid.dims()[2]);
    n88_assert (index[1] < reduced_grid.dims()[1]);
    n88_assert (index[2] < reduced_grid.dims()[0]);
    if (!odd[0] && !odd[1] && !odd[2])
      {
      for (size_t i=0; i<3; ++i) {
        full_data->SetComponent(p,i,reduced_grid(index[2],index[1],index[0],i)); }
      }
    else if (odd[0] && !odd[1] && !odd[2])
      {
      for (size_t i=0; i<3; ++i) {
        full_data->SetComponent(p,i,
          (reduced_grid(index[2],index[1],index[0]  ,i) +
           reduced_grid(index[2],index[1],index[0]+1,i))/2); }
      }
    else if (!odd[0] && odd[1] && !odd[2])
      {
      for (size_t i=0; i<3; ++i) {
        full_data->SetComponent(p,i,
          (reduced_grid(index[2],index[1]  ,index[0],i) +
           reduced_grid(index[2],index[1]+1,index[0],i))/2); }
      }
    else if (!odd[0] && !odd[1] && odd[2])
      {
      for (size_t i=0; i<3; ++i) {
        full_data->SetComponent(p,i,
          (reduced_grid(index[2]  ,index[1],index[0],i) +
           reduced_grid(index[2]+1,index[1],index[0],i))/2); }
      }
    else if (!odd[0] && odd[1] && odd[2])
      {
      for (size_t i=0; i<3; ++i) {
        full_data->SetComponent(p,i,
          (reduced_grid(index[2]  ,index[1]  ,index[0],i) +
           reduced_grid(index[2]  ,index[1]+1,index[0],i) +
           reduced_grid(index[2]+1,index[1]  ,index[0],i) +
           reduced_grid(index[2]+1,index[1]+1,index[0],i))/4); }
      }
    else if (odd[0] && !odd[1] && odd[2])
      {
      for (size_t i=0; i<3; ++i) {
        full_data->SetComponent(p,i,
          (reduced_grid(index[2]  ,index[1],index[0]  ,i) +
           reduced_grid(index[2]  ,index[1],index[0]+1,i) +
           reduced_grid(index[2]+1,index[1],index[0]  ,i) +
           reduced_grid(index[2]+1,index[1],index[0]+1,i))/4); }
      }
    else if (odd[0] && odd[1] && !odd[2])
      {
      for (size_t i=0; i<3; ++i) {
        full_data->SetComponent(p,i,
          (reduced_grid(index[2],index[1]  ,index[0]  ,i) +
           reduced_grid(index[2],index[1]  ,index[0]+1,i) +
           reduced_grid(index[2],index[1]+1,index[0]  ,i) +
           reduced_grid(index[2],index[1]+1,index[0]+1,i))/4); }
      }
    else // all odd
      {
      for (size_t i=0; i<3; ++i) {
        full_data->SetComponent(p,i,
          (reduced_grid(index[2]  ,index[1]  ,index[0]  ,i) +
           reduced_grid(index[2]  ,index[1]  ,index[0]+1,i) +
           reduced_grid(index[2]  ,index[1]+1,index[0]  ,i) +
           reduced_grid(index[2]  ,index[1]+1,index[0]+1,i) +
           reduced_grid(index[2]+1,index[1]  ,index[0]  ,i) +
           reduced_grid(index[2]+1,index[1]  ,index[0]+1,i) +
           reduced_grid(index[2]+1,index[1]+1,index[0]  ,i) +
           reduced_grid(index[2]+1,index[1]+1,index[0]+1,i))/8); }
      }
    }

  if (this->SolutionArray) { this->SolutionArray->Delete(); }
  this->SolutionArray = full_data;

  return 1;
}
