#include "vtkboneCoarsenModel.h"

#include "vtkboneFiniteElementModel.h"
#include "vtkboneMaterialTable.h"
#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkboneLinearIsotropicMaterialArray.h"
#include "vtkboneLinearOrthotropicMaterial.h"
#include "vtkboneLinearOrthotropicMaterialArray.h"
#include "vtkboneLinearAnisotropicMaterial.h"
#include "vtkboneLinearAnisotropicMaterialArray.h"
#include "vtkboneConstraintCollection.h"
#include "vtkboneSolverParameters.h"
#include "vtkboneStressStrainMatrix.h"
#include "vtkDataArrayCollection.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkObjectFactory.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkInformationStringVectorKey.h"
#include "n88util/const_array.hpp"
#include <map>
#include <set>

vtkStandardNewMacro(vtkboneCoarsenModel);


const unsigned int EMPTY = std::numeric_limits<unsigned int>::max();


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::RequestUpdateExtent(
  vtkInformation *, vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  // always request the whole extent
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
              inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()),6);

  return 1;
}


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the data object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkboneFiniteElementModel *output = vtkboneFiniteElementModel::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkboneFiniteElementModel *input = vtkboneFiniteElementModel::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  return this->SimpleExecute(input, output);
}


//----------------------------------------------------------------------------
void vtkboneCoarsenModel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::SimpleExecute(
  vtkboneFiniteElementModel* input,
  vtkboneFiniteElementModel* output)
  {

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

  vtkIdType nInputPoints = input->GetNumberOfPoints();
  vtkIdType nInputCells = input->GetNumberOfCells();
  if (!nInputPoints || !nInputCells)
    {
    vtkErrorMacro(<<"vtkboneFiniteElementModel has no Cells or Points.");
    return VTK_ERROR;
    }
  if (input->GetCellType(0) != VTK_VOXEL)
    {
    vtkErrorMacro(<<"Unsupported cell type");
    return VTK_ERROR;
    }

  // std::cout << "Number of cells " << nInputCells << "\n";
  // std::cout << "Number of points " << nInputPoints << "\n";

  // ---- Get input spacing and dimensions

  double bounds[6];
  input->GetBounds(bounds);
  // std::cout << "Bounds: "
  //           << bounds[0] << ", "
  //           << bounds[1] << ", "
  //           << bounds[2] << ", "
  //           << bounds[3] << ", "
  //           << bounds[4] << ", "
  //           << bounds[5] << "\n";

  double inputSpacing[3];
    {  // scope
    vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
    input->GetCellPoints(0, pointIds);
    double p0[3];
    double p1[3]; 
    double p2[3]; 
    double p4[3]; 
    input->GetPoint(pointIds->GetId(0), p0);
    input->GetPoint(pointIds->GetId(1), p1);
    input->GetPoint(pointIds->GetId(2), p2);
    input->GetPoint(pointIds->GetId(4), p4);
    inputSpacing[0] = p1[0] - p0[0];
    inputSpacing[1] = p2[1] - p0[1];
    inputSpacing[2] = p4[2] - p0[2];
    }
  // std::cout << "Spacing: "
  //           << inputSpacing[0] << ", "
  //           << inputSpacing[1] << ", "
  //           << inputSpacing[2] << "\n";
  unsigned int inputDims[3];
  inputDims[0] = 0.5 + (bounds[1] - bounds[0])/inputSpacing[0];
  inputDims[1] = 0.5 + (bounds[3] - bounds[2])/inputSpacing[1];
  inputDims[2] = 0.5 + (bounds[5] - bounds[4])/inputSpacing[2];
  // std::cout << "Dimensions: "
  //           << inputDims[0] << ", "
  //           << inputDims[1] << ", "
  //           << inputDims[2] << "\n";

  // ---- Generate input cell grid

  n88::array<3,unsigned int> inputCellGrid(
          inputDims[2],
          inputDims[1],
          inputDims[0]);
  for (size_t i=0; i<inputCellGrid.size(); ++i)
    { inputCellGrid[i] = EMPTY; }
    { // scope
    vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
    for (vtkIdType cellId=0; cellId<nInputCells; ++cellId)
      {
      if (input->GetCellType(cellId) != VTK_VOXEL)
        {
        vtkErrorMacro(<<"Unsupported cell type");
        return VTK_ERROR;
        }
      input->GetCellPoints(cellId, pointIds);
      double p0[3];
      input->GetPoint(pointIds->GetId(0),p0);
      unsigned int i = 0.5 + (p0[0] - bounds[0])/inputSpacing[0];
      unsigned int j = 0.5 + (p0[1] - bounds[2])/inputSpacing[1];
      unsigned int k = 0.5 + (p0[2] - bounds[4])/inputSpacing[2];
      inputCellGrid(k,j,i) = cellId;
      }
    }

    {  // scope
    size_t count = 0;
    for (size_t i=0; i<inputCellGrid.size(); ++i)
      if (inputCellGrid[i] != EMPTY)
        { ++count; }
    // std::cout << "Number of non-empty cell grid points " << count << "\n";
    }

  // ---- Generate input point grid

  n88::array<3,unsigned int> inputPointGrid(
          inputDims[2]+1,
          inputDims[1]+1,
          inputDims[0]+1);
  for (size_t i=0; i<inputPointGrid.size(); ++i)
    { inputPointGrid[i] = EMPTY; }
    {  // scope
    for (vtkIdType pointId=0; pointId<nInputPoints; ++pointId)
      {
      double p0[3];
      input->GetPoint(pointId,p0);
      unsigned int i = 0.5 + (p0[0] - bounds[0])/inputSpacing[0];
      unsigned int j = 0.5 + (p0[1] - bounds[2])/inputSpacing[1];
      unsigned int k = 0.5 + (p0[2] - bounds[4])/inputSpacing[2];
      inputPointGrid(k,j,i) = pointId;
      }
    }

    {  // scope
    size_t count = 0;
    for (size_t i=0; i<inputPointGrid.size(); ++i)
      if (inputPointGrid[i] != EMPTY)
        { ++count; }
    // std::cout << "Number of non-empty point grid points " << count << "\n";
    }

  // ---- Output dimensions

  unsigned int outputDims[3];
  outputDims[0] = (inputDims[0] + 1)/2;   // Round up
  outputDims[1] = (inputDims[1] + 1)/2;
  outputDims[2] = (inputDims[2] + 1)/2;
  // std::cout << "Output dimensions: "
  //           << outputDims[0] << ", "
  //           << outputDims[1] << ", "
  //           << outputDims[2] << "\n";
  double outputSpacing[3];
  outputSpacing[0] = inputSpacing[0]*2;
  outputSpacing[1] = inputSpacing[1]*2;
  outputSpacing[2] = inputSpacing[2]*2;

  // ---- Generate output cell grid

  // Note that n88::array starts out zeroed.
  n88::array<3,unsigned int> outputCellGrid(
          outputDims[2],
          outputDims[1],
          outputDims[0]);
  // identify where we need output cells
    { // scope
    vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
    for (vtkIdType cellId=0; cellId<nInputCells; ++cellId)
      {
      input->GetCellPoints(cellId, pointIds);
      double p0[3];
      input->GetPoint(pointIds->GetId(0),p0);
      // These are indices for the output cell grid
      unsigned int i = 0.25 + (p0[0] - bounds[0])/outputSpacing[0];
      unsigned int j = 0.25 + (p0[1] - bounds[2])/outputSpacing[1];
      unsigned int k = 0.25 + (p0[2] - bounds[4])/outputSpacing[2];
      outputCellGrid(k,j,i) = 1;
      }
    }
  // number the output cells
  unsigned int nOutputCells = 0;
    { // scope
    vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
    for (size_t i=0; i<outputCellGrid.size(); ++i)
      {
      if (outputCellGrid[i])
        {
        outputCellGrid[i] = nOutputCells;
        ++nOutputCells;
        }
      else
        {
        outputCellGrid[i] = EMPTY;
        }
      }
    }
  // std::cout << "Number of output cells " << nOutputCells << "\n";

  // ---- Generate output point grid

  // Note that n88::array starts out zeroed.
  n88::array<3,unsigned int> outputPointGrid(
          outputDims[2]+1,
          outputDims[1]+1,
          outputDims[0]+1);
  // identify where we need output points
  for (unsigned int k=0; k<outputDims[2]; ++k)
    for (unsigned int j=0; j<outputDims[1]; ++j)
      for (unsigned int i=0; i<outputDims[0]; ++i)
        {
        if (outputCellGrid(k,j,i) != EMPTY)
          {
          outputPointGrid(k  ,j  ,i  ) = 1;
          outputPointGrid(k  ,j  ,i+1) = 1;
          outputPointGrid(k  ,j+1,i  ) = 1;
          outputPointGrid(k  ,j+1,i+1) = 1;
          outputPointGrid(k+1,j  ,i  ) = 1;
          outputPointGrid(k+1,j  ,i+1) = 1;
          outputPointGrid(k+1,j+1,i  ) = 1;
          outputPointGrid(k+1,j+1,i+1) = 1;
          }
        }
  // number the output points
  unsigned int nOutputPoints = 0;
    { // scope
    vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
    for (size_t i=0; i<outputPointGrid.size(); ++i)
      {
      if (outputPointGrid[i])
        {
        outputPointGrid[i] = nOutputPoints;
        ++nOutputPoints;
        }
      else
        {
        outputPointGrid[i] = EMPTY;
        }
      }
    }
  // std::cout << "Number of output points " << nOutputPoints << "\n";

  // --- Generate reverse cell map (output to input)

  n88::array<2,unsigned int> reverseCellMap(nOutputCells,8);
    {  // scope
    size_t a = 0;
    vtkIdType outputCellId = 0;
    for (unsigned int k=0; k<outputDims[2]; ++k)
      {
      unsigned int kk = 2*k;
      unsigned int kk_1 = kk+1;
      // Bring back in bounds if at odd-layer face. This has the effect
      // of duplicating the odd-layer face.
      if (kk_1 == inputDims[2])
        { kk_1 = kk; }
      for (unsigned int j=0; j<outputDims[1]; ++j)
        {
        unsigned int jj = 2*j;
        unsigned int jj_1 = jj+1;
        if (jj_1 == inputDims[1])
          { jj_1 = jj; }
        for (unsigned int i=0; i<outputDims[0]; ++i)
          {
          if (outputCellGrid[a] != EMPTY)
            {
            n88_assert (outputCellId == outputCellGrid[a]);
            unsigned int ii = 2*i;
            unsigned int ii_1 = ii+1;
            if (ii_1 == inputDims[0])
              { ii_1 = ii; }
            reverseCellMap(outputCellId,0) = inputCellGrid(kk  ,jj  ,ii  );
            reverseCellMap(outputCellId,1) = inputCellGrid(kk  ,jj  ,ii_1);
            reverseCellMap(outputCellId,2) = inputCellGrid(kk  ,jj_1,ii  );
            reverseCellMap(outputCellId,3) = inputCellGrid(kk  ,jj_1,ii_1);
            reverseCellMap(outputCellId,4) = inputCellGrid(kk_1,jj  ,ii  );
            reverseCellMap(outputCellId,5) = inputCellGrid(kk_1,jj  ,ii_1);
            reverseCellMap(outputCellId,6) = inputCellGrid(kk_1,jj_1,ii  );
            reverseCellMap(outputCellId,7) = inputCellGrid(kk_1,jj_1,ii_1);
            ++outputCellId;
            }
          ++a;
          }
        }
      }
    }

  // Done with input cell grid
  inputCellGrid.destruct();

  // --- Generate cell map (input to output)

  n88::array<1,vtkIdType> cellMap(nInputCells);
  for (vtkIdType outputCellId=0; outputCellId<nOutputCells; ++outputCellId)
    {
    for (unsigned int i=0; i<8; ++i)
      {
      vtkIdType inputCellID = reverseCellMap(outputCellId,i);
      if (inputCellID != EMPTY)
        { cellMap(inputCellID) = outputCellId; }
      }
    }

  // --- Generate point map (input to output)

  n88::array<1,unsigned int> pointMap(nInputPoints);
    {  // scope
    size_t a = 0;
    vtkIdType inputPointId = 0;
    for (unsigned int k=0; k<=inputDims[2]; ++k)
      {
      // Round up beyond the centre (ie. round outwards)
      unsigned int kk = (k + (unsigned int)(2*k > inputDims[2]))/2;
      for (unsigned int j=0; j<=inputDims[1]; ++j)
        {
        unsigned int jj = (j + (unsigned int)(2*j > inputDims[1]))/2;
        for (unsigned int i=0; i<=inputDims[0]; ++i)
          {
          if (inputPointGrid[a] != EMPTY)
            {
            n88_assert (inputPointId == inputPointGrid[a]);
            unsigned int ii = (i + (unsigned int)(2*i > inputDims[0]))/2;
            pointMap[inputPointId] = outputPointGrid(kk,jj,ii);
            n88_assert (pointMap[inputPointId] != EMPTY);
            ++inputPointId;
            }
          ++a;
          }
        }
      }
    }

  // Done with input point grid
  inputPointGrid.destruct();

  int return_val = 0;

  return_val = this->GenerateCells(
    output,
    outputPointGrid.data(),
    outputCellGrid.data(),
    outputDims,
    nOutputCells);
  if (return_val == 0) { return VTK_ERROR; }

  // Done with output cell grid
  outputCellGrid.destruct();

  return_val = this->GeneratePointCoordinates(
    output,
    outputPointGrid.data(),
    outputDims,
    outputSpacing,
    bounds,
    nOutputPoints);
  if (return_val == 0) { return VTK_ERROR; }

  // Done with output point grid
  outputPointGrid.destruct();

  return_val = this->GenerateMaterials (output, input, reverseCellMap.data());
  if (return_val == 0) { return VTK_ERROR; }

  return_val = this->GenerateConstraints (output, input, pointMap.data());
  if (return_val == 0) { return VTK_ERROR; }

  return_val = this->GenerateConvergenceSet (output, input, pointMap.data());
  if (return_val == 0) { return VTK_ERROR; }

  return_val = this->GenerateNodeAndElementSets (output, input, pointMap.data(), cellMap.data());
  if (return_val == 0) { return VTK_ERROR; }

  return_val = this->GenerateAdditionalInformation (output, input);
  if (return_val == 0) { return VTK_ERROR; }

  return 1;
  }


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::GeneratePointCoordinates
  (
  vtkboneFiniteElementModel* output,
  const unsigned int* outputPointGrid,
  unsigned int outputDims[3],
  double outputSpacing[3],
  double bounds[6],
  vtkIdType nOutputPoints
  )
  {
  n88_assert (output != 0);
  n88_assert (outputPointGrid != 0);
  n88_assert (outputDims[0] > 0 &&
              outputDims[1] > 0 &&
              outputDims[2] > 0);

  vtkSmartPointer<vtkFloatArray> pointCoord = vtkSmartPointer<vtkFloatArray>::New();
  pointCoord->SetNumberOfComponents(3);
  pointCoord->SetNumberOfTuples(nOutputPoints);
    {  // scope
    size_t a = 0;
    for (unsigned int k=0; k<=outputDims[2]; ++k)
      {
      double z = bounds[4] + outputSpacing[2]*k;
      for (unsigned int j=0; j<=outputDims[1]; ++j)
        {
        double y = bounds[2] + outputSpacing[1]*j;
        for (unsigned int i=0; i<=outputDims[0]; ++i)
          {
          if (outputPointGrid[a] != EMPTY)
            {
            double x = bounds[0] + outputSpacing[0]*i;
            pointCoord->SetTuple3(outputPointGrid[a],x,y,z);
            }
          ++a;
          }
        }
      }
    }
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetData(pointCoord);
  output->SetPoints(points);
  return 1;
  }


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::GenerateCells
  (
  vtkboneFiniteElementModel* output,
  const unsigned int* outputPointGrid_ptr,
  const unsigned int* outputCellGrid,
  unsigned int outputDims[3],
  vtkIdType nOutputCells
  )
  {
  n88_assert (output != 0);
  n88_assert (outputPointGrid_ptr != 0);
  n88_assert (outputCellGrid != 0);
  n88_assert (outputDims[0] > 0 &&
              outputDims[1] > 0 &&
              outputDims[2] > 0);

  // outputPointGrid was passed as pointer: create array reference.
  n88::const_array<3,unsigned int> outputPointGrid (outputPointGrid_ptr,
                                                    outputDims[2]+1,
                                                    outputDims[1]+1,
                                                    outputDims[0]+1);

  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  // The following allocation is exact
  const int pointsPerCell = 8;
  cells->Allocate(cells->EstimateSize(nOutputCells, pointsPerCell));
    {  // scope
    size_t a = 0;
    vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
    pointIds->SetNumberOfIds(pointsPerCell);
    for (unsigned int k=0; k<outputDims[2]; ++k)
      {
      for (unsigned int j=0; j<outputDims[1]; ++j)
        {
        for (unsigned int i=0; i<outputDims[0]; ++i)
          {
          if (outputCellGrid[a] != EMPTY)
            {
            pointIds->SetId (0, outputPointGrid(k  ,j  ,i  ));
            pointIds->SetId (1, outputPointGrid(k  ,j  ,i+1));
            pointIds->SetId (2, outputPointGrid(k  ,j+1,i  ));
            pointIds->SetId (3, outputPointGrid(k  ,j+1,i+1));
            pointIds->SetId (4, outputPointGrid(k+1,j  ,i  ));
            pointIds->SetId (5, outputPointGrid(k+1,j  ,i+1));
            pointIds->SetId (6, outputPointGrid(k+1,j+1,i  ));
            pointIds->SetId (7, outputPointGrid(k+1,j+1,i+1));
            n88_assert (pointIds->GetId(0) != EMPTY &&
                        pointIds->GetId(1) != EMPTY &&
                        pointIds->GetId(2) != EMPTY &&
                        pointIds->GetId(3) != EMPTY &&
                        pointIds->GetId(4) != EMPTY &&
                        pointIds->GetId(5) != EMPTY &&
                        pointIds->GetId(6) != EMPTY &&
                        pointIds->GetId(7) != EMPTY);
            cells->InsertNextCell(pointIds);
            }
          ++a;
          }
        }
      }
    }
  output->SetCells(VTK_VOXEL, cells);
  return 1;
  }


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::GenerateMaterials
  (
  vtkboneFiniteElementModel* output,
  vtkboneFiniteElementModel* input,
  const unsigned int* reverseCellMap_ptr
  )
  {
  vtkboneMaterialTable* inputMaterialTable = input->GetMaterialTable();

  // Check if we have a single material, and can use the special method
  // GenerateMaterialsSingleInputMaterial.
  int numberUniqueMaterials = 0;
  inputMaterialTable->InitTraversal(); 
  while (inputMaterialTable->GetNextUniqueIndex())
    { ++numberUniqueMaterials; }
  if (numberUniqueMaterials == 0)
    {
    vtkErrorMacro(<<"Empty material table.");
    return VTK_ERROR;
    }  
  if (numberUniqueMaterials == 1)
    {
    // Need additionally to verify that this material is not a material array.
    inputMaterialTable->InitTraversal(); 
    inputMaterialTable->GetNextIndex();
    vtkboneMaterial* material = inputMaterialTable->GetCurrentMaterial();
    if (vtkboneMaterialArray::SafeDownCast(material) == 0)
      {
      return this->GenerateMaterialsSingleInputMaterial(
                    output, input, reverseCellMap_ptr);
      }
    }

  vtkIdType nOutputCells = output->GetNumberOfCells();
  n88::const_array<2,unsigned int> reverseCellMap (reverseCellMap_ptr, nOutputCells, 8);

  // ---- Generate cell scalars

  // Since we are going to use a material array, want sequential values
  vtkSmartPointer<vtkUnsignedIntArray> outputScalars = vtkSmartPointer<vtkUnsignedIntArray>::New();
  outputScalars->SetNumberOfTuples(nOutputCells);
  outputScalars->SetName("MaterialID");
    {  // scope
    vtkIdType count = 1;
    for (vtkIdType cellId=0; cellId < nOutputCells; ++cellId)
      {
      outputScalars->SetComponent(cellId, 0, count);
      ++count;
      }
    }
  output->GetCellData()->SetScalars(outputScalars);

  // ---- Determine what kind of material array to hold all defined materials

  bool have_orthotropic = false;
  bool have_anisotropic = false;
  inputMaterialTable->InitTraversal(); 
  while (int index = inputMaterialTable->GetNextUniqueIndex())
    {
    vtkboneMaterial* material = inputMaterialTable->GetCurrentMaterial();
    if (vtkboneLinearAnisotropicMaterial::SafeDownCast(material) ||
        vtkboneLinearAnisotropicMaterialArray::SafeDownCast(material))
      {
      have_anisotropic = true;
      break;
      }
    if (vtkboneLinearOrthotropicMaterial::SafeDownCast(material) ||
        vtkboneLinearOrthotropicMaterialArray::SafeDownCast(material))
      {
      have_orthotropic = true;
      }
    }

  // ---- Now populate the material array by averaging over 2x2x2 input cells

  // Even in orthotropic case, use anisotropic for output: more sensible averaging.
  if (have_anisotropic || have_orthotropic)
    {
    vtkSmartPointer<vtkboneLinearAnisotropicMaterialArray> anisoMaterials =
        vtkSmartPointer<vtkboneLinearAnisotropicMaterialArray>::New();
    anisoMaterials->Resize(nOutputCells);
    output->GetMaterialTable()->AddMaterial(1, anisoMaterials);
    std::vector<float> D (21);
    std::vector<float> sum_D (21);
    vtkSmartPointer<vtkboneStressStrainMatrix> stressStrain =
        vtkSmartPointer<vtkboneStressStrainMatrix>::New();
    for (unsigned int oel=0; oel<nOutputCells; ++oel)
      {
      for (unsigned int k=0; k<21; ++k)
        { sum_D[k] = 0; }
      unsigned int count = 0;
      for (unsigned int i=0; i<8; ++i)
        {
        unsigned int iel = reverseCellMap(oel,i);
        if (iel != EMPTY)
          {
          int id = input->GetCellData()->GetScalars()->GetComponent(iel,0);
          vtkboneMaterial* material = NULL;
          int offset = 0;
          inputMaterialTable->GetMaterialOrArray (id, material, offset);
          n88_assert (material);
          if (vtkboneLinearAnisotropicMaterial* m =
                vtkboneLinearAnisotropicMaterial::SafeDownCast(material))
            {
            stressStrain->SetStressStrainMatrix (m->GetStressStrainMatrix());
            }
          else if (vtkboneLinearAnisotropicMaterialArray* m =
                vtkboneLinearAnisotropicMaterialArray::SafeDownCast(material))
            {
            n88_assert (m->GetSize() > offset);
            stressStrain->SetUpperTriangularPacked (m->GetItemUpperTriangular (offset));
            }
          else if (vtkboneLinearOrthotropicMaterial* m =
                vtkboneLinearOrthotropicMaterial::SafeDownCast(material))
            {
            stressStrain->SetOrthotropic (m->GetYoungsModulusX(),
                                          m->GetYoungsModulusY(),
                                          m->GetYoungsModulusZ(),
                                          m->GetPoissonsRatioYZ(),
                                          m->GetPoissonsRatioZX(),
                                          m->GetPoissonsRatioXY(),
                                          m->GetShearModulusYZ(),
                                          m->GetShearModulusZX(),
                                          m->GetShearModulusXY());
            }
          else if (vtkboneLinearOrthotropicMaterialArray* m =
                vtkboneLinearOrthotropicMaterialArray::SafeDownCast(material))
            {
            n88_assert (m->GetSize() > offset);
            stressStrain->SetOrthotropic (
                     m->GetYoungsModulus()->GetComponent (offset, 0),
                     m->GetYoungsModulus()->GetComponent (offset, 1),
                     m->GetYoungsModulus()->GetComponent (offset, 2),
                     m->GetPoissonsRatio()->GetComponent (offset, 0),
                     m->GetPoissonsRatio()->GetComponent (offset, 1),
                     m->GetPoissonsRatio()->GetComponent (offset, 2),
                     m->GetShearModulus()->GetComponent (offset, 0),
                     m->GetShearModulus()->GetComponent (offset, 1),
                     m->GetShearModulus()->GetComponent (offset, 2));
            }
          else if (vtkboneLinearIsotropicMaterial* m =
                vtkboneLinearIsotropicMaterial::SafeDownCast(material))
            {
            stressStrain->SetIsotropic (m->GetYoungsModulus(),
                                        m->GetPoissonsRatio());
            }
          else if (vtkboneLinearIsotropicMaterialArray* m =
                vtkboneLinearIsotropicMaterialArray::SafeDownCast(material))
            {
            n88_assert (m->GetSize() > offset);
            stressStrain->SetIsotropic (
                     m->GetYoungsModulus()->GetComponent (offset, 0),
                     m->GetPoissonsRatio()->GetComponent (offset, 0));
            }
          else
            {
            throw_n88_exception ("Internal error.");
            }
          for (unsigned int k=0; k<21; ++k)
            {
            stressStrain->GetUpperTriangularPacked(D.data());
            sum_D[k] += D[k];
            }
          ++count;
          }
        }
      n88_assert (count); // Must be at least one input element corresponding to output element
      anisoMaterials->SetScaledItemUpperTriangular (oel,sum_D.data(), 1.0/8.0);
      }
    }

  else  // Only isotropic materials
    {
    vtkSmartPointer<vtkboneLinearIsotropicMaterialArray> isoMaterials =
        vtkSmartPointer<vtkboneLinearIsotropicMaterialArray>::New();
    isoMaterials->Resize(nOutputCells);
    output->GetMaterialTable()->AddMaterial(1, isoMaterials);
    for (unsigned int oel=0; oel<nOutputCells; ++oel)
      {
      double E=0;
      double nu=0;
      unsigned int count = 0;
      for (unsigned int i=0; i<8; ++i)
        {
        unsigned int iel = reverseCellMap(oel,i);
        if (iel != EMPTY)
          {
          int id = input->GetCellData()->GetScalars()->GetComponent(iel,0);
          vtkboneMaterial* material = NULL;
          int offset = 0;
          inputMaterialTable->GetMaterialOrArray (id, material, offset);
          n88_assert (material);
          if (vtkboneLinearIsotropicMaterial* m =
                vtkboneLinearIsotropicMaterial::SafeDownCast(material))
            {
            n88_assert (offset == 0);
            E += m->GetYoungsModulus();
            nu += m->GetPoissonsRatio();
            }
          else if (vtkboneLinearIsotropicMaterialArray* m =
                vtkboneLinearIsotropicMaterialArray::SafeDownCast(material))
            {
            n88_assert (m->GetSize() > offset);
            // std::cout << id << ", " << offset << ", " << m->GetYoungsModulus()->GetComponent(offset,0) << "\n";
            E += m->GetYoungsModulus()->GetComponent(offset,0);
            nu += m->GetPoissonsRatio()->GetComponent(offset,0);
            }
          else
            {
            throw_n88_exception ("Internal error.");
            }
          ++count;
          }
        }
      n88_assert (count); // Must be at least one input element corresponding to output element
      E /= 8;
      nu /= count;
      isoMaterials->GetYoungsModulus()->SetComponent(oel,0,E);
      isoMaterials->GetPoissonsRatio()->SetComponent(oel,0,nu);
      }
    }

  return 1;
  }


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::GenerateMaterialsSingleInputMaterial
  (
  vtkboneFiniteElementModel* output,
  vtkboneFiniteElementModel* input,
  const unsigned int* reverseCellMap_ptr
  )
  {
  vtkIdType nOutputCells = output->GetNumberOfCells();
  n88::const_array<2,unsigned int> reverseCellMap (reverseCellMap_ptr, nOutputCells, 8);

  // We know there is only one material defined.
  vtkboneMaterialTable* inputMaterialTable = input->GetMaterialTable();
  inputMaterialTable->InitTraversal(); 
  inputMaterialTable->GetNextUniqueIndex();
  vtkboneMaterial* material = inputMaterialTable->GetCurrentMaterial();
  n88_assert (inputMaterialTable->GetNextUniqueIndex() == 0); // verify no other materials

  // ---- Determine what kind of material

  vtkboneLinearIsotropicMaterial* iso_material =
      vtkboneLinearIsotropicMaterial::SafeDownCast(material);
  vtkboneLinearOrthotropicMaterial* ortho_material =
  vtkboneLinearOrthotropicMaterial::SafeDownCast(material);
  vtkboneLinearAnisotropicMaterial* aniso_material =
      vtkboneLinearAnisotropicMaterial::SafeDownCast(material);

  // --- Populate output material array with 8 possible materials

  if (aniso_material)
    {
    vtkSmartPointer<vtkboneLinearAnisotropicMaterialArray> anisoMaterials =
        vtkSmartPointer<vtkboneLinearAnisotropicMaterialArray>::New();
    anisoMaterials->Resize(8);
    output->GetMaterialTable()->AddMaterial(1, anisoMaterials);
    for (unsigned int m=0; m<8; ++m)
      {
      anisoMaterials->SetScaledItem (m, aniso_material, (m+1)/8.0);
      }
    }

  else if (ortho_material)
    {
    vtkSmartPointer<vtkboneLinearOrthotropicMaterialArray> orthoMaterials =
        vtkSmartPointer<vtkboneLinearOrthotropicMaterialArray>::New();
    orthoMaterials->Resize(8);
    output->GetMaterialTable()->AddMaterial(1, orthoMaterials);
    for (unsigned int m=0; m<8; ++m)
      {
      orthoMaterials->SetScaledItem (m, ortho_material, (m+1)/8.0);
      }
    }

  else if (iso_material)
    {
    vtkSmartPointer<vtkboneLinearIsotropicMaterialArray> isoMaterials =
        vtkSmartPointer<vtkboneLinearIsotropicMaterialArray>::New();
    isoMaterials->Resize(8);
    output->GetMaterialTable()->AddMaterial(1, isoMaterials);
    for (unsigned int m=0; m<8; ++m)
      {
      isoMaterials->SetScaledItem (m, iso_material, (m+1)/8.0);
      }
    }

  else
    {
    vtkErrorMacro(<<"Unknown material.");
    return VTK_ERROR;
    }

  // ---- Generate cell scalars (valid values 1-8)

  vtkSmartPointer<vtkUnsignedIntArray> outputScalars = vtkSmartPointer<vtkUnsignedIntArray>::New();
  outputScalars->SetNumberOfTuples(nOutputCells);
  outputScalars->SetName("MaterialID");
  output->GetCellData()->SetScalars(outputScalars);

  for (unsigned int oel=0; oel<nOutputCells; ++oel)
    {
    unsigned int count = 0;
    for (unsigned int i=0; i<8; ++i)
      {
      unsigned int iel = reverseCellMap(oel,i);
      if (iel != EMPTY)
        {
        ++count;
        }
      }
    n88_assert (count); // Must be at least one input element corresponding to output element
    outputScalars->SetComponent(oel, 0, count);
    }

  return 1;
  }


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::GenerateConstraints
  (
  vtkboneFiniteElementModel* output,
  vtkboneFiniteElementModel* input,
  const unsigned int* pointMap
  )
  {
  vtkboneConstraintCollection* inputConstraints = input->GetConstraints();
  inputConstraints->InitTraversal();
  while (vtkboneConstraint* inputConstraint = inputConstraints->GetNextItem())
    {
    if (inputConstraint->GetConstraintAppliedTo() == vtkboneConstraint::NODES)
      {
      vtkSmartPointer<vtkboneConstraint> outputConstraint =
          vtkSmartPointer<vtkboneConstraint>::New();
      outputConstraint->SetName (inputConstraint->GetName());
      outputConstraint->SetConstraintAppliedTo (inputConstraint->GetConstraintAppliedTo());
      outputConstraint->SetConstraintType (inputConstraint->GetConstraintType());
      vtkIdTypeArray* inputIndices = inputConstraint->GetIndices();
      vtkSmartPointer<vtkIdTypeArray> outputIndices =
          vtkSmartPointer<vtkIdTypeArray>::New();
      vtkIdType N = inputIndices->GetNumberOfTuples();
      outputIndices->SetNumberOfTuples(N);
      for (vtkIdType i=0; i<N; ++i)
        { outputIndices->SetValue (i, pointMap[inputIndices->GetValue(i)]); }
      outputConstraint->SetIndices (outputIndices);
      // Sense and value can just be copied:
      // the solver handles duplicates correctly.
      outputConstraint->SetConstraintType (inputConstraint->GetConstraintType());
      vtkDataArray* sense = inputConstraint->GetAttributes()->GetArray("SENSE");
      if (sense == NULL)
        {
        vtkErrorMacro(<<"Missing constraint attribute array SENSE.");
        return VTK_ERROR;
        }
      n88_assert (sense->GetNumberOfTuples() == N);
      outputConstraint->GetAttributes()->AddArray (sense);
      vtkDataArray* value = inputConstraint->GetAttributes()->GetArray("VALUE");
      if (value == NULL)
        {
        vtkErrorMacro(<<"Missing constraint attribute array VALUE.");
        return VTK_ERROR;
        }
      n88_assert (value->GetNumberOfTuples() == N);
      outputConstraint->GetAttributes()->AddArray (value);
      output->GetConstraints()->AddItem(outputConstraint);
      }
    else
      {
      vtkErrorMacro (<< "Cannot handle element constraints.");
      return VTK_ERROR;
      }
    }

  return 1;
  }


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::GenerateConvergenceSet
  (
  vtkboneFiniteElementModel* output,
  vtkboneFiniteElementModel* input,
  const unsigned int* pointMap
  )
  {
  vtkboneConstraint* inputConvergenceSet = input->GetConvergenceSet();
  if (inputConvergenceSet)
    {
    if (inputConvergenceSet->GetConstraintAppliedTo() == vtkboneConstraint::NODES)
      {
      vtkSmartPointer<vtkboneConstraint> outputConvergenceSet =
          vtkSmartPointer<vtkboneConstraint>::New();
      outputConvergenceSet->SetName (inputConvergenceSet->GetName());
      outputConvergenceSet->SetConstraintAppliedTo (inputConvergenceSet->GetConstraintAppliedTo());
      outputConvergenceSet->SetConstraintType (inputConvergenceSet->GetConstraintType());
      vtkIdTypeArray* inputIndices = inputConvergenceSet->GetIndices();
      vtkSmartPointer<vtkIdTypeArray> outputIndices =
          vtkSmartPointer<vtkIdTypeArray>::New();
      vtkIdType N = inputIndices->GetNumberOfTuples();
      outputIndices->SetNumberOfTuples(N);
      for (vtkIdType i=0; i<N; ++i)
        { outputIndices->SetValue (i, pointMap[inputIndices->GetValue(i)]); }
      outputConvergenceSet->SetIndices (outputIndices);
      // Sense and value can just be copied:
      // the solver handles duplicates correctly.
      outputConvergenceSet->SetConstraintType (inputConvergenceSet->GetConstraintType());
      vtkDataArray* sense = inputConvergenceSet->GetAttributes()->GetArray("SENSE");
      if (sense == NULL)
        {
        vtkErrorMacro(<<"Missing constraint attribute array SENSE.");
        return VTK_ERROR;
        }
      n88_assert (sense->GetNumberOfTuples() == N);
      outputConvergenceSet->GetAttributes()->AddArray (sense);
      vtkDataArray* value = inputConvergenceSet->GetAttributes()->GetArray("VALUE");
      if (value == NULL)
        {
        vtkErrorMacro(<<"Missing constraint attribute array VALUE.");
        return VTK_ERROR;
        }
      n88_assert (value->GetNumberOfTuples() == N);
      outputConvergenceSet->GetAttributes()->AddArray (value);
      output->SetConvergenceSet(outputConvergenceSet);
      }
    else
      {
      vtkErrorMacro (<< "Cannot handle element constraints.");
      return 0;
      }
    }

  return 1;
  }


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::GenerateNodeAndElementSets
  (
  vtkboneFiniteElementModel* output,
  vtkboneFiniteElementModel* input,
  const unsigned int* pointMap,
  const vtkIdType* cellMap
  )
  {
  for (int n=0; n < input->GetNodeSets()->GetNumberOfItems(); n++)
    {
    vtkIdTypeArray* inputIds = vtkIdTypeArray::SafeDownCast (input->GetNodeSets()->GetItem(n));
    const char* name = inputIds->GetName();
    // Use std::set to eliminate duplicates and to sort.
    std::set<vtkIdType> deduped_set;
    for (vtkIdType i=0; i<inputIds->GetNumberOfTuples(); ++i)
      {
      deduped_set.insert(pointMap[inputIds->GetValue(i)]);
      }
    vtkSmartPointer<vtkIdTypeArray> outputIds = vtkSmartPointer<vtkIdTypeArray>::New();
    outputIds->SetName (name);
    outputIds->SetNumberOfTuples (deduped_set.size());
    vtkIdType i = 0;
    for (std::set<vtkIdType>::const_iterator it = deduped_set.begin();
         it != deduped_set.end();
         ++it)
      {
      outputIds->SetValue(i,*it);
      ++i;
      }
    output->GetNodeSets()->AddItem(outputIds);
    }

  for (int n=0; n < input->GetElementSets()->GetNumberOfItems(); n++)
    {
    vtkIdTypeArray* inputIds = vtkIdTypeArray::SafeDownCast (input->GetElementSets()->GetItem(n));
    const char* name = inputIds->GetName();
    // Use std::set to eliminate duplicates and to sort.
    std::set<vtkIdType> deduped_set;
    for (vtkIdType i=0; i<inputIds->GetNumberOfTuples(); ++i)
      {
      deduped_set.insert(cellMap[inputIds->GetValue(i)]);
      }
    vtkSmartPointer<vtkIdTypeArray> outputIds = vtkSmartPointer<vtkIdTypeArray>::New();
    outputIds->SetName (name);
    outputIds->SetNumberOfTuples (deduped_set.size());
    vtkIdType i = 0;
    for (std::set<vtkIdType>::const_iterator it = deduped_set.begin();
         it != deduped_set.end();
         ++it)
      {
      outputIds->SetValue(i,*it);
      ++i;
      }
    output->GetElementSets()->AddItem(outputIds);
    }

  return 1;
  }


