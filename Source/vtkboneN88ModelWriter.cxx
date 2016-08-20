#include "vtkboneN88ModelWriter.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkboneMaterialTable.h"
#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkboneLinearOrthotropicMaterial.h"
#include "vtkboneLinearAnisotropicMaterial.h"
#include "vtkboneVonMisesIsotropicMaterial.h"
#include "vtkboneMaximumPrincipalStrainIsotropicMaterial.h"
#include "vtkboneMohrCoulombIsotropicMaterial.h"
#include "vtkboneLinearIsotropicMaterialArray.h"
#include "vtkboneLinearOrthotropicMaterialArray.h"
#include "vtkboneLinearAnisotropicMaterialArray.h"
#include "vtkboneConstraint.h"
#include "vtkboneConstraintCollection.h"
#include "vtkboneConstraintUtilities.h"
#include "vtkboneSolverParameters.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkLongLongArray.h"
#include "vtkUnsignedLongLongArray.h"
#include "vtkLongArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkIntArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkShortArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkCharArray.h"
#include "vtkSignedCharArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkIdTypeArray.h"
#include "vtkFloatArray.h"
#include "vtkDataArrayCollection.h"
#include "vtkCollectionIterator.h"
#include "vtkDoubleArray.h"
#include "vtkSmartPointer.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationStringVectorKey.h"
#ifdef VTKBONE_USE_VTKNETCDF
#include "vtk_netcdf.h"
#else
#include "netcdf.h"
#endif
#include "n88util/exception.hpp"
#include <boost/format.hpp>
#include <set>
#include <map>

vtkStandardNewMacro(vtkboneN88ModelWriter);

const int deflate_level = 7;

// This can only be used in a function that can return VTK_ERROR.
#define NC_SAFE_CALL(x) \
{ \
  int sc_status = (x); \
  if (sc_status != NC_NOERR) \
    { \
    vtkErrorMacro(<< "NetCDF error " <<  sc_status << ": " << nc_strerror(sc_status) << "."); \
    return VTK_ERROR; \
    } \
}

const size_t CHUNK_SIZE = 1<<22;


//----------------------------------------------------------------------------
vtkboneN88ModelWriter::vtkboneN88ModelWriter()
:
  FileName (NULL),
  Compression (0)
{
}

//----------------------------------------------------------------------------
vtkboneN88ModelWriter::~vtkboneN88ModelWriter()
{
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
void vtkboneN88ModelWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "Compression: " << this->Compression << "\n";
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkboneFiniteElementModel");
  return 1;
}

