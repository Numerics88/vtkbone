#include "vtkboneFaimVersion5InputWriter.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkboneMaterialTable.h"
#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkboneLinearOrthotropicMaterial.h"
#include "vtkboneConstraint.h"
#include "vtkboneConstraintCollection.h"
#include "vtkboneConstraintUtilities.h"
#include "vtkboneSolverParameters.h"
#include "vtkIdList.h"
#include "vtkDoubleArray.h"
#include "vtkCellTypes.h"
#include "vtkCellLocator.h"
#include "vtkSpatialRepresentationFilter.h"
#include "vtkHexahedron.h"
#include "vtkTetra.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkGenericCell.h"
#include "vtkByteSwap.h"
#include "vtkMath.h"
#include "vtkDoubleArray.h"
#include "vtkCharArray.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationStringVectorKey.h"
#include "vtkSmartPointer.h"
#include <boost/format.hpp>
#include <cmath>
#include <fstream>
#include <iostream>

vtkStandardNewMacro(vtkboneFaimVersion5InputWriter);

const float VTKBONE_FAIM_INPUT_WRITER_VERSION = 5.1;

//----------------------------------------------------------------------------
vtkboneFaimVersion5InputWriter::vtkboneFaimVersion5InputWriter()
:
  FileName                  (NULL),
  DisplacementTolerance     (1E-8)
{
}

//----------------------------------------------------------------------------
vtkboneFaimVersion5InputWriter::~vtkboneFaimVersion5InputWriter()
{
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
void vtkboneFaimVersion5InputWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5InputWriter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkboneFiniteElementModel");
  return 1;
}