//----------------------------------------------------------------------------
int vtkboneCoarsenModel::GenerateAdditionalInformation
  (
  vtkboneFiniteElementModel* output,
  vtkboneFiniteElementModel* input
  )
  {
  vtkInformation* inputInfo = input->GetInformation();
  vtkInformation* outputInfo = output->GetInformation();
  vtkInformationStringVectorKey* postProcessingNodeSetsKey = 
                         vtkboneSolverParameters::POST_PROCESSING_NODE_SETS();
  if (postProcessingNodeSetsKey->Has(inputInfo))
    {
    int numNodeSets = postProcessingNodeSetsKey->Length(inputInfo);
    for (int n=0; n<numNodeSets; ++n)
      {
      const char* setName = postProcessingNodeSetsKey->Get(inputInfo,n);
      vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append(outputInfo, setName);
      }    
    }
  vtkInformationStringVectorKey* postProcessingElementSetsKey = 
                         vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS();
  if (postProcessingElementSetsKey->Has(inputInfo))
    {
    int numElementSets = postProcessingElementSetsKey->Length(inputInfo);
    for (int n=0; n<numElementSets; ++n)
      {
      const char* setName = postProcessingElementSetsKey->Get(inputInfo,n);
      vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append(outputInfo, setName);
      }    
    }

  return 1;
  }