//----------------------------------------------------------------------------
void vtkboneN88ModelWriter::WriteData()
{
  using boost::format;
  int status;
  int ncid;

  vtkboneFiniteElementModel *model = vtkboneFiniteElementModel::SafeDownCast(this->GetInput());
  if (model == NULL)
    {
    vtkErrorMacro(<<"No input data.");
    return;   
    }

  vtkDebugMacro(<<"\n  Writing file " << this->FileName << ".");

  // Open output file.
  status = nc_create (this->FileName, NC_NETCDF4, &ncid);
  if (status != NC_NOERR)
    {
    vtkErrorMacro(<< "Unable to open file " << this->FileName
                  << " ; NetCDF error " <<  nc_strerror(status));
    return;
    }

  // First we "define" the NetCDF-4 file.  It is possible to to define and
  // write data as we go, but it is faster to define first and write
  // the later subsequently, when the file is completely defined.

  int vtk_status = this->DefineNetCDFFile(ncid, model);
  if (vtk_status == VTK_ERROR)
  {
      // No need to report VTK error; should have already been done.
      status = nc_close (ncid);
      return;
  }
  // This is actually not necessary for NetCFD-4 files, but we do it anyway.
  status = nc_enddef(ncid);
  if (status != NC_NOERR)
    {
    vtkErrorMacro(<< "Error writing file " << this->FileName
                  << " ; NetCDF error " <<  nc_strerror(status));
    status = nc_close (ncid);
    return;
    }

  vtk_status = this->WriteDataToNetCDFFile(ncid, model);
  if (vtk_status == VTK_ERROR)
  {
      // No need to report VTK error; should have already been done.
      status = nc_close (ncid);
      return;
  }

  status = nc_close (ncid);
  if (status != NC_NOERR)
    {
    vtkErrorMacro(<< "Error writing file " << this->FileName
                  << " ; NetCDF error " <<  nc_strerror(status));
    return;
    }

  return;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::DefineNetCDFFile
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  // Root-level attributes
  const char* value = "Numerics88/Finite_Element_Model-1.0";
  NC_SAFE_CALL (nc_put_att_text (ncid, NC_GLOBAL, "Conventions", strlen(value), value));

  if (model->GetHistory())
    {
    NC_SAFE_CALL (nc_put_att_text (ncid, NC_GLOBAL, "History", strlen(model->GetHistory()), model->GetHistory()));
    }
  if (model->GetLog())
    {
    NC_SAFE_CALL (nc_put_att_text (ncid, NC_GLOBAL, "Log", strlen(model->GetLog()), model->GetLog()));
    }

  value = "Problem1";
  NC_SAFE_CALL (nc_put_att_text (ncid, NC_GLOBAL, "ActiveProblem", strlen(value), value));

  // Figure out if we need to create an active solution.
  bool solutionRequired = false;
  std::set<std::string> arrayNames;
  if (this->GetSolutionArrayNames(model->GetPointData(), arrayNames) == VTK_OK)
    {
    if (arrayNames.size() > 0)
      { solutionRequired = true; }
    }
  if (!solutionRequired)
    {
    if (this->GetSolutionArrayNames(model->GetCellData(), arrayNames) == VTK_OK)
      {
      if (arrayNames.size() > 0)
        { solutionRequired = true; }
      }
    }
  if (solutionRequired)
    {
    value = "Solution1";
    NC_SAFE_CALL (nc_put_att_text (ncid, NC_GLOBAL, "ActiveSolution", strlen(value), value));
    }

  // Root-level dimensions
  int dimensionality_dimid;
  NC_SAFE_CALL (nc_def_dim (ncid, "Dimensionality", 3, &dimensionality_dimid));

  if (this->DefineMaterialDefinitions(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->DefinePart(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->DefineMaterialTable(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->DefineConstraints(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->DefineSets(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->DefineProblem(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->DefineSolution(ncid, model) == VTK_ERROR) return VTK_ERROR;
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteDataToNetCDFFile
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  if (this->WriteMaterialDefinitions(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->WriteNodes(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->WriteMaterialTable(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->WriteElements(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->WriteConstraints(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->WriteSets(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->WriteSolution(ncid, model) == VTK_ERROR) return VTK_ERROR;
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::DefineMaterialDefinitions
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  vtkboneMaterialTable* materialTable = model->GetMaterialTable();
  if (!materialTable || materialTable->GetNumberOfMaterials() == 0)
    {
    // Ignore if no MaterialTable
    return VTK_OK;
    }

  int materialDefinitions_ncid;
  NC_SAFE_CALL (nc_def_grp(ncid, "MaterialDefinitions", &materialDefinitions_ncid));

  if (materialTable->CheckNames() == 0)
    {
    vtkErrorMacro(<< "Material names are not unique.");
    return VTK_ERROR;
    }
  materialTable->InitTraversal();
  
  while (int index = materialTable->GetNextUniqueIndex())
    {
    vtkboneMaterial* material = materialTable->GetCurrentMaterial();
    int material_ncid;
    NC_SAFE_CALL (nc_def_grp(materialDefinitions_ncid, material->GetName(), &material_ncid));
    if (vtkboneVonMisesIsotropicMaterial* vmmat =
        vtkboneVonMisesIsotropicMaterial::SafeDownCast(material))
      {
      const char* value = "VonMisesIsotropic";
      NC_SAFE_CALL (nc_put_att_text (material_ncid, NC_GLOBAL, "Type", strlen(value), value));
      double double_value = vmmat->GetYoungsModulus();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "E", NC_DOUBLE, 1, &double_value));
      double_value = vmmat->GetPoissonsRatio();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "nu", NC_DOUBLE, 1, &double_value));
      double_value = vmmat->GetYieldStrength();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "Y", NC_DOUBLE, 1, &double_value));
      continue;
      }
    if (vtkboneMaximumPrincipalStrainIsotropicMaterial* mpsmat =
        vtkboneMaximumPrincipalStrainIsotropicMaterial::SafeDownCast(material))
      {
      const char* value = "MaximumPrincipalStrainIsotropic";
      NC_SAFE_CALL (nc_put_att_text (material_ncid, NC_GLOBAL, "Type", strlen(value), value));
      double double_value = mpsmat->GetYoungsModulus();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "E", NC_DOUBLE, 1, &double_value));
      double_value = mpsmat->GetPoissonsRatio();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "nu", NC_DOUBLE, 1, &double_value));
      double_value = mpsmat->GetMaximumTensilePrincipalStrain();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "epsilon_YT", NC_DOUBLE, 1, &double_value));
      double_value = mpsmat->GetMaximumCompressivePrincipalStrain();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "epsilon_YC", NC_DOUBLE, 1, &double_value));
      continue;
      }
    if (vtkboneMohrCoulombIsotropicMaterial* mcmat =
        vtkboneMohrCoulombIsotropicMaterial::SafeDownCast(material))
      {
      const char* value = "MohrCoulombIsotropic";
      NC_SAFE_CALL (nc_put_att_text (material_ncid, NC_GLOBAL, "Type", strlen(value), value));
      double double_value = mcmat->GetYoungsModulus();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "E", NC_DOUBLE, 1, &double_value));
      double_value = mcmat->GetPoissonsRatio();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "nu", NC_DOUBLE, 1, &double_value));
      double_value = mcmat->GetC();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "c", NC_DOUBLE, 1, &double_value));
      double_value = mcmat->GetPhi();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "phi", NC_DOUBLE, 1, &double_value));
      continue;
      }
    if (vtkboneLinearIsotropicMaterial* isomat =
        vtkboneLinearIsotropicMaterial::SafeDownCast(material))
      {
      const char* value = "LinearIsotropic";
      NC_SAFE_CALL (nc_put_att_text (material_ncid, NC_GLOBAL, "Type", strlen(value), value));
      double double_value = isomat->GetYoungsModulus();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "E", NC_DOUBLE, 1, &double_value));
      double_value = isomat->GetPoissonsRatio();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "nu", NC_DOUBLE, 1, &double_value));
      continue;
      }
    if (vtkboneLinearOrthotropicMaterial* orthomat =
        vtkboneLinearOrthotropicMaterial::SafeDownCast(material))
      {
      const char* value = "LinearOrthotropic";
      NC_SAFE_CALL (nc_put_att_text (material_ncid, NC_GLOBAL, "Type", strlen(value), value));
      double double_vec[3];
      double_vec[0] = orthomat->GetYoungsModulusX();
      double_vec[1] = orthomat->GetYoungsModulusY();
      double_vec[2] = orthomat->GetYoungsModulusZ();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "E", NC_DOUBLE, 3, double_vec));
      double_vec[0] = orthomat->GetPoissonsRatioYZ();
      double_vec[1] = orthomat->GetPoissonsRatioZX();
      double_vec[2] = orthomat->GetPoissonsRatioXY();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "nu", NC_DOUBLE, 3, double_vec));
      double_vec[0] = orthomat->GetShearModulusYZ();
      double_vec[1] = orthomat->GetShearModulusZX();
      double_vec[2] = orthomat->GetShearModulusXY();
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "G", NC_DOUBLE, 3, double_vec));
      continue;
      }
    if (vtkboneLinearAnisotropicMaterial* anisomat =
        vtkboneLinearAnisotropicMaterial::SafeDownCast(material))
      {
      const char* value = "LinearAnisotropic";
      NC_SAFE_CALL (nc_put_att_text (material_ncid, NC_GLOBAL, "Type", strlen(value), value));
      NC_SAFE_CALL (nc_put_att_double (material_ncid, NC_GLOBAL, "StressStrainMatrix", NC_DOUBLE, 6*6,
                                       anisomat->GetStressStrainMatrix()));
      continue;
      }
    if (vtkboneLinearIsotropicMaterialArray* isomatarray =
        vtkboneLinearIsotropicMaterialArray::SafeDownCast(material))
      {
      const char* value = "LinearIsotropic";
      NC_SAFE_CALL (nc_put_att_text (material_ncid, NC_GLOBAL, "Type", strlen(value), value));
      int nmats_dimid;
      NC_SAFE_CALL (nc_def_dim (material_ncid, "MaterialArrayLength", isomatarray->GetSize(), &nmats_dimid));
      int E_varid;
      NC_SAFE_CALL (nc_def_var (material_ncid, "E", NC_FLOAT, 1, &nmats_dimid, &E_varid));
      NC_SAFE_CALL (SetChunking (material_ncid, E_varid));
      int nu_varid;
      NC_SAFE_CALL (nc_def_var (material_ncid, "nu", NC_FLOAT, 1, &nmats_dimid, &nu_varid));
      NC_SAFE_CALL (SetChunking (material_ncid, nu_varid));
      continue;
      }
    if (vtkboneLinearOrthotropicMaterialArray* orthomatarray =
        vtkboneLinearOrthotropicMaterialArray::SafeDownCast(material))
      {
      const char* value = "LinearOrthotropic";
      NC_SAFE_CALL (nc_put_att_text (material_ncid, NC_GLOBAL, "Type", strlen(value), value));
      int nmats_dimid;
      NC_SAFE_CALL (nc_def_dim (material_ncid, "MaterialArrayLength", orthomatarray->GetSize(), &nmats_dimid));
      int comp_dimid;
      NC_SAFE_CALL (nc_def_dim (material_ncid, "NComponents", 3, &comp_dimid));
      int dimids[2];
      dimids[0] = nmats_dimid;
      dimids[1] = comp_dimid;
      int E_varid;
      NC_SAFE_CALL (nc_def_var (material_ncid, "E", NC_FLOAT, 2, dimids, &E_varid));
      NC_SAFE_CALL (SetChunking (material_ncid, E_varid));
      int nu_varid;
      NC_SAFE_CALL (nc_def_var (material_ncid, "nu", NC_FLOAT, 2, dimids, &nu_varid));
      NC_SAFE_CALL (SetChunking (material_ncid, nu_varid));
      int G_varid;
      NC_SAFE_CALL (nc_def_var (material_ncid, "G", NC_FLOAT, 2, dimids, &G_varid));
      NC_SAFE_CALL (SetChunking (material_ncid, G_varid));
      continue;
      }
    if (vtkboneLinearAnisotropicMaterialArray* anisomatarray =
        vtkboneLinearAnisotropicMaterialArray::SafeDownCast(material))
      {
      const char* value = "LinearAnisotropic";
      NC_SAFE_CALL (nc_put_att_text (material_ncid, NC_GLOBAL, "Type", strlen(value), value));
      int nmats_dimid;
      NC_SAFE_CALL (nc_def_dim (material_ncid, "MaterialArrayLength", anisomatarray->GetSize(), &nmats_dimid));
      int comp_dimid;
      NC_SAFE_CALL (nc_def_dim (material_ncid, "NComponents", 21, &comp_dimid));
      int dimids[2];
      dimids[0] = nmats_dimid;
      dimids[1] = comp_dimid;
      int K_varid;
      NC_SAFE_CALL (nc_def_var (material_ncid, "StressStrainMatrix", NC_FLOAT, 2, dimids, &K_varid));
      NC_SAFE_CALL (SetChunking (material_ncid, K_varid));
      continue;
      }
    vtkErrorMacro(<<"Unsupported material type.");
    return VTK_ERROR;
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::DefinePart
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  int dimensionality_dimid;
  NC_SAFE_CALL (nc_inq_dimid (ncid, "Dimensionality", &dimensionality_dimid));
  int parts_ncid;
  NC_SAFE_CALL (nc_def_grp (ncid, "Parts", &parts_ncid));
  int part1_ncid;
  NC_SAFE_CALL (nc_def_grp (parts_ncid, "Part1", &part1_ncid));
  int numberOfNodes_dimid;
  NC_SAFE_CALL (nc_def_dim (part1_ncid, "NumberOfNodes", model->GetNumberOfPoints(), &numberOfNodes_dimid));
  int dimids[2];
  dimids[0] = numberOfNodes_dimid;
  dimids[1] = dimensionality_dimid;
  int nodeCoordinates_varid;
  NC_SAFE_CALL (nc_def_var (part1_ncid, "NodeCoordinates", NC_FLOAT, 2, dimids, &nodeCoordinates_varid));
  NC_SAFE_CALL (SetChunking (part1_ncid, nodeCoordinates_varid));
  int elements_ncid;
  NC_SAFE_CALL (nc_def_grp (part1_ncid, "Elements", &elements_ncid));
  int hexahedrons_ncid;
  NC_SAFE_CALL (nc_def_grp (elements_ncid, "Hexahedrons", &hexahedrons_ncid));
  int numberOfNodesPerElement_dimid;
  NC_SAFE_CALL (nc_def_dim (hexahedrons_ncid, "NumberOfNodesPerElement", 8, &numberOfNodesPerElement_dimid));
  int numberOfElements_dimid;
  NC_SAFE_CALL (nc_def_dim (hexahedrons_ncid, "NumberOfElements", model->GetNumberOfCells(), &numberOfElements_dimid));
  int elementNumber_varid;
  dimids[0] = numberOfElements_dimid;
  NC_SAFE_CALL (nc_def_var (hexahedrons_ncid, "ElementNumber", NC_UINT, 1, dimids, &elementNumber_varid));
  NC_SAFE_CALL (SetChunking (hexahedrons_ncid, elementNumber_varid));
  int nodeNumbers_varid;
  dimids[0] = numberOfElements_dimid;
  dimids[1] = numberOfNodesPerElement_dimid;
  NC_SAFE_CALL (nc_def_var (hexahedrons_ncid, "NodeNumbers", NC_UINT, 2, dimids, &nodeNumbers_varid));
  NC_SAFE_CALL (SetChunking (hexahedrons_ncid, nodeNumbers_varid));
  int materialid_varid;
  dimids[0] = numberOfElements_dimid;
  vtkDataArray* scalars = model->GetCellData()->GetScalars();
  if (scalars)
    {
      for (vtkIdType i=0; i<scalars->GetNumberOfTuples(); ++i)
        {
        double testValue = scalars->GetTuple1(i);
        if (testValue < 1 || testValue >= 2147483648.0)
          {
          vtkErrorMacro (<< "Material ID is out of range.\n");
          return VTK_ERROR;
          }
        }
    NC_SAFE_CALL (nc_def_var (hexahedrons_ncid, "MaterialID", NC_UINT, 1, dimids, &materialid_varid));
    NC_SAFE_CALL (SetChunking (hexahedrons_ncid, materialid_varid));
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::DefineMaterialTable
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  vtkboneMaterialTable* materialTable = model->GetMaterialTable();
  if (!materialTable || materialTable->GetNumberOfMaterials() == 0)
    {
    // Ignore if no material table.
    return VTK_OK;
    }
  int parts_ncid;
  NC_SAFE_CALL (nc_inq_ncid (ncid, "Parts", &parts_ncid));
  int part1_ncid;
  NC_SAFE_CALL (nc_inq_ncid (parts_ncid, "Part1", &part1_ncid));
  int materialTable_ncid;
  NC_SAFE_CALL (nc_def_grp (part1_ncid, "MaterialTable", &materialTable_ncid));
  int size_dimid;
  NC_SAFE_CALL (nc_def_dim (materialTable_ncid, "Size", materialTable->GetNumberOfMaterials(), &size_dimid));
  int dimids[1];
  dimids[0] = size_dimid;
  int id_varid;
  NC_SAFE_CALL (nc_def_var (materialTable_ncid, "ID", NC_UINT, 1, dimids, &id_varid));
  int material_varid;
  NC_SAFE_CALL (nc_def_var (materialTable_ncid, "MaterialName", NC_STRING, 1, dimids, &material_varid));

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::DefineConstraint
  (
  int constraints_ncid,
  vtkboneConstraint* constraint,
  vtkboneFiniteElementModel *model
  )
  {
  if (constraint->GetName() == NULL)
    {
    vtkErrorMacro(<< "Constraint has no name.");
    return VTK_ERROR;
    }
  int constraint_ncid;
  NC_SAFE_CALL (nc_def_grp (constraints_ncid, constraint->GetName(), &constraint_ncid));
  const char* string_value = "Part1";
  NC_SAFE_CALL (nc_put_att_text (constraint_ncid, NC_GLOBAL, "Part", strlen(string_value), string_value));
  if (constraint->GetConstraintAppliedTo() == vtkboneConstraint::NODES)
    {
    int numberOfConstraints_dimid;
    NC_SAFE_CALL (nc_def_dim (constraint_ncid, "NumberOfValues", constraint->GetNumberOfValues(), &numberOfConstraints_dimid));
    if (constraint->GetConstraintType() == vtkboneConstraint::DISPLACEMENT)
      {
      const char* string_value = "NodeAxisDisplacement";
      NC_SAFE_CALL (nc_put_att_text (constraint_ncid, NC_GLOBAL, "Type", strlen(string_value), string_value));
      int dimids[1];
      dimids[0] = numberOfConstraints_dimid;
      int varid;
      NC_SAFE_CALL (nc_def_var (constraint_ncid, "NodeNumber", NC_UINT, 1, dimids, &varid));
      NC_SAFE_CALL (SetChunking (constraint_ncid, varid));
      NC_SAFE_CALL (nc_def_var (constraint_ncid, "Sense", NC_BYTE, 1, dimids, &varid));
      NC_SAFE_CALL (SetChunking (constraint_ncid, varid));
      NC_SAFE_CALL (nc_def_var (constraint_ncid, "Value", NC_FLOAT, 1, dimids, &varid));
      NC_SAFE_CALL (SetChunking (constraint_ncid, varid));
      }
    else if (constraint->GetConstraintType() == vtkboneConstraint::FORCE)
      {
      const char* string_value = "NodeAxisForce";
      NC_SAFE_CALL (nc_put_att_text (constraint_ncid, NC_GLOBAL, "Type", strlen(string_value), string_value));
      int dimids[1];
      dimids[0] = numberOfConstraints_dimid;
      int varid;
      NC_SAFE_CALL (nc_def_var (constraint_ncid, "NodeNumber", NC_UINT, 1, dimids, &varid));
      NC_SAFE_CALL (SetChunking (constraint_ncid, varid));
      NC_SAFE_CALL (nc_def_var (constraint_ncid, "Sense", NC_BYTE, 1, dimids, &varid));
      NC_SAFE_CALL (SetChunking (constraint_ncid, varid));
      NC_SAFE_CALL (nc_def_var (constraint_ncid, "Value", NC_FLOAT, 1, dimids, &varid));
      NC_SAFE_CALL (SetChunking (constraint_ncid, varid));
      }
    else
      {
      vtkErrorMacro(<<"Unknown Constraint Type.");
      return VTK_ERROR;
      }
    }
  else if (constraint->GetConstraintAppliedTo() == vtkboneConstraint::ELEMENTS)
    {
    if (constraint->GetConstraintType() == vtkboneConstraint::DISPLACEMENT)
      {
      vtkErrorMacro(<<"Displacement constraints cannot be applied to elements.");
      return VTK_ERROR;
      }
    else if (constraint->GetConstraintType() == vtkboneConstraint::FORCE)
      {
      vtkSmartPointer<vtkboneConstraint> forceConstraints =
          vtkSmartPointer<vtkboneConstraint>::Take(
            vtkboneConstraintUtilities::DistributeConstraintToNodes(model, constraint));
      if (forceConstraints.GetPointer() == NULL)
        {
        vtkErrorMacro (<< "Error processing force constraints.");
        return VTK_ERROR;
        }
      vtkIdTypeArray* ids = forceConstraints->GetIndices();
      vtkDataArray* senses = forceConstraints->GetAttributes()->GetArray("SENSE");
      vtkDataArray* values = forceConstraints->GetAttributes()->GetArray("VALUE");
      // These should never be NULL, but assert that anyway
      n88_assert(ids != NULL);
      n88_assert(senses != NULL);
      n88_assert(values != NULL);
      const char* string_value = "NodeAxisForce";
      NC_SAFE_CALL (nc_put_att_text (constraint_ncid, NC_GLOBAL, "Type", strlen(string_value), string_value));
      int numberOfConstraints_dimid;
      NC_SAFE_CALL (nc_def_dim (constraint_ncid, "NumberOfValues", ids->GetNumberOfTuples(), &numberOfConstraints_dimid));
      int dimids[1];
      dimids[0] = numberOfConstraints_dimid;
      int varid;
      NC_SAFE_CALL (nc_def_var (constraint_ncid, "NodeNumber", NC_UINT, 1, dimids, &varid));
      NC_SAFE_CALL (SetChunking (constraint_ncid, varid));
      NC_SAFE_CALL (nc_def_var (constraint_ncid, "Sense", NC_BYTE, 1, dimids, &varid));
      NC_SAFE_CALL (SetChunking (constraint_ncid, varid));
      NC_SAFE_CALL (nc_def_var (constraint_ncid, "Value", NC_FLOAT, 1, dimids, &varid));
      NC_SAFE_CALL (SetChunking (constraint_ncid, varid));
      }
    else
      {
      vtkErrorMacro(<<"Unknown Constraint Type.");
      return VTK_ERROR;
      }
    }
  else
    {
    vtkErrorMacro(<<"Unknown Constraint Type.");
    return VTK_ERROR;
    }
  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::DefineConstraints
  (
  int ncid,
  vtkboneFiniteElementModel *model
  )
  {
  vtkboneConstraintCollection* constraints = model->GetConstraints();
  vtkboneConstraint* convergence_set = model->GetConvergenceSet();
  if ((!constraints  || constraints->GetNumberOfItems() == 0) &&
      (convergence_set == NULL))
    {
    // Ignore if no constraints.
    return VTK_OK;
    }
  int constraints_ncid;
  NC_SAFE_CALL (nc_def_grp (ncid, "Constraints", &constraints_ncid));

  if (constraints)
    {
    constraints->InitTraversal();
    while (vtkboneConstraint* constraint = constraints->GetNextItem())
      {
      int return_val = this->DefineConstraint (constraints_ncid, constraint, model);
      if (return_val != VTK_OK)
        {
        return return_val;
        }
      }
    }

  if (convergence_set)
    {
    int return_val = this->DefineConstraint (constraints_ncid, convergence_set, model);
    if (return_val != VTK_OK)
      {
      return return_val;
      }
    }

  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::DefineProblem
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  vtkInformation* info = model->GetInformation();

  int problems_ncid;
  NC_SAFE_CALL (nc_def_grp (ncid, "Problems", &problems_ncid));
  int problem1_ncid;
  NC_SAFE_CALL (nc_def_grp (problems_ncid, "Problem1", &problem1_ncid));
  const char* string_value = "Part1";
  NC_SAFE_CALL (nc_put_att_text (problem1_ncid, NC_GLOBAL, "Part", strlen(string_value), string_value));

  vtkInformationDoubleKey* convergenceToleranceKey = vtkboneSolverParameters::CONVERGENCE_TOLERANCE();
  if (convergenceToleranceKey->Has(info) != 0)
    {
    double double_value = convergenceToleranceKey->Get(info);
    NC_SAFE_CALL (nc_put_att_double (
                      problem1_ncid,
                      NC_GLOBAL,
                      "ConvergenceTolerance",
                      NC_DOUBLE,
                      1, &double_value));
    }

  vtkInformationIntegerKey* maximumIterationsKey = vtkboneSolverParameters::MAXIMUM_ITERATIONS();
  if (maximumIterationsKey->Has(info) != 0)
    {
    int int_value = maximumIterationsKey->Get(info);
    NC_SAFE_CALL (nc_put_att_int (
                      problem1_ncid,
                      NC_GLOBAL,
                      "MaximumIterations",
                      NC_INT,
                      1,
                      &int_value));
    }

  vtkInformationDoubleKey* plasticConvergenceToleranceKey = vtkboneSolverParameters::PLASTIC_CONVERGENCE_TOLERANCE();
  if (plasticConvergenceToleranceKey->Has(info) != 0)
    {
    double double_value = plasticConvergenceToleranceKey->Get(info);
    NC_SAFE_CALL (nc_put_att_double (
                      problem1_ncid,
                      NC_GLOBAL,
                      "PlasticConvergenceTolerance",
                      NC_DOUBLE,
                      1, &double_value));
    }

  vtkInformationIntegerKey* maximumPlasticIterationsKey = vtkboneSolverParameters::MAXIMUM_PLASTIC_ITERATIONS();
  if (maximumPlasticIterationsKey->Has(info) != 0)
    {
    int int_value = maximumPlasticIterationsKey->Get(info);
    NC_SAFE_CALL (nc_put_att_int (
                      problem1_ncid,
                      NC_GLOBAL,
                      "MaximumPlasticIterations",
                      NC_INT,
                      1,
                      &int_value));
    }

  vtkboneConstraintCollection* constraints = model->GetConstraints();
  if (constraints && constraints->GetNumberOfItems() > 0)
    {
    std::string constraintsList;
    constraints->InitTraversal();
    while (vtkboneConstraint* constraint = constraints->GetNextItem())
      {
      if (constraintsList.size() > 0)
        { constraintsList += ","; }
      n88_assert (constraint->GetName());
      constraintsList += constraint->GetName();
      }
    NC_SAFE_CALL (nc_put_att_text (problem1_ncid, NC_GLOBAL, "Constraints", constraintsList.size(), constraintsList.c_str()));
    }

  vtkboneConstraint* convergence_set = model->GetConvergenceSet();
  if (convergence_set)
    {
    n88_assert (convergence_set->GetName());
    NC_SAFE_CALL (nc_put_att_text (problem1_ncid,
                                   NC_GLOBAL,
                                   "ConvergenceSet",
                                   strlen(convergence_set->GetName()),
                                   convergence_set->GetName()));    
    }

  vtkInformationStringVectorKey* postProcessingNodeSetsKey = 
                         vtkboneSolverParameters::POST_PROCESSING_NODE_SETS();
  if (postProcessingNodeSetsKey->Has(info) != 0)
    {
    std::string nodeSetList;
    int numNodeSets = postProcessingNodeSetsKey->Length(info);
    for (int n=0; n<numNodeSets; n++)
      {
      const char* setName = postProcessingNodeSetsKey->Get(info,n);
      if (nodeSetList.size() > 0)
        { nodeSetList += ","; }
      nodeSetList += setName;
      }
    if (numNodeSets > 0)
      {
      NC_SAFE_CALL (nc_put_att_text (problem1_ncid, NC_GLOBAL, "PostProcessingNodeSets", nodeSetList.size(), nodeSetList.c_str()));
      }
    }

  vtkInformationStringVectorKey* postProcessingElementSetsKey =
                      vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS();
  if (postProcessingElementSetsKey->Has(info) != 0)
    {
    std::string elementSetList;
    int numElementSets = postProcessingElementSetsKey->Length(info);
    for (int n=0; n<numElementSets; n++)
      {
      const char* setName = postProcessingElementSetsKey->Get(info,n);
      if (elementSetList.size() > 0)
        { elementSetList += ","; }
      elementSetList += setName;
      }
    if (numElementSets > 0)
      {
      NC_SAFE_CALL (nc_put_att_text (problem1_ncid, NC_GLOBAL, "PostProcessingElementSets", elementSetList.size(), elementSetList.c_str()));
      }
    }

  vtkInformationDoubleVectorKey* rotationCenterKey = 
                                  vtkboneSolverParameters::ROTATION_CENTER();
  if (rotationCenterKey->Has(info) != 0)
    {
    // Ignore if not a triplet of values
    if (rotationCenterKey->Length(info) != 3)
      {
      vtkWarningMacro (<< "Information ROTATION_CENTER not a triplet of values.");
      }
    else
      {
      double rotationCenter[3];
      for (int i=0; i<3; ++i)
        { rotationCenter[i] = rotationCenterKey->Get(info,i); }
      NC_SAFE_CALL (nc_put_att_double (problem1_ncid, NC_GLOBAL, "RotationCenter", NC_DOUBLE, 3, rotationCenter));
      }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::DefineSets
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  if (model->GetNodeSets()->GetNumberOfItems() +
      model->GetElementSets()->GetNumberOfItems() == 0)
    {
    return VTK_OK;
    }
  int sets_ncid;
  NC_SAFE_CALL (nc_def_grp (ncid, "Sets", &sets_ncid));

  // Node sets
  if (model->GetNodeSets()->GetNumberOfItems() > 0)
    {
    int nodesets_ncid;
    NC_SAFE_CALL (nc_def_grp (sets_ncid, "NodeSets", &nodesets_ncid));
    for (int n=0; n<model->GetNodeSets()->GetNumberOfItems(); n++)
      {
      vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast (model->GetNodeSets()->GetItem(n));
      if (ids == NULL)
        {
        vtkErrorMacro(<<"Error accessing node set. Not type vtkIdTypeArray?");
        return VTK_ERROR;
        }    
      const char* setName = ids->GetName();
      if (setName == NULL)
        {
        vtkErrorMacro(<<"Unnamed node set.");
        return VTK_ERROR;
        }
      int set_ncid;
      NC_SAFE_CALL (nc_def_grp (nodesets_ncid, setName, &set_ncid));
      int dimid;
      NC_SAFE_CALL (nc_def_dim (set_ncid, "NumberOfNodes", ids->GetNumberOfTuples(), &dimid));
      const char* string_value = "Part1";
      NC_SAFE_CALL (nc_put_att_text (set_ncid, NC_GLOBAL, "Part", strlen(string_value), string_value));
      int varid;
      NC_SAFE_CALL (nc_def_var (set_ncid, "NodeNumber", NC_UINT, 1, &dimid, &varid));
      NC_SAFE_CALL (SetChunking (set_ncid, varid));
      }
    }

  // Element sets
  if (model->GetElementSets()->GetNumberOfItems() > 0)
    {
    int elementsets_ncid;
    NC_SAFE_CALL (nc_def_grp (sets_ncid, "ElementSets", &elementsets_ncid));
    for (int n=0; n<model->GetElementSets()->GetNumberOfItems(); n++)
      {
      vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast (model->GetElementSets()->GetItem(n));
      if (ids == NULL)
        {
        vtkErrorMacro(<<"Error accessing element set. Not type vtkIdTypeArray?");
        return VTK_ERROR;
        }    
      const char* setName = ids->GetName();
      if (setName == NULL)
        {
        vtkErrorMacro(<<"Unnamed element set.");
        return VTK_ERROR;
        }
      int set_ncid;
      NC_SAFE_CALL (nc_def_grp (elementsets_ncid, setName, &set_ncid));
      int dimid;
      NC_SAFE_CALL (nc_def_dim (set_ncid, "NumberOfElements", ids->GetNumberOfTuples(), &dimid));
      const char* string_value = "Part1";
      NC_SAFE_CALL (nc_put_att_text (set_ncid, NC_GLOBAL, "Part", strlen(string_value), string_value));
      int varid;
      NC_SAFE_CALL (nc_def_var (set_ncid, "ElementNumber", NC_UINT, 1, &dimid, &varid));
      NC_SAFE_CALL (SetChunking (set_ncid, varid));
      }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::GetSolutionArrayNames
(
  vtkDataSetAttributes* fieldData,
  std::set<std::string>& names
)
{
  names.clear();
  for (int i=0; i<fieldData->GetNumberOfArrays(); ++i)
    {
    vtkDataArray* data = fieldData->GetArray(i);
    if (data == fieldData->GetScalars() ||
        data == fieldData->GetNormals() ||
        data == fieldData->GetGlobalIds() ||
        data == fieldData->GetPedigreeIds() ||
        data->GetName() == NULL)
      { continue; }
    names.insert (data->GetName());
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::GetNeededGaussPointGroups
(
  vtkboneFiniteElementModel *model,
  std::set<size_t>& nGaussPoints
)
{
  vtkDataArrayCollection* gaussPointData = model->GetGaussPointData();
  nGaussPoints.clear();
  vtkSmartPointer<vtkCollectionIterator> iterator = 
      vtkSmartPointer<vtkCollectionIterator>::Take(gaussPointData->NewIterator());
  iterator->GoToFirstItem();
  while (iterator->IsDoneWithTraversal() == false)
    {
    vtkFloatArray* data = vtkFloatArray::SafeDownCast(iterator->GetCurrentObject());
    if (data == NULL)
      {
      vtkWarningMacro (<< "Gauss Point Data not float: discarding.");  
      }
    else
      {
      size_t numberOfValuesPerGaussPoint = data->GetNumberOfComponents();
      size_t nTuples = data->GetNumberOfTuples();
      size_t nElements = model->GetNumberOfCells();
      if (nTuples % nElements)
        {
        vtkWarningMacro (<< "Gauss Point Data not multiple of number of cells: discarding.");  
        }
      else
        {
        nGaussPoints.insert (nTuples / nElements);
        }
      }
    iterator->GoToNextItem();
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::DefineSolution
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  // First we will just count how many node and element arrays there are.
  std::set<std::string> nodeArrayNames;
  if (this->GetSolutionArrayNames(model->GetPointData(), nodeArrayNames) != VTK_OK)
    { return VTK_ERROR; }
  std::set<std::string> elementArrayNames;
  if (this->GetSolutionArrayNames(model->GetCellData(), elementArrayNames) != VTK_OK)
    { return VTK_ERROR; }

  if (nodeArrayNames.size() == 0 && elementArrayNames.size() == 0)
    { return VTK_OK; }

  int solutions_ncid;
  NC_SAFE_CALL (nc_def_grp (ncid, "Solutions", &solutions_ncid));
  int solution1_ncid;
  NC_SAFE_CALL (nc_def_grp (solutions_ncid, "Solution1", &solution1_ncid));
  const char* string_value = "Problem1";
  NC_SAFE_CALL (nc_put_att_text (solution1_ncid, NC_GLOBAL, "Problem", strlen(string_value), string_value));
  
  if (nodeArrayNames.size())
    {
    int nodeValues_ncid;
    NC_SAFE_CALL (nc_def_grp (solution1_ncid, "NodeValues", &nodeValues_ncid));
    // First generate a set of all second dimensions that we will need
    std::set<int> second_dims;
    for (std::set<std::string>::const_iterator name = nodeArrayNames.begin();
         name != nodeArrayNames.end();
         ++name)
      {
      vtkDataArray* data = model->GetPointData()->GetArray(name->c_str());
      if (data->GetNumberOfComponents() > 1)
        { second_dims.insert(data->GetNumberOfComponents()); }
      }

    // Create dimensions as required
    int nn_dimid;
    NC_SAFE_CALL (nc_def_dim (nodeValues_ncid, "NumberOfNodes", model->GetNumberOfPoints(), &nn_dimid));
    std::map<int,int> dimids_map;
    for (std::set<int>::const_iterator it=second_dims.begin();
         it != second_dims.end();
         ++it)
      {
      std::string dimName = (boost::format("Components%d") % *it).str();
      int dimid;
      NC_SAFE_CALL (nc_def_dim (nodeValues_ncid, dimName.c_str(), *it, &dimid));
      dimids_map[*it] = dimid;
      }

    // Create variables
    for (std::set<std::string>::const_iterator name = nodeArrayNames.begin();
         name != nodeArrayNames.end();
         ++name)
      {
      int varid;
      vtkDataArray* data = model->GetPointData()->GetArray(name->c_str());
      if (data->GetNumberOfComponents() == 1)
        {
        NC_SAFE_CALL (nc_def_var (nodeValues_ncid, name->c_str(), NC_FLOAT, 1, &nn_dimid, &varid));
        NC_SAFE_CALL (SetChunking (nodeValues_ncid, varid));
        }
      else
        {
        int dimids[2] = {nn_dimid, dimids_map[data->GetNumberOfComponents()]};
        NC_SAFE_CALL (nc_def_var (nodeValues_ncid, name->c_str(), NC_FLOAT, 2, dimids, &varid));
        NC_SAFE_CALL (SetChunking (nodeValues_ncid, varid));
        }
      }
    }

  std::set<size_t> nGaussPoints;
  this->GetNeededGaussPointGroups (model, nGaussPoints);

  if (elementArrayNames.size() || nGaussPoints.size())
    {
    int elementValues_ncid;
    NC_SAFE_CALL (nc_def_grp (solution1_ncid, "ElementValues", &elementValues_ncid));
    // First generate a set of all second dimensions that we will need
    std::set<int> second_dims;
    for (std::set<std::string>::const_iterator name = elementArrayNames.begin();
         name != elementArrayNames.end();
         ++name)
      {
      vtkDataArray* data = model->GetCellData()->GetArray(name->c_str());
      if (data->GetNumberOfComponents() > 1)
        { second_dims.insert(data->GetNumberOfComponents()); }
      }
    // also check last dimensions of gauss point values.
      { // scope
      vtkDataArrayCollection* gaussPointData = model->GetGaussPointData();
      vtkSmartPointer<vtkCollectionIterator> iterator = 
          vtkSmartPointer<vtkCollectionIterator>::Take(gaussPointData->NewIterator());
      iterator->GoToFirstItem();
      while (iterator->IsDoneWithTraversal() == false)
        {
        vtkFloatArray* data = vtkFloatArray::SafeDownCast(iterator->GetCurrentObject());
        if (data != NULL && data->GetNumberOfComponents() > 1)
          {
          second_dims.insert (data->GetNumberOfComponents());
          }
        iterator->GoToNextItem();
        }
      } // scope

    // Create dimensions as required
    int nels_dimid;
    NC_SAFE_CALL (nc_def_dim (elementValues_ncid, "NumberOfElements", model->GetNumberOfCells(), &nels_dimid));
    std::map<int,int> dimids_map;
    for (std::set<int>::const_iterator it=second_dims.begin();
         it != second_dims.end();
         ++it)
      {
      std::string dimName = (boost::format("Components%d") % *it).str();
      int dimid;
      NC_SAFE_CALL (nc_def_dim (elementValues_ncid, dimName.c_str(), *it, &dimid));
      dimids_map[*it] = dimid;
      }

    // Create variables
    for (std::set<std::string>::const_iterator name = elementArrayNames.begin();
         name != elementArrayNames.end();
         ++name)
      {
      int varid;
      vtkDataArray* data = model->GetCellData()->GetArray(name->c_str());
      if (data->GetNumberOfComponents() == 1)
        {
        NC_SAFE_CALL (nc_def_var (elementValues_ncid, name->c_str(), NC_FLOAT, 1, &nels_dimid, &varid));
        NC_SAFE_CALL (SetChunking (elementValues_ncid, varid));
        }
      else
        {
        int dimids[2] = {nels_dimid, dimids_map[data->GetNumberOfComponents()]};
        NC_SAFE_CALL (nc_def_var (elementValues_ncid, name->c_str(), NC_FLOAT, 2, dimids, &varid));
        NC_SAFE_CALL (SetChunking (elementValues_ncid, varid));
        }
      }

    // Create subgroups for Gauss Values
    for (std::set<size_t>::const_iterator n = nGaussPoints.begin();
         n != nGaussPoints.end();
         ++n)
      {
      int gaussValues_ncid;
      std::string groupName ((boost::format("GaussPoint%dValues") % *n).str());
      NC_SAFE_CALL (nc_def_grp (elementValues_ncid, groupName.c_str(), &gaussValues_ncid));
      int gaussPoints_dimid;
      NC_SAFE_CALL (nc_def_dim (gaussValues_ncid, "NumberOfGaussPoints", *n, &gaussPoints_dimid));

      // Create variables for Gauss values
      vtkDataArrayCollection* gaussPointData = model->GetGaussPointData();
      vtkSmartPointer<vtkCollectionIterator> iterator = 
          vtkSmartPointer<vtkCollectionIterator>::Take(gaussPointData->NewIterator());
      iterator->GoToFirstItem();
      while (iterator->IsDoneWithTraversal() == false)
        {
        vtkFloatArray* data = vtkFloatArray::SafeDownCast(iterator->GetCurrentObject());
        if (data != NULL)
          {
          size_t numberOfValuesPerGaussPoint = data->GetNumberOfComponents();
          size_t nTuples = data->GetNumberOfTuples();
          size_t nElements = model->GetNumberOfCells();
          size_t numberOfGaussPoints = nTuples / nElements;
          // On this iteration skip any that don't have correct nip for this group.
          if (numberOfGaussPoints == *n)
            {
            const char* name = data->GetName();
            int varid;
            if (data->GetNumberOfComponents() == 1)
              {
              int dimids[2] = {nels_dimid, gaussPoints_dimid};
              NC_SAFE_CALL (nc_def_var (gaussValues_ncid, name, NC_FLOAT, 2, dimids, &varid));
              NC_SAFE_CALL (SetChunking (gaussValues_ncid, varid));
              }
            else
              {
              int dimids[3] = {nels_dimid, gaussPoints_dimid, dimids_map[numberOfValuesPerGaussPoint]};
              NC_SAFE_CALL (nc_def_var (gaussValues_ncid, name, NC_FLOAT, 3, dimids, &varid));
              NC_SAFE_CALL (SetChunking (gaussValues_ncid, varid));
              }
            }
          }
        iterator->GoToNextItem();
        }
      }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteMaterialDefinitions
  (
  int ncid,
  vtkboneFiniteElementModel *model
  )
  {
  vtkboneMaterialTable* materialTable = model->GetMaterialTable();
  if (!materialTable || materialTable->GetNumberOfMaterials() == 0)
    {
    // Ignore if no MaterialTable
    return VTK_OK;
    }

  int materialDefinitions_ncid;
  NC_SAFE_CALL (nc_inq_ncid(ncid, "MaterialDefinitions", &materialDefinitions_ncid));

  materialTable->InitTraversal();
  
  while (int index = materialTable->GetNextUniqueIndex())
    {
    vtkboneMaterial* material = materialTable->GetCurrentMaterial();
    int material_ncid;
    NC_SAFE_CALL (nc_inq_ncid(materialDefinitions_ncid, material->GetName(), &material_ncid));
    // only for MaterialArray derived types is there any variable data to write.
    if (vtkboneLinearIsotropicMaterialArray* isomatarray =
        vtkboneLinearIsotropicMaterialArray::SafeDownCast(material))
      {
      int E_varid;
      NC_SAFE_CALL (nc_inq_varid (material_ncid, "E", &E_varid));
      if (this->WriteVTKDataArrayToNetCDF (material_ncid, E_varid, isomatarray->GetYoungsModulus()) == VTK_ERROR)
        { return VTK_ERROR; }
      int nu_varid;
      NC_SAFE_CALL (nc_inq_varid (material_ncid, "nu", &nu_varid));
      if (this->WriteVTKDataArrayToNetCDF (material_ncid, nu_varid, isomatarray->GetPoissonsRatio()) == VTK_ERROR)
      continue;
      }
    if (vtkboneLinearOrthotropicMaterialArray* orthomatarray =
        vtkboneLinearOrthotropicMaterialArray::SafeDownCast(material))
      {
      int E_varid;
      NC_SAFE_CALL (nc_inq_varid (material_ncid, "E", &E_varid));
      if (this->WriteVTKDataArrayToNetCDF (material_ncid, E_varid, orthomatarray->GetYoungsModulus()) == VTK_ERROR)
        { return VTK_ERROR; }
      int nu_varid;
      NC_SAFE_CALL (nc_inq_varid (material_ncid, "nu", &nu_varid));
      if (this->WriteVTKDataArrayToNetCDF (material_ncid, nu_varid, orthomatarray->GetPoissonsRatio()) == VTK_ERROR)
        { return VTK_ERROR; }
      int G_varid;
      NC_SAFE_CALL (nc_inq_varid (material_ncid, "G", &G_varid));
      if (this->WriteVTKDataArrayToNetCDF (material_ncid, G_varid, orthomatarray->GetShearModulus()) == VTK_ERROR)
        { return VTK_ERROR; }
      continue;
      }
    if (vtkboneLinearAnisotropicMaterialArray* anisomatarray =
        vtkboneLinearAnisotropicMaterialArray::SafeDownCast(material))
      {
      int K_varid;
      NC_SAFE_CALL (nc_inq_varid (material_ncid, "StressStrainMatrix", &K_varid));
      if (this->WriteVTKDataArrayToNetCDF (material_ncid, K_varid, anisomatarray->GetStressStrainMatrixUpperTriangular()) == VTK_ERROR)
        { return VTK_ERROR; }
      continue;
      }
    }

  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteNodes
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  int parts_ncid;
  NC_SAFE_CALL (nc_inq_ncid (ncid, "Parts", &parts_ncid));
  int part1_ncid;
  NC_SAFE_CALL (nc_inq_ncid (parts_ncid, "Part1", &part1_ncid));
  int nodeCoordinates_varid;
  NC_SAFE_CALL (nc_inq_varid (part1_ncid, "NodeCoordinates", &nodeCoordinates_varid));
  vtkPoints* points = model->GetPoints();
  vtkDataArray* data = points->GetData();
  if (this->WriteVTKDataArrayToNetCDF (part1_ncid, nodeCoordinates_varid, data) == VTK_ERROR)
    { return VTK_ERROR; }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteMaterialTable
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  vtkboneMaterialTable* materialTable = model->GetMaterialTable();
  if (!materialTable || materialTable->GetNumberOfMaterials() == 0)
    {
    // Ignore if no materials.
    return VTK_OK;
    }
  int parts_ncid;
  NC_SAFE_CALL (nc_inq_ncid (ncid, "Parts", &parts_ncid));
  int part1_ncid;
  NC_SAFE_CALL (nc_inq_ncid (parts_ncid, "Part1", &part1_ncid));
  int materialTable_ncid;
  NC_SAFE_CALL (nc_inq_ncid (part1_ncid, "MaterialTable", &materialTable_ncid));
  int id_varid;
  NC_SAFE_CALL (nc_inq_varid (materialTable_ncid, "ID", &id_varid));
  int material_varid;
  NC_SAFE_CALL (nc_inq_varid (materialTable_ncid, "MaterialName", &material_varid));

  materialTable->InitTraversal();  
  size_t tableindex[1];
  tableindex[0] = 0;
  size_t count[1] = {1};
  while (int matindex = materialTable->GetNextIndex())
    {
    // The following call crashes on Linux with netCDF 4.2.  No idea why.
    // The nc_put_vara variation seems to be OK though.
//     NC_SAFE_CALL (nc_put_var1_int (materialTable_ncid, id_varid, tableindex, &matindex));
    NC_SAFE_CALL (nc_put_vara_int (materialTable_ncid, id_varid, tableindex, count, &matindex));
    const char* matname = materialTable->GetCurrentMaterial()->GetName();
    NC_SAFE_CALL (nc_put_var1_string (materialTable_ncid, material_varid, tableindex, &matname));
    ++(tableindex[0]);
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteElements
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  int dimensionality_dimid;
  NC_SAFE_CALL (nc_inq_dimid (ncid, "Dimensionality", &dimensionality_dimid));
  int parts_ncid;
  NC_SAFE_CALL (nc_inq_ncid (ncid, "Parts", &parts_ncid));
  int part1_ncid;
  NC_SAFE_CALL (nc_inq_ncid (parts_ncid, "Part1", &part1_ncid));
  int elements_ncid;
  NC_SAFE_CALL (nc_inq_ncid (part1_ncid, "Elements", &elements_ncid));
  int hexahedrons_ncid;
  NC_SAFE_CALL (nc_inq_ncid (elements_ncid, "Hexahedrons", &hexahedrons_ncid));

  int elementNumber_varid;
  NC_SAFE_CALL (nc_inq_varid (hexahedrons_ncid, "ElementNumber", &elementNumber_varid));
  size_t tableindex[1];
  tableindex[0] = 0;
  size_t count[2] = {1,1};
  // Note that elementid is 1-indexed
  for (vtkIdType elementid = 1; elementid <= model->GetNumberOfCells(); ++elementid)
    {
    // The following call crashes on Linux with netCDF 4.2.  No idea why.
    // The nc_put_vara variation seems to be OK though.
//     NC_SAFE_CALL (nc_put_var1_longlong (hexahedrons_ncid, elementNumber_varid, tableindex, &elementid));
    NC_SAFE_CALL (nc_put_vara_longlong (hexahedrons_ncid, elementNumber_varid, tableindex, count, &elementid));
    ++(tableindex[0]);
    }

  int nodeNumbers_varid;
  NC_SAFE_CALL (nc_inq_varid (hexahedrons_ncid, "NodeNumbers", &nodeNumbers_varid));

  vtkCellArray* cells = model->GetCells();
  cells->InitTraversal();
  vtkIdType npts = 0;
  vtkIdType* pts = NULL;
  vtkIdType pts1[8];
  size_t start[2];
  start[0] = 0;
  start[1] = 0;
  count[0] = 1;
  count[1] = 8;
  vtkIdType cellid = 0;
  vtkIdType transform[8];
  while (cells->GetNextCell(npts, pts))
    {
    // Note that for now we do this on an element-by-element basis, as
    // vtkboneFiniteElementModel can in principle be composed of mixed types.
    switch (model->GetCellType(cellid))
      {
      case VTK_VOXEL:
        transform[0] = 0;
        transform[1] = 1;
        transform[2] = 2;
        transform[3] = 3;
        transform[4] = 4;
        transform[5] = 5;
        transform[6] = 6;
        transform[7] = 7;
        break;
      case VTK_HEXAHEDRON:
        transform[0] = 0;
        transform[1] = 1;
        transform[2] = 3;
        transform[3] = 2;
        transform[4] = 4;
        transform[5] = 5;
        transform[6] = 7;
        transform[7] = 6;
        break;
      default:
        vtkErrorMacro(<<"Unsupported Element Type.");
        return VTK_ERROR;
      }
    if (npts != 8)
      {
      vtkErrorMacro(<<"Unexpected number of cell points.");
      return VTK_ERROR;
      }
    // Convert to 1-indexed
    for (int i=0; i<8; ++i)
      { pts1[i] = pts[transform[i]] + 1; }
    NC_SAFE_CALL (nc_put_vara_longlong (hexahedrons_ncid, 
                                        nodeNumbers_varid,
                                        start,
                                        count,
                                        pts1));
    ++(start[0]);
    ++cellid;
    }

  vtkDataArray* scalars = model->GetCellData()->GetScalars();
  if (scalars)
    {
    int materialid_varid;
    NC_SAFE_CALL (nc_inq_varid (hexahedrons_ncid, "MaterialID", &materialid_varid));
    if (this->WriteVTKDataArrayToNetCDF (hexahedrons_ncid, materialid_varid, scalars) == VTK_ERROR)
      { return VTK_ERROR; }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteConstraint
  (
  int constraints_ncid,
  vtkboneConstraint* constraint,
  vtkboneFiniteElementModel* model
  )
  {
  int constraint_ncid;
  NC_SAFE_CALL (nc_inq_ncid (constraints_ncid, constraint->GetName(), &constraint_ncid));
  if (constraint->GetConstraintAppliedTo() == vtkboneConstraint::NODES)
    {
    if (constraint->GetConstraintType() == vtkboneConstraint::DISPLACEMENT ||
        constraint->GetConstraintType() == vtkboneConstraint::FORCE)
      {
      int varid;
      vtkIdTypeArray* indices = constraint->GetIndices();
      NC_SAFE_CALL (nc_inq_varid (constraint_ncid, "NodeNumber", &varid));
      if (this->WriteVTKDataArrayToNetCDFOneIndexed(constraint_ncid, varid, indices) == VTK_ERROR)
        { return VTK_ERROR; }
      vtkDataArray* sense = constraint->GetAttributes()->GetArray("SENSE");
      if (sense == NULL)
        {
        vtkErrorMacro(<<"Missing constraint attribute array SENSE.");
        return VTK_ERROR;
        }
      NC_SAFE_CALL (nc_inq_varid (constraint_ncid, "Sense", &varid));
      if (this->WriteVTKDataArrayToNetCDFOneIndexed(constraint_ncid, varid, sense) == VTK_ERROR)
        { return VTK_ERROR; }
      NC_SAFE_CALL (nc_inq_varid (constraint_ncid, "Value", &varid));
      vtkDataArray* values = constraint->GetAttributes()->GetArray("VALUE");
      if (values == NULL)
        {
        vtkErrorMacro(<<"Missing constraint attribute array VALUE.");
        return VTK_ERROR;
        }
      if (this->WriteVTKDataArrayToNetCDF(constraint_ncid, varid, values) == VTK_ERROR)
        { return VTK_ERROR; }
      }
    }
  else if (constraint->GetConstraintAppliedTo() == vtkboneConstraint::ELEMENTS)
    {
    if (constraint->GetConstraintType() == vtkboneConstraint::FORCE)
      {
      vtkSmartPointer<vtkboneConstraint> forceConstraints =
          vtkSmartPointer<vtkboneConstraint>::Take(
            vtkboneConstraintUtilities::DistributeConstraintToNodes(model, constraint));
      vtkIdTypeArray* ids = forceConstraints->GetIndices();
      vtkDataArray* senses = forceConstraints->GetAttributes()->GetArray("SENSE");
      vtkDataArray* values = forceConstraints->GetAttributes()->GetArray("VALUE");
      int varid;
      NC_SAFE_CALL (nc_inq_varid (constraint_ncid, "NodeNumber", &varid));
      if (this->WriteVTKDataArrayToNetCDFOneIndexed(constraint_ncid, varid, ids) == VTK_ERROR)
        { return VTK_ERROR; }
      NC_SAFE_CALL (nc_inq_varid (constraint_ncid, "Sense", &varid));
      if (this->WriteVTKDataArrayToNetCDFOneIndexed(constraint_ncid, varid, senses) == VTK_ERROR)
        { return VTK_ERROR; }
      NC_SAFE_CALL (nc_inq_varid (constraint_ncid, "Value", &varid));
      if (this->WriteVTKDataArrayToNetCDF(constraint_ncid, varid, values) == VTK_ERROR)
        { return VTK_ERROR; }
      }
    }
  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteConstraints
  (
  int ncid,
  vtkboneFiniteElementModel* model
  )
  {
  vtkboneConstraintCollection* constraints = model->GetConstraints();
  vtkboneConstraint* convergence_set = model->GetConvergenceSet();
  if ((!constraints  || constraints->GetNumberOfItems() == 0) &&
      (convergence_set == NULL))
    {
    // Ignore if no constraints.
    return VTK_OK;
    }
  int constraints_ncid;
  NC_SAFE_CALL (nc_inq_ncid (ncid, "Constraints", &constraints_ncid));

  if (constraints)
    {  
    constraints->InitTraversal();
    while (vtkboneConstraint* constraint = constraints->GetNextItem())
      {
      int return_val = this->WriteConstraint(constraints_ncid,constraint,model);
      if (return_val != VTK_OK)
        {
        return return_val;
        }
      }
    }

  if (convergence_set)
    {
    int return_val = this->WriteConstraint (constraints_ncid, convergence_set, model);
    if (return_val != VTK_OK)
      {
      return return_val;
      }
    }

  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteSets
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  if (model->GetNodeSets()->GetNumberOfItems() +
      model->GetElementSets()->GetNumberOfItems() == 0)
    {
    return VTK_OK;
    }
  int sets_ncid;
  NC_SAFE_CALL (nc_inq_ncid (ncid, "Sets", &sets_ncid));

  // Node sets
  if (model->GetNodeSets()->GetNumberOfItems() > 0)
    {
    int nodesets_ncid;
    NC_SAFE_CALL (nc_inq_ncid (sets_ncid, "NodeSets", &nodesets_ncid));
    for (int n=0; n<model->GetNodeSets()->GetNumberOfItems(); n++)
      {
      // No error checking required on next 2 calls; did that already in DefineSets.
      vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast (model->GetNodeSets()->GetItem(n));
      const char* setName = ids->GetName();
      int set_ncid;
      NC_SAFE_CALL (nc_inq_ncid (nodesets_ncid, setName, &set_ncid));
      int varid;
      NC_SAFE_CALL (nc_inq_varid (set_ncid, "NodeNumber", &varid));
      if (this->WriteVTKDataArrayToNetCDFOneIndexed(set_ncid, varid, ids) == VTK_ERROR)
        { return VTK_ERROR; }
      }
    }

  // Element sets
  if (model->GetElementSets()->GetNumberOfItems() > 0)
    {
    int elementsets_ncid;
    NC_SAFE_CALL (nc_inq_ncid (sets_ncid, "ElementSets", &elementsets_ncid));
    for (int n=0; n<model->GetElementSets()->GetNumberOfItems(); n++)
      {
      // No error checking required on next 2 calls; did that already in DefineSets.
      vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast (model->GetElementSets()->GetItem(n));
      const char* setName = ids->GetName();
      int set_ncid;
      NC_SAFE_CALL (nc_inq_ncid (elementsets_ncid, setName, &set_ncid));
      int varid;
      NC_SAFE_CALL (nc_inq_varid (set_ncid, "ElementNumber", &varid));
      if (this->WriteVTKDataArrayToNetCDFOneIndexed(set_ncid, varid, ids) == VTK_ERROR)
        { return VTK_ERROR; }
      }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteSolution
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  // First we will just count how many node and element arrays there are.
  std::set<std::string> nodeArrayNames;
  if (this->GetSolutionArrayNames(model->GetPointData(), nodeArrayNames) != VTK_OK)
    { return VTK_ERROR; }
  std::set<std::string> elementArrayNames;
  if (this->GetSolutionArrayNames(model->GetCellData(), elementArrayNames) != VTK_OK)
    { return VTK_ERROR; }

  if (nodeArrayNames.size() == 0 && elementArrayNames.size() == 0)
    { return VTK_OK; }
      
  int solutions_ncid;
  NC_SAFE_CALL (nc_inq_ncid (ncid, "Solutions", &solutions_ncid));
  int solution1_ncid;
  NC_SAFE_CALL (nc_inq_ncid (solutions_ncid, "Solution1", &solution1_ncid));
  
  if (nodeArrayNames.size())
    {
    int nodeValues_ncid;
    NC_SAFE_CALL (nc_inq_ncid (solution1_ncid, "NodeValues", &nodeValues_ncid));
    for (std::set<std::string>::const_iterator name = nodeArrayNames.begin();
         name != nodeArrayNames.end();
         ++name)
      {
      vtkDataArray* data = model->GetPointData()->GetArray(name->c_str());
      int varid;
      NC_SAFE_CALL (nc_inq_varid (nodeValues_ncid, name->c_str(), &varid));
      if (this->WriteVTKDataArrayToNetCDF (nodeValues_ncid, varid, data) == VTK_ERROR)
        { return VTK_ERROR; }
      }
    }

  if (elementArrayNames.size())
    {
    int elementValues_ncid;
    NC_SAFE_CALL (nc_inq_ncid (solution1_ncid, "ElementValues", &elementValues_ncid));
    for (std::set<std::string>::const_iterator name = elementArrayNames.begin();
         name != elementArrayNames.end();
         ++name)
      {
      vtkDataArray* data = model->GetCellData()->GetArray(name->c_str());
      int varid;
      NC_SAFE_CALL (nc_inq_varid (elementValues_ncid, name->c_str(), &varid));
      if (this->WriteVTKDataArrayToNetCDF (elementValues_ncid, varid, data) == VTK_ERROR)
        { return VTK_ERROR; }
      }
    }

  vtkDataArrayCollection* gaussPointData = model->GetGaussPointData();
  vtkSmartPointer<vtkCollectionIterator> iterator = 
      vtkSmartPointer<vtkCollectionIterator>::Take(gaussPointData->NewIterator());
  iterator->GoToFirstItem();
  while (iterator->IsDoneWithTraversal() == false)
    {
    vtkFloatArray* data = vtkFloatArray::SafeDownCast(iterator->GetCurrentObject());
    if (data != NULL)
      {
      size_t nTuples = data->GetNumberOfTuples();
      size_t nElements = model->GetNumberOfCells();
      size_t numberOfGaussPoints = nTuples / nElements;
      int elementValues_ncid;
      NC_SAFE_CALL (nc_inq_ncid (solution1_ncid, "ElementValues", &elementValues_ncid));
      std::string groupName ((boost::format("GaussPoint%dValues") % numberOfGaussPoints).str());
      int gaussValues_ncid;
      NC_SAFE_CALL (nc_inq_ncid (elementValues_ncid, groupName.c_str(), &gaussValues_ncid));
      const char* name = data->GetName();
      int varid;
      NC_SAFE_CALL (nc_inq_varid (gaussValues_ncid, name, &varid));
      if (this->WriteVTKDataArrayToNetCDF (gaussValues_ncid, varid, data, numberOfGaussPoints) == VTK_ERROR)
        { return VTK_ERROR; }
      }
    iterator->GoToNextItem();
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteVTKDataArrayToNetCDF
(
  int ncid,
  int varid,
  vtkDataArray* data
)
{
  // nc_get_vara calls crashes on Linux with netCDF 4.2.  No idea why.
  // The nc_get_vara variation seems to be OK though.
  size_t start[2] = {0,0};
  size_t count[2];
  count[0] = data->GetNumberOfTuples();
  count[1] = data->GetNumberOfComponents();
  switch (data->GetDataType())
    {
    case VTK_FLOAT:
      NC_SAFE_CALL (nc_put_vara_float (ncid, varid, start, count, vtkFloatArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_DOUBLE:
      NC_SAFE_CALL (nc_put_vara_double (ncid, varid, start, count, vtkDoubleArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_CHAR:
      NC_SAFE_CALL (nc_put_vara_schar (ncid, varid, start, count, (const signed char*)vtkCharArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_SIGNED_CHAR:
      NC_SAFE_CALL (nc_put_vara_schar (ncid, varid, start, count, vtkSignedCharArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_UNSIGNED_CHAR:
      NC_SAFE_CALL (nc_put_vara_uchar (ncid, varid, start, count, vtkUnsignedCharArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_SHORT:
      NC_SAFE_CALL (nc_put_vara_short (ncid, varid, start, count, vtkShortArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_UNSIGNED_SHORT:
      NC_SAFE_CALL (nc_put_vara_ushort (ncid, varid, start, count, vtkUnsignedShortArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_INT:
      NC_SAFE_CALL (nc_put_vara_int (ncid, varid, start, count, vtkIntArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_UNSIGNED_INT:
      NC_SAFE_CALL (nc_put_vara_uint (ncid, varid, start, count, vtkUnsignedIntArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_ID_TYPE:
    case VTK_LONG_LONG:
      NC_SAFE_CALL (nc_put_vara_longlong (ncid, varid, start, count, vtkLongLongArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_UNSIGNED_LONG_LONG:
      NC_SAFE_CALL (nc_put_vara_ulonglong (ncid, varid, start, count, vtkUnsignedLongLongArray::SafeDownCast(data)->GetPointer(0)));
      break;
    default:
      vtkErrorMacro(<< "Unsupported type for constraint value data.");
      return VTK_ERROR;
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteVTKDataArrayToNetCDF
(
  int ncid,
  int varid,
  vtkDataArray* data,
  size_t dim1
)
{
  n88_assert (data->GetNumberOfTuples() % dim1 == 0);
  // nc_get_vara calls crashes on Linux with netCDF 4.2.  No idea why.
  // The nc_get_vara variation seems to be OK though.
  size_t start[3] = {0,0,0};
  size_t count[3];
  count[0] = data->GetNumberOfTuples() / dim1;
  count[1] = dim1;
  count[2] = data->GetNumberOfComponents();
  switch (data->GetDataType())
    {
    case VTK_FLOAT:
      NC_SAFE_CALL (nc_put_vara_float (ncid, varid, start, count, vtkFloatArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_DOUBLE:
      NC_SAFE_CALL (nc_put_vara_double (ncid, varid, start, count, vtkDoubleArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_CHAR:
      NC_SAFE_CALL (nc_put_vara_schar (ncid, varid, start, count, (const signed char*)vtkCharArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_SIGNED_CHAR:
      NC_SAFE_CALL (nc_put_vara_schar (ncid, varid, start, count, vtkSignedCharArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_UNSIGNED_CHAR:
      NC_SAFE_CALL (nc_put_vara_uchar (ncid, varid, start, count, vtkUnsignedCharArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_SHORT:
      NC_SAFE_CALL (nc_put_vara_short (ncid, varid, start, count, vtkShortArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_UNSIGNED_SHORT:
      NC_SAFE_CALL (nc_put_vara_ushort (ncid, varid, start, count, vtkUnsignedShortArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_INT:
      NC_SAFE_CALL (nc_put_vara_int (ncid, varid, start, count, vtkIntArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_UNSIGNED_INT:
      NC_SAFE_CALL (nc_put_vara_uint (ncid, varid, start, count, vtkUnsignedIntArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_ID_TYPE:
    case VTK_LONG_LONG:
      NC_SAFE_CALL (nc_put_vara_longlong (ncid, varid, start, count, vtkLongLongArray::SafeDownCast(data)->GetPointer(0)));
      break;
    case VTK_UNSIGNED_LONG_LONG:
      NC_SAFE_CALL (nc_put_vara_ulonglong (ncid, varid, start, count, vtkUnsignedLongLongArray::SafeDownCast(data)->GetPointer(0)));
      break;
    default:
      vtkErrorMacro(<< "Unsupported type for constraint value data.");
      return VTK_ERROR;
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelWriter::WriteVTKDataArrayToNetCDFOneIndexed
(
  int ncid,
  int varid,
  vtkDataArray* data
)
{
  // nc_get_vara calls crashes on Linux with netCDF 4.2.  No idea why.
  // The nc_get_vara variation seems to be OK though.
  size_t start[2] = {0,0};
  size_t count[2];
  count[0] = data->GetNumberOfTuples();
  count[1] = data->GetNumberOfComponents();
  switch (data->GetDataType())
    {
    case VTK_CHAR:
      {
      vtkCharArray* char_data = vtkCharArray::SafeDownCast(data);
      vtkSmartPointer<vtkCharArray> data1 = vtkSmartPointer<vtkCharArray>::New();
      vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
      data1->SetNumberOfValues (n);
      for (vtkIdType i=0; i<n; ++i)
        { data1->SetValue(i, char_data->GetValue(i) + 1); }
      NC_SAFE_CALL (nc_put_vara_schar (ncid, varid, start, count, (signed char*)data1->GetPointer(0)));
      break;
      }
    case VTK_SIGNED_CHAR:
      {
      vtkSignedCharArray* char_data = vtkSignedCharArray::SafeDownCast(data);
      vtkSmartPointer<vtkSignedCharArray> data1 = vtkSmartPointer<vtkSignedCharArray>::New();
      vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
      data1->SetNumberOfValues (n);
      for (vtkIdType i=0; i<n; ++i)
        { data1->SetValue(i, char_data->GetValue(i) + 1); }
      NC_SAFE_CALL (nc_put_vara_schar (ncid, varid, start, count, data1->GetPointer(0)));
      break;
      }
    case VTK_UNSIGNED_CHAR:
      {
      vtkUnsignedCharArray* uchar_data = vtkUnsignedCharArray::SafeDownCast(data);
      vtkSmartPointer<vtkUnsignedCharArray> data1 = vtkSmartPointer<vtkUnsignedCharArray>::New();
      vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
      data1->SetNumberOfValues (n);
      for (vtkIdType i=0; i<n; ++i)
        { data1->SetValue(i, uchar_data->GetValue(i) + 1); }
      NC_SAFE_CALL (nc_put_vara_uchar (ncid, varid, start, count, data1->GetPointer(0)));
      break;
      }
    case VTK_SHORT:
      {
      vtkShortArray* short_data = vtkShortArray::SafeDownCast(data);
      vtkSmartPointer<vtkShortArray> data1 = vtkSmartPointer<vtkShortArray>::New();
      vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
      data1->SetNumberOfValues (n);
      for (vtkIdType i=0; i<n; ++i)
        { data1->SetValue(i, short_data->GetValue(i) + 1); }
      NC_SAFE_CALL (nc_put_vara_short (ncid, varid, start, count, data1->GetPointer(0)));
      break;
      }
    case VTK_UNSIGNED_SHORT:
      {
      vtkUnsignedShortArray* ushort_data = vtkUnsignedShortArray::SafeDownCast(data);
      vtkSmartPointer<vtkUnsignedShortArray> data1 = vtkSmartPointer<vtkUnsignedShortArray>::New();
      vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
      data1->SetNumberOfValues (n);
      for (vtkIdType i=0; i<n; ++i)
        { data1->SetValue(i, ushort_data->GetValue(i) + 1); }
      NC_SAFE_CALL (nc_put_vara_ushort (ncid, varid, start, count, data1->GetPointer(0)));
      break;
      }
    case VTK_INT:
      {
      vtkIntArray* int_data = vtkIntArray::SafeDownCast(data);
      vtkSmartPointer<vtkIntArray> data1 = vtkSmartPointer<vtkIntArray>::New();
      vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
      data1->SetNumberOfValues (n);
      for (vtkIdType i=0; i<n; ++i)
        { data1->SetValue(i, int_data->GetValue(i) + 1); }
      NC_SAFE_CALL (nc_put_vara_int (ncid, varid, start, count, data1->GetPointer(0)));
      break;
      }
    case VTK_UNSIGNED_INT:
      {
      vtkUnsignedIntArray* uint_data = vtkUnsignedIntArray::SafeDownCast(data);
      vtkSmartPointer<vtkUnsignedIntArray> data1 = vtkSmartPointer<vtkUnsignedIntArray>::New();
      vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
      data1->SetNumberOfValues (n);
      for (vtkIdType i=0; i<n; ++i)
        { data1->SetValue(i, uint_data->GetValue(i) + 1); }
      NC_SAFE_CALL (nc_put_vara_uint (ncid, varid, start, count, data1->GetPointer(0)));
      break;
      }
    case VTK_LONG:
      {
      vtkLongArray* int_data = vtkLongArray::SafeDownCast(data);
      vtkSmartPointer<vtkLongArray> data1 = vtkSmartPointer<vtkLongArray>::New();
      vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
      data1->SetNumberOfValues (n);
      for (vtkIdType i=0; i<n; ++i)
        { data1->SetValue(i, int_data->GetValue(i) + 1); }
      NC_SAFE_CALL (nc_put_vara_long (ncid, varid, start, count, data1->GetPointer(0)));
      break;
      }
    // case VTK_UNSIGNED_LONG:
    //   {
    //   vtkUnsignedLongArray* uint_data = vtkUnsignedLongArray::SafeDownCast(data);
    //   vtkSmartPointer<vtkUnsignedLongArray> data1 = vtkSmartPointer<vtkUnsignedLongArray>::New();
    //   vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
    //   data1->SetNumberOfValues (n);
    //   for (vtkIdType i=0; i<n; ++i)
    //     { data1->SetValue(i, uint_data->GetValue(i) + 1); }
    //   NC_SAFE_CALL (nc_put_vara_ulonglong (ncid, varid, start, count, data1->GetPointer(0)));
    //   break;
    //   }
    case VTK_ID_TYPE:
      {
      vtkIdTypeArray* longlong_data = vtkIdTypeArray::SafeDownCast(data);
      vtkSmartPointer<vtkIdTypeArray> data1 = vtkSmartPointer<vtkIdTypeArray>::New();
      vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
      data1->SetNumberOfValues (n);
      for (vtkIdType i=0; i<n; ++i)
        { data1->SetValue(i, longlong_data->GetValue(i) + 1); }
      NC_SAFE_CALL (nc_put_vara_longlong (ncid, varid, start, count, data1->GetPointer(0)));
      break;
      }
    case VTK_LONG_LONG:
      {
      vtkLongLongArray* longlong_data = vtkLongLongArray::SafeDownCast(data);
      vtkSmartPointer<vtkLongLongArray> data1 = vtkSmartPointer<vtkLongLongArray>::New();
      vtkIdType n = data->GetNumberOfTuples() * data->GetNumberOfComponents();
      data1->SetNumberOfValues (n);
      for (vtkIdType i=0; i<n; ++i)
        { data1->SetValue(i, longlong_data->GetValue(i) + 1); }
      NC_SAFE_CALL (nc_put_vara_longlong (ncid, varid, start, count, data1->GetPointer(0)));
      break;
      }
    default:
      vtkErrorMacro(<< "Unsupported type for index data.");
      return VTK_ERROR;
    }

  return VTK_OK;
}

int vtkboneN88ModelWriter::SetChunking (int ncid, int varid)
{
  if (this->Compression)
    {
    NC_SAFE_CALL (nc_def_var_deflate(ncid, varid, 0, 1, deflate_level));
    }

  int return_val = NC_NOERR;
  nc_type xtype;
  return_val = nc_inq_vartype (ncid, varid, &xtype);
  if (return_val != NC_NOERR) { return return_val; }
  size_t varsize = 1;
  switch (xtype)
    {
    case NC_BYTE:
    case NC_CHAR:
    case NC_UBYTE:
      varsize = 1;
      break;
    case NC_SHORT:
    case NC_USHORT:
      varsize = 2;
      break;
    case NC_INT:
    case NC_FLOAT:
    case NC_UINT:
      varsize = 4;
      break;
    case NC_DOUBLE:
    case NC_INT64:
    case NC_UINT64:
      varsize = 8;
      break;
    default:
      return -999;          
    }
  int ndims = 0;
  return_val = nc_inq_varndims (ncid, varid, &ndims);
  if (return_val != NC_NOERR) { return return_val; }
  if (ndims < 1 || ndims > 3)
    { return -998; }
  int dimids[3];
  return_val = nc_inq_vardimid (ncid, varid, dimids);
  if (return_val != NC_NOERR) { return return_val; }
  size_t dims[3];
  dims[1] = 1;
  dims[2] = 1;
  return_val = nc_inq_dimlen (ncid, dimids[0], &(dims[0]));
  if (return_val != NC_NOERR) { return return_val; }
  if (ndims >= 2)
    {
    return_val = nc_inq_dimlen (ncid, dimids[1], &(dims[1]));
    if (return_val != NC_NOERR) { return return_val; }
    }
  if (ndims >= 3)
    {
    return_val = nc_inq_dimlen (ncid, dimids[2], &(dims[2]));
    if (return_val != NC_NOERR) { return return_val; }
    }
  size_t chunksizes[3];
  chunksizes[1] = dims[1];
  chunksizes[2] = dims[2];
  chunksizes[0] = CHUNK_SIZE / (varsize * dims[1] * dims[2]);
  if (chunksizes[0] > dims[0])
    {
    chunksizes[0] = dims[0];
    }
  return_val = nc_def_var_chunking (ncid, varid, NC_CHUNKED, chunksizes);
  return return_val;
}