//----------------------------------------------------------------------------
void vtkboneFaimVersion5InputWriter::WriteData()
{
  using boost::format;

  vtkboneFiniteElementModel *input = vtkboneFiniteElementModel::SafeDownCast(this->GetInput());
  if (input == NULL)
    {
    vtkErrorMacro(<<"No input data");
    return;   
    }

  // Checks and setup.
  int elementType = input->GetElementType();
  if (elementType != vtkboneFiniteElementModel::N88_TETRAHEDRON &&
      elementType != vtkboneFiniteElementModel::N88_HEXAHEDRON)
    {
    vtkErrorMacro(<<"The mesh writer currently is designed for either a pure tetrahedron"
                  <<" or hexahedron mesh.");
    return;
    }

  //
  // Open output file.
  //
  ofstream fp;
  fp.open (this->FileName, ios::out);
  if (fp.fail())
    {
    vtkErrorMacro(<< "Unable to open file " << this->FileName);
    return;
    }

  vtkDebugMacro(<<"\n  Writing file " << this->FileName << ".");

  WriteMeshOutput (&fp, input);

  return;
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5InputWriter::WriteNodes (ostream *fp, vtkboneFiniteElementModel *model)
{
  using boost::format;

  vtkPoints* points = model->GetPoints();
  vtkIdType npts = points->GetNumberOfPoints();

  for (vtkIdType i=0; i<npts; i++)
    {
    double x[3];
    points->GetPoint(i,x);
    *fp << format("%.6f %.6f %.6f\n") % x[0] % x[1] % x[2];
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5InputWriter::WriteElements
(
  ostream *fp,
  vtkboneFiniteElementModel* model
)
{

  // Convert from VTK topology to FE topology.
  // For definitions of these topologies, refer to:
  // A. The Visualization Toolkit, section 5.4 Cell Types
  // B. Programming the Finite Element Method, section 2.13,
  //      Three-dimensional stress and strain.
  int hexahedronTransform[8] = {0,4,5,1,3,7,6,2};
  int voxelTransform[8] = {0,4,5,1,2,6,7,3};
  int tetrahedronTransform[4] = {0,1,2,3};
  
  int ncells = model->GetNumberOfCells();
  for (vtkIdType id=0; id<ncells; id++)
    {
    int* transform;
    switch (model->GetCellType(id))
      {
      case VTK_VOXEL:
        transform = voxelTransform;
        break;
      case VTK_HEXAHEDRON:
        transform = hexahedronTransform;
        break;
      case VTK_TETRA:
        transform = tetrahedronTransform;
        break;
      default:
        vtkErrorMacro(<<"Unsupported Element Type");
        return 0;
      }
    vtkIdType npts;
    vtkIdType* pts;
    model->GetCellPoints(id, npts, pts);
    for (int p=0; p<npts; p++)
      {
      if (p > 0)
        {
        *fp << " ";
        }
      *fp << pts[transform[p]]+1;  // 1-based
      }
    *fp << "\n";
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5InputWriter::WriteMeshOutput
(
  ostream *fp,
  vtkboneFiniteElementModel *model
)
{
  using boost::format;

  vtkInformation* info = model->GetInformation();

  vtkDebugMacro(<<"\n  Writing FAIM file format.");

  vtkboneMaterialTable* materialTable = model->GetMaterialTable();

  *fp << format("# Faim Version: %.2f\n") % VTKBONE_FAIM_INPUT_WRITER_VERSION;

  *fp << "# FILEHEADER: Input for finite element analysis (faim).\n";
  *fp << format("# Generated by vtkboneFaimVersion5InputWriter %.1f\n") % VTKBONE_FAIM_INPUT_WRITER_VERSION;
  *fp << "# \n";
  int elementType = model->GetElementType();
  *fp << format("# %-45s %s\n") % "Model type:" %
          vtkboneFiniteElementModel::GetElementTypeAsString(elementType);
  *fp << "# \n";
  *fp << "# DESCRIPTION OF FILE CONTENTS\n";
  *fp << "# line1: element_type, nels, nn, nip, nodof, nod, nst, ndim\n";
  *fp << "# line2: aa, bb, cc (isotropic element dimenions in mm)\n";
  *fp << "# line3: nprops, ntype (element properties and types)\n";
  *fp << "# line4: material properties\n";
  *fp << "# line5: convergence_tol, iter_limit, maxc, maxt, iters, iters_exit\n";
  *fp << "# line : Nodal Coordinates:     x, y, z (mm)\n";
  *fp << "# line : Element connectivity:  n1, n2, n3, n4, n5, n6, n7, n8\n";
  *fp << "# line : Material IDs:          #elements; mat# (1 - 127)\n";
  *fp << "# line : Nodal contraints:      #nodes; nd#, x, y, z (0->constrained, 1->free)\n";
  *fp << "# line : Force contraints:      #nodes; nd#, x, y, z (N)\n";
  *fp << "# line : Fixed displacements:   #nodes; nd#, coord_dir (1,2,3), disp (mm)\n";
  *fp << "# line : Number of node sets:   #sets\n";
  *fp << "# line : Top node set:          #nodes; nd#\n";
  *fp << "# line : Btm node set:          #nodes; nd#\n";
  *fp << "# line : Number of el sets:     #sets\n";
  *fp << "# line : Top elmt set:          #els;   el#\n";
  *fp << "# line : Btm elmt set:          #els;   el#\n";
  *fp << "# \n";
  *fp << "# Begin model:\n";
  *fp << "# \n";

  *fp << ((elementType == vtkboneFiniteElementModel::N88_HEXAHEDRON)
           ? "'hexahedron' " : "'tetrahedron' ");
  *fp << model->GetNumberOfCells() << " ";
  *fp << model->GetNumberOfPoints() << " ";
  *fp << "8 3 8 6 3\n";

  *fp << format("%.3f %.3f %.3f\n")
        % (model->GetCell(0)->GetBounds()[1] - model->GetCell(0)->GetBounds()[0])
        % (model->GetCell(0)->GetBounds()[3] - model->GetCell(0)->GetBounds()[2])
        % (model->GetCell(0)->GetBounds()[5] - model->GetCell(0)->GetBounds()[4]);

    {  // scope
    *fp << format("%d %d\n") % 9 % materialTable->GetNumberOfMaterials();
    materialTable->InitTraversal();
    int index = materialTable->GetNextIndex();
    while (index)
      {
      if (vtkboneLinearIsotropicMaterial* material =
          vtkboneLinearIsotropicMaterial::SafeDownCast(materialTable->GetCurrentMaterial()))
        {
        *fp << format("%d %.4f %.4f %.4f %.6f %.6f %.6f %.4f %.4f %.4f\n")
        % index
        % material->GetYoungsModulus()
        % material->GetYoungsModulus()
        % material->GetYoungsModulus()
        % material->GetPoissonsRatio()
        % material->GetPoissonsRatio()
        % material->GetPoissonsRatio()
        % material->GetShearModulus()
        % material->GetShearModulus()
        % material->GetShearModulus();
        index = materialTable->GetNextIndex();
        continue;
        }
      if (vtkboneLinearOrthotropicMaterial* material =
          vtkboneLinearOrthotropicMaterial::SafeDownCast(materialTable->GetCurrentMaterial()))
        {
        *fp << format("%d %.4f %.4f %.4f %.6f %.6f %.6f %.4f %.4f %.4f\n")
        % index
        % material->GetYoungsModulusX()
        % material->GetYoungsModulusY()
        % material->GetYoungsModulusZ()
        % material->GetPoissonsRatioXY()
        % material->GetPoissonsRatioYZ()
        % material->GetPoissonsRatioZX()
        % material->GetShearModulusXY()
        % material->GetShearModulusYZ()
        % material->GetShearModulusZX();
        index = materialTable->GetNextIndex();
        continue;
        }
      vtkErrorMacro(<<"Unsupported material type");
      return 0;
      }   // while (index)
    }  //scope

  double convergenceTolerance = 0.0001;
  vtkInformationDoubleKey* convergenceToleranceKey = vtkboneSolverParameters::CONVERGENCE_TOLERANCE();
  if (convergenceToleranceKey->Has(info) != 0)
    {
    convergenceTolerance = convergenceToleranceKey->Get(info);
    }
  int maximumIterations = 30000;
  vtkInformationIntegerKey* maximumIterationsKey = vtkboneSolverParameters::MAXIMUM_ITERATIONS();
  if (maximumIterationsKey->Has(info) != 0)
    {
    maximumIterations = maximumIterationsKey->Get(info);
    }
  *fp << format("%.8f %d -1E6 1E6 1 1\n")
    % convergenceTolerance
    % maximumIterations;

  *fp << "\n# Nodes\n";
  this->WriteNodes (fp, model);

  *fp << "\n# Elements\n";
  this->WriteElements (fp, model);

  int matnumID = model->GetNumberOfCells();                                  // Material IDs
  *fp << "\n# Mat Ids\n";
  for(int k=0; k<matnumID; k++)
    {
    *fp << model->GetCellData()->GetScalars()->GetTuple1(k) << endl;
    }

  *fp << "\n# Boundary conditions\n";                    // Boundary Conditions

  this->WriteFixedConstraints (fp, model);

  this->WriteForceConstraints (fp, model);

  this->WriteDisplacementConstraints (fp, model);

  *fp << "\n# Node and element sets\n";
  
  // Write node sets
  vtkInformationStringVectorKey* postProcessingNodeSetsKey = vtkboneSolverParameters::POST_PROCESSING_NODE_SETS();
  if (postProcessingNodeSetsKey->Has(info) == 0)
    {
    *fp << "0\n";
    }
  else
    {
    int numNodeSets = postProcessingNodeSetsKey->Length(info);
    *fp << numNodeSets << "\n";
    for (int n=0; n<numNodeSets; n++)
      {
      const char* nodeSetName = postProcessingNodeSetsKey->Get(info,n);
      this->WriteNodeSet (fp, model, nodeSetName);
      }
    }
    
  // Write elements sets
  // Here we actually get the same node sets, but write the element Ids
  // of all elements that contain a node from the node set.
  if (postProcessingNodeSetsKey->Has(info) == 0)
    {
    *fp << "0\n";
    }
  else
    {
    int numNodeSets = postProcessingNodeSetsKey->Length(info);
    *fp << numNodeSets << "\n";
    for (int n=0; n<numNodeSets; n++)
      {
      const char* nodeSetName = postProcessingNodeSetsKey->Get(info,n);
      this->WriteContainingElementSet (fp, model, nodeSetName);
      }
    }
    
  return 1;
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5InputWriter::WriteFixedConstraints (ostream *fp, vtkboneFiniteElementModel *model)
{
  using boost::format;

  vtkSmartPointer<vtkboneConstraint> fixedConstraints =
      vtkSmartPointer<vtkboneConstraint>::Take(
        vtkboneConstraintUtilities::GatherZeroValuedDisplacementConstraints(model, this->DisplacementTolerance));
  if (fixedConstraints.GetPointer() == NULL)
    {
    vtkErrorMacro (<< "Error processing fixed constraints.");
    return 0;
    }
  vtkIdTypeArray* ids = fixedConstraints->GetIndices();
  vtkDataArray* senses = fixedConstraints->GetAttributes()->GetArray("SENSE");
  // These should never be NULL, but assert that anyway
  assert(ids != NULL);
  assert(senses != NULL);

  vtkIdType N = fixedConstraints->GetNumberOfValues();
  *fp << N << "\n";
  for (vtkIdType i=0; i<N; i++)
    {
    // 1-based output
    if (fabs(senses->GetTuple1(i) - 0) < 1E-8)
      {
      *fp << format("%d 0 1 1\n") % (ids->GetValue(i)+1);
      }
    else if (fabs(senses->GetTuple1(i) - 1) < 1E-8)
      {
      *fp << format("%d 1 0 1\n") % (ids->GetValue(i)+1);
      }
    else if (fabs(senses->GetTuple1(i) - 2) < 1E-8)
      {
      *fp << format("%d 1 1 0\n") % (ids->GetValue(i)+1);
      }
    else
      {
      vtkErrorMacro (<< "Invalid axis value.");
      return 0;
      }
    }
  if (N == 0)
    {
    *fp << "\n";
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5InputWriter::WriteDisplacementConstraints (ostream *fp, vtkboneFiniteElementModel *model)
{
  using boost::format;

  vtkSmartPointer<vtkboneConstraint> displacementConstraints =
      vtkSmartPointer<vtkboneConstraint>::Take(
        vtkboneConstraintUtilities::GatherNonzeroDisplacementConstraints(model, this->DisplacementTolerance));
  if (displacementConstraints.GetPointer() == NULL)
    {
    vtkErrorMacro (<< "Error processing displacement constraints.");
    return 0;
    }
  vtkIdTypeArray* ids = displacementConstraints->GetIndices();
  vtkDataArray* senses = displacementConstraints->GetAttributes()->GetArray("SENSE");
  vtkDataArray* values = displacementConstraints->GetAttributes()->GetArray("VALUE");
  // These should never be NULL, but assert that anyway
  assert(ids != NULL);
  assert(senses != NULL);
  assert(values != NULL);

  vtkIdType N = displacementConstraints->GetNumberOfValues();
  *fp << N << "\n";
  for (vtkIdType i=0; i<N; i++)
    {
    // 1-based output
    *fp << format("%d %d %.6g\n") % (ids->GetValue(i)+1)
        % (int)(senses->GetTuple1(i) + 1) % values->GetTuple1(i);
    }
  if (N == 0)
    {
    *fp << "\n";
    }
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5InputWriter::WriteForceConstraints (ostream *fp, vtkboneFiniteElementModel *model)
{
  using boost::format;

  vtkSmartPointer<vtkboneConstraint> forceConstraints =
      vtkSmartPointer<vtkboneConstraint>::Take(
        vtkboneConstraintUtilities::DistributeForceConstraintsToNodes(model));
  if (forceConstraints.GetPointer() == NULL)
    {
    vtkErrorMacro (<< "Error processing force constraints.");
    return 0;
    }
  vtkIdTypeArray* ids = forceConstraints->GetIndices();
  vtkDataArray* senses = forceConstraints->GetAttributes()->GetArray("SENSE");
  vtkDataArray* values = forceConstraints->GetAttributes()->GetArray("VALUE");
  // These should never be NULL, but assert that anyway
  assert(ids != NULL);
  assert(senses != NULL);
  assert(values != NULL);

  vtkIdType N = forceConstraints->GetNumberOfValues();
  *fp << N << "\n";
  for (vtkIdType i=0; i<N; i++)
    {
    // 1-based output
    *fp << format("%d %d %.6g\n") % (ids->GetValue(i)+1)
        % (int)(senses->GetTuple1(i) + 1) % values->GetTuple1(i);
    }
  if (N == 0)
    {
    *fp << "\n";
    }
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5InputWriter::WriteNodeSet
(
  ostream *fp,
  vtkboneFiniteElementModel* model,
  const char* setName
)
{
  using boost::format;

  vtkDebugMacro (<< "Writing node set " << setName);

  vtkIdTypeArray* ids  = model->GetNodeSet (setName);
  if (!ids)
    {
    vtkWarningMacro(<<"Missing selection: " << setName);
    *fp << "0\n\n";
    return 1;
    }

  vtkIdType N = ids->GetNumberOfTuples();
  *fp << N << "\n";

  for (int i=0; i<N; i++)
    {
    if (i>0) *fp << " ";
    *fp << format("%d") % (ids->GetValue(i)+1);  // 1-based
    if (!((i+1)%8) && i!=N-1) *fp << "\n";
    }
  *fp << "\n";

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5InputWriter::WriteContainingElementSet
(
  ostream *fp,
  vtkboneFiniteElementModel* model,
  const char* setName
)
{
  using boost::format;

  vtkDebugMacro (<< "Writing elements for set " << setName);

  vtkIdTypeArray* ids = model->GetAssociatedElementsFromNodeSet (setName);
  if (!ids)
    {
    vtkWarningMacro(<<"Missing selection: " << setName);
    *fp << "0\n\n";
    return 1;
    }

  vtkIdType N = ids->GetNumberOfTuples();
  *fp << N << "\n";

  for (int i=0; i<N; i++)
    {
    if (i>0) *fp << " ";
    *fp << format("%d") % (ids->GetValue(i)+1);  // 1-based
    if (!((i+1)%8) && i!=N-1) *fp << "\n";
    }
  *fp << "\n";

  return 1;
}
