#include "vtkboneN88ModelReader.h"
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
#include "vtkboneSolverParameters.h"
#include "vtkObjectFactory.h"
#include "vtkIntArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkCharArray.h"
#include "vtkCellArray.h"
#include "vtkDataArrayCollection.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationStringVectorKey.h"
#include "n88util/text.hpp"
#include "n88util/exception.hpp"
#ifdef VTKBONE_USE_VTKNETCDF
#include "vtk_netcdf.h"
#else
#include "netcdf.h"
#endif
#include <map>
#include <string>

// This can only be used in a function that can return VTK_ERROR.
// NOTE: This macro should be used very sparingly in the read.
// Anytime an error might conceivably be caused by a malformed file, a more
// informative error message should be returned.
#define NC_SAFE_CALL(x) \
{ \
  int sc_status = (x); \
  if (sc_status != NC_NOERR) \
    { \
    vtkErrorMacro(<< "NetCDF error " <<  nc_strerror(sc_status) << "."); \
    return VTK_ERROR; \
    } \
}


vtkStandardNewMacro(vtkboneN88ModelReader);

//-----------------------------------------------------------------------
vtkboneN88ModelReader::vtkboneN88ModelReader()
{
  this->FileName = NULL;
  this->ReadMaterials = 1;
  this->SetNumberOfInputPorts(0);
  this->ActiveSolution = NULL;
  this->ActiveProblem = NULL;
  this->ActivePart = NULL;
}

//-----------------------------------------------------------------------
vtkboneN88ModelReader::~vtkboneN88ModelReader()
{
  this->SetFileName(NULL);
  this->SetActiveSolution(NULL);
  this->SetActiveProblem(NULL);
  this->SetActivePart(NULL);
}

//----------------------------------------------------------------------------
void vtkboneN88ModelReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::RequestData(
                               vtkInformation *,
                               vtkInformationVector **,
                               vtkInformationVector *outputVector)
{

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkboneFiniteElementModel *output = vtkboneFiniteElementModel::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
  {
    vtkErrorMacro("No output object.");
    return VTK_ERROR;
  }

  // Make sure we have a file to read.
  if (!this->FileName)
  {
    vtkErrorMacro("FileName not set.");
    return VTK_ERROR;
  }

  int status;
  int ncid;

  // Open input file.
  status = nc_open (this->FileName, NC_NOWRITE, &ncid);
  if (status != NC_NOERR)
    {
    vtkErrorMacro(<< "Unable to open file " << this->FileName
                  << " ; NetCDF error " <<  nc_strerror(status) << ".");
    return VTK_ERROR;
    }

  this->SetActiveSolution(NULL);
  this->SetActiveProblem(NULL);
  this->SetActivePart(NULL);

  status = this->ReadDataFromNetCDFFile (ncid, output);

  nc_close (ncid);

  return status;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadDataFromNetCDFFile
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  if (this->ReadAttributes(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->ReadProblem(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->ReadNodes(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->ReadMaterials != 0)
    {
    if (this->ReadMaterialTable(ncid, model) == VTK_ERROR) return VTK_ERROR;
    }
  if (this->ReadElements(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->ReadConstraints(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->ReadConvergenceSet(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->ReadSets(ncid, model) == VTK_ERROR) return VTK_ERROR;
  if (this->ReadSolutions(ncid, model) == VTK_ERROR) return VTK_ERROR;
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadAttributes
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  size_t att_len = 0;
  if (nc_inq_attlen(ncid, NC_GLOBAL, "Conventions", &att_len) != NC_NOERR)
    {
    vtkErrorMacro(<< "No Conventions attribute in NetCDF file.");
    return VTK_ERROR;
    }
  std::string buffer;
  buffer.resize(att_len);
  if (nc_get_att_text(ncid, NC_GLOBAL, "Conventions", &buffer[0]) != NC_NOERR)
    {
    vtkErrorMacro(<< "Error reading Conventions attribute in NetCDF file.");
    return VTK_ERROR;
    }
  if (buffer != "Numerics88/Finite_Element_Model-1.0")
    {
    vtkErrorMacro(<< "File not identified as Numerics88/Finite_Element_Model-1.0.");
    return VTK_ERROR;
    }

  if (nc_inq_attlen(ncid, NC_GLOBAL, "History", &att_len) == NC_NOERR)
    {
    buffer.resize(att_len);
    if (nc_get_att_text (ncid, NC_GLOBAL, "History", &buffer[0]) != NC_NOERR)
      {
      vtkErrorMacro(<< "Error reading History attribute in NetCDF file.");
      return VTK_ERROR;
      }
    model->SetHistory(buffer.c_str());
    }

  // Backwards support for older Comments field
  model->SetLog(NULL);
  if (nc_inq_attlen(ncid, NC_GLOBAL, "Comments", &att_len) == NC_NOERR)
    {
    buffer.resize(att_len);
    if (nc_get_att_text (ncid, NC_GLOBAL, "Comments", &buffer[0]) != NC_NOERR)
      {
      vtkErrorMacro(<< "Error reading Comments attribute in NetCDF file.");
      return VTK_ERROR;
      }
    model->SetLog(buffer.c_str());
    }
  if (nc_inq_attlen(ncid, NC_GLOBAL, "Log", &att_len) == NC_NOERR)
    {
    buffer.resize(att_len);
    if (nc_get_att_text (ncid, NC_GLOBAL, "Log", &buffer[0]) != NC_NOERR)
      {
      vtkErrorMacro(<< "Error reading Log attribute in NetCDF file.");
      return VTK_ERROR;
      }
    std::string completeLog;
    if (model->GetLog() != NULL)
      { completeLog = model->GetLog(); }
    size_t len = completeLog.size();
    if (len > 0 && completeLog[len - 1] != '\n')
        {
        completeLog += "\n";
        ++len;
        }
    if (len > 1 && completeLog[len - 2] != '\n')
        {
        completeLog += "\n";
        ++len;
        }
    completeLog += buffer;
    model->SetLog (completeLog.c_str());
    }

  if (nc_inq_attlen  (ncid, NC_GLOBAL, "ActiveSolution", &att_len) == NC_NOERR)
    {
    buffer.resize(att_len);
    if (nc_get_att_text (ncid, NC_GLOBAL, "ActiveSolution", &buffer[0]) != NC_NOERR)
      {
      vtkErrorMacro(<< "Error reading ActiveSolution attribute in NetCDF file.");
      return VTK_ERROR;
      }
    this->SetActiveSolution(buffer.c_str());
    // Get ActiveProblem from Solution
    int solutions_ncid;
    if (nc_inq_ncid(ncid, "Solutions", &solutions_ncid) != NC_NOERR)
      {
      vtkErrorMacro(<< "Solutions group not found.");
      return VTK_ERROR;
      }
    int activeSolution_ncid;
    if (nc_inq_ncid(solutions_ncid, this->ActiveSolution, &activeSolution_ncid) != NC_NOERR)
      {
      vtkErrorMacro(<< "ActiveSolution group not found " << this->ActiveSolution << ".");
      return VTK_ERROR;
      }
    if (nc_inq_attlen  (activeSolution_ncid, NC_GLOBAL, "Problem", &att_len) != NC_NOERR)
      {
      vtkErrorMacro(<< "Problem attribute not found for specified ActiveSolution.");
      return VTK_ERROR;
      }
    buffer.resize(att_len);
    if (nc_get_att_text (activeSolution_ncid, NC_GLOBAL, "Problem", &buffer[0]) != NC_NOERR)
      {
      vtkErrorMacro(<< "Error reading Problem in ActiveSolution group " << this->ActiveSolution << ".");
      return VTK_ERROR;
      }
    this->SetActiveProblem(buffer.c_str());
    }
  else
    {
    // No ActiveSolution; try ActiveProblem
    if (nc_inq_attlen  (ncid, NC_GLOBAL, "ActiveProblem", &att_len) != NC_NOERR)
      {
      vtkErrorMacro(<< "Neither ActiveSolution nor ActiveProblem attribute not found in NetCDF file.");
      return VTK_ERROR;
      }
    buffer.resize(att_len);
    if (nc_get_att_text (ncid, NC_GLOBAL, "ActiveProblem", &buffer[0]) != NC_NOERR)
      {
      vtkErrorMacro(<< "Error reading ActiveProblem attribute in NetCDF file.");
      return VTK_ERROR;
      }
    this->SetActiveProblem(buffer.c_str());
    }

  int dimid;
  if (nc_inq_dimid (ncid, "Dimensionality", &dimid))
    {
    vtkErrorMacro(<< "Dimensionality dimension not found in NetCDF file.");
    return VTK_ERROR;
    }
  size_t dimensions = 0;
  NC_SAFE_CALL( nc_inq_dimlen(ncid, dimid, &dimensions) );
  if (dimensions != 3)
    {
    vtkErrorMacro(<< "Only dimensionality of 3 currently supported.");
    return VTK_ERROR;
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadProblem
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  int problems_ncid;
  if (nc_inq_ncid(ncid, "Problems", &problems_ncid) != NC_NOERR)
    {
    vtkErrorMacro(<< "Problems group not found.");
    return VTK_ERROR;
    }

  int activeProblem_ncid;
  if (nc_inq_ncid(problems_ncid, this->ActiveProblem, &activeProblem_ncid) != NC_NOERR)
    {
    vtkErrorMacro(<< "ActiveProblem group not found " << this->ActiveProblem << ".");
    return VTK_ERROR;
    }

  size_t att_len = 0;
  std::string buffer;
  if (nc_inq_attlen  (activeProblem_ncid, NC_GLOBAL, "Part", &att_len) != NC_NOERR)
    {
    vtkErrorMacro(<< "Part attribute not found in NetCDF file.");
    return VTK_ERROR;
    }
  buffer.resize(att_len);
  if (nc_get_att_text (activeProblem_ncid, NC_GLOBAL, "Part", &buffer[0]) != NC_NOERR)
    {
    vtkErrorMacro(<< "Unable to read Part attribute in Part group " << this->ActiveProblem << ".");
    return VTK_ERROR;
    }
  this->SetActivePart(buffer.c_str());

  vtkInformation* info = model->GetInformation();
  double double_value;
  if (nc_get_att_double (activeProblem_ncid, NC_GLOBAL, "ConvergenceTolerance", &double_value) == NC_NOERR)
    {
    vtkboneSolverParameters::CONVERGENCE_TOLERANCE()->Set (info, double_value);
    }

  int int_value;
  if (nc_get_att_int (activeProblem_ncid, NC_GLOBAL, "MaximumIterations", &int_value) == NC_NOERR)
    {
    vtkboneSolverParameters::MAXIMUM_ITERATIONS()->Set (info, int_value);
    }

  if (nc_get_att_double (activeProblem_ncid, NC_GLOBAL, "PlasticConvergenceTolerance", &double_value) == NC_NOERR)
    {
    vtkboneSolverParameters::PLASTIC_CONVERGENCE_TOLERANCE()->Set (info, double_value);
    }

  if (nc_get_att_int (activeProblem_ncid, NC_GLOBAL, "MaximumPlasticIterations", &int_value) == NC_NOERR)
    {
    vtkboneSolverParameters::MAXIMUM_PLASTIC_ITERATIONS()->Set (info, int_value);
    }

  if (nc_inq_attlen  (activeProblem_ncid, NC_GLOBAL, "PostProcessingNodeSets", &att_len) == NC_NOERR)
    {
    buffer.resize(att_len);
    NC_SAFE_CALL (nc_get_att_text (activeProblem_ncid, NC_GLOBAL, "PostProcessingNodeSets", &buffer[0]));
    std::vector<std::string> tokens;
    n88util::split_arguments(std::string(buffer), tokens, ",");
    // Backwards compatible splitting on ";"
    if (tokens.size() == 1)
      { n88util::split_arguments(std::string(buffer), tokens, ";"); }
    for (size_t c=0; c<tokens.size(); ++c)
      {
      vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append (info, tokens[c].c_str());
      }
    }

  if (nc_inq_attlen  (activeProblem_ncid, NC_GLOBAL, "PostProcessingElementSets", &att_len) == NC_NOERR)
    {
    buffer.resize(att_len);
    NC_SAFE_CALL (nc_get_att_text (activeProblem_ncid, NC_GLOBAL, "PostProcessingElementSets", &buffer[0]));
    std::vector<std::string> tokens;
    n88util::split_arguments(std::string(buffer), tokens, ",");
    // Backwards compatible splitting on ";"
    if (tokens.size() == 1)
      { n88util::split_arguments(std::string(buffer), tokens, ";"); }
    for (size_t c=0; c<tokens.size(); ++c)
      {
      vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append (info, tokens[c].c_str());
      }
    }

  if (nc_inq_attlen (activeProblem_ncid, NC_GLOBAL, "RotationCenter", &att_len) == NC_NOERR)
    {
    if (att_len != 3)
      {
      vtkWarningMacro(<< "RotationCenter does not have length 3: skipping.");
      }
    else
      {
      double double_triplet[3];
      if (nc_get_att_double (activeProblem_ncid, NC_GLOBAL, "RotationCenter", double_triplet) == NC_NOERR)
        {
        vtkboneSolverParameters::ROTATION_CENTER()->Set (info, double_triplet, 3);
        }
      }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadNodes
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  int parts_ncid;
  if (nc_inq_ncid(ncid, "Parts", &parts_ncid) != NC_NOERR)
    {
    vtkErrorMacro(<< "Parts group not found.");
    return VTK_ERROR;
    }
  int activePart_ncid;
  if (nc_inq_ncid (parts_ncid, this->GetActivePart(), &activePart_ncid) != NC_NOERR)
    {
    vtkErrorMacro(<< "ActivePart group not found " << this->GetActivePart() << ".");
    return VTK_ERROR;
    }

  int varid;
  if (nc_inq_varid (activePart_ncid, "NodeCoordinates", &varid) != NC_NOERR)
    {
    vtkErrorMacro(<< "Unable to find variable NodeCoordinates for Part " << this->GetActivePart() << ".");
    return VTK_ERROR;
    }
  int ndims = 0;
  NC_SAFE_CALL (nc_inq_varndims(activePart_ncid, varid, &ndims));
  if (ndims != 2)
    {
    vtkErrorMacro(<< "NodeCoordinates must be 2-dimensional.");
    return VTK_ERROR;
    }
  int dimids[2];
  NC_SAFE_CALL (nc_inq_vardimid(activePart_ncid, varid, dimids));
  size_t len = 0;
  NC_SAFE_CALL( nc_inq_dimlen(activePart_ncid, dimids[1], &len) );
  if (len != 3)
    {
    vtkErrorMacro(<< "Second dimension of NodeCoordinates must have length 3.");
    return VTK_ERROR;
    }
  NC_SAFE_CALL( nc_inq_dimlen(activePart_ncid, dimids[0], &len) );
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataTypeToFloat();
  points->SetNumberOfPoints(len);
  // The following call crashes on Linux with netCDF 4.2.  No idea why.
  // The nc_get_vara variation seems to be OK though.
//   NC_SAFE_CALL( nc_get_var_float(activePart_ncid, varid,
//                         reinterpret_cast<float*>(points->GetVoidPointer(0))));
  size_t start[2] = {0,0};
  size_t count[2];
  count[0] = len;
  count[1] = 3;
  NC_SAFE_CALL (nc_get_vara_float (activePart_ncid, varid, start, count,
                         reinterpret_cast<float*>(points->GetVoidPointer(0))));
  model->SetPoints(points);

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadMaterialTable
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  // -------------------------------------------------------------------
  // Read the MaterialDefinitions: store in a map

  int materialDefinitions_ncid;
  if (nc_inq_ncid(ncid, "MaterialDefinitions", &materialDefinitions_ncid) != NC_NOERR)
    {
    vtkWarningMacro(<< "MaterialDefinitions group not found.");
    return VTK_OK;
    }
  typedef std::map<std::string, vtkSmartPointer<vtkboneMaterial> > matdefs_map_t;
  matdefs_map_t matdefs;
  int numentries = 0;
  NC_SAFE_CALL (nc_inq_grps(materialDefinitions_ncid, &numentries, NULL));
  std::vector<int> ncids;
  ncids.resize(numentries);
  NC_SAFE_CALL (nc_inq_grps(materialDefinitions_ncid, &numentries, &ncids.front()));
  std::vector<char> name;
  name.resize(NC_MAX_NAME+1);
  for (int i=0; i<numentries; ++i)
    {
    NC_SAFE_CALL (nc_inq_grpname (ncids[i], &name[0]));
    size_t att_len = 0;
    if (nc_inq_attlen  (ncids[i], NC_GLOBAL, "Type", &att_len) != NC_NOERR)
      {
      vtkErrorMacro(<< "Type attribute not found for material " << &name[0] << ".");
      return VTK_ERROR;
      }
    std::string type;
    type.resize(att_len);
    NC_SAFE_CALL (nc_get_att_text (ncids[i], NC_GLOBAL, "Type", &type[0]));

    if (type == "LinearIsotropic")
      {
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "E", &att_len) != NC_NOERR)
        {
        // No attributes -> values must be in a variable.
        vtkboneLinearIsotropicMaterialArray* material = vtkboneLinearIsotropicMaterialArray::New();
        material->SetName(&name[0]);
        matdefs[std::string(&name[0])] = vtkSmartPointer<vtkboneMaterial>::Take(material);
        int varid;
        if (nc_inq_varid (ncids[i], "E", &varid) != NC_NOERR)
          {
          vtkErrorMacro (<< "LinearIsotropic material " << &name[0] << " has no 'E' attribute or variable.");
          return VTK_ERROR;
          }
        int ndims = 0;
        NC_SAFE_CALL (nc_inq_varndims(ncids[i], varid, &ndims));
        if (ndims != 1)
          {
          vtkErrorMacro (<< "LinearIsotropic material " << &name[0] << " has 'E' variable of incorrect dimension.");
          return VTK_ERROR;
          }
        int dimid;
        NC_SAFE_CALL (nc_inq_vardimid(ncids[i], varid, &dimid));
        size_t size = 0;
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimid, &size));
        vtkSmartPointer<vtkFloatArray> E = vtkSmartPointer<vtkFloatArray>::New();
        E->SetNumberOfTuples(size);
        NC_SAFE_CALL (nc_get_var_float(ncids[i], varid, (float*)(E->GetPointer(0))));
        material->SetYoungsModulus(E);
        if (nc_inq_varid (ncids[i], "nu", &varid) != NC_NOERR)
          {
          vtkErrorMacro (<< "LinearIsotropic material " << &name[0] << " has no 'nu' attribute or variable.");
          return VTK_ERROR;
          }
        ndims = 0;
        NC_SAFE_CALL (nc_inq_varndims(ncids[i], varid, &ndims));
        if (ndims != 1)
          {
          vtkErrorMacro (<< "LinearIsotropic material " << &name[0] << " has 'nu' variable of incorrect dimension.");
          return VTK_ERROR;
          }
        NC_SAFE_CALL (nc_inq_vardimid(ncids[i], varid, &dimid));
        size_t size2 = 0;
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimid, &size2));
        if (size2 != size)
          {
          vtkErrorMacro (<< "LinearIsotropic material " << &name[0] << " has inconsistent variable lengths");
          return VTK_ERROR;
          }
        vtkSmartPointer<vtkFloatArray> nu = vtkSmartPointer<vtkFloatArray>::New();
        nu->SetNumberOfTuples(size);
        NC_SAFE_CALL (nc_get_var_float(ncids[i], varid, (float*)(nu->GetPointer(0))));
        material->SetPoissonsRatio(nu);
        }
      else
        {
        // Values in attribute -> single value
        if (att_len != 1)
          {
          vtkErrorMacro(<< "LinearIsotropic material " << &name[0] << " E attribute does not have length 1.");
          return VTK_ERROR;
          }
        vtkboneLinearIsotropicMaterial* material = vtkboneLinearIsotropicMaterial::New();
        material->SetName(&name[0]);
        matdefs[std::string(&name[0])] = vtkSmartPointer<vtkboneMaterial>::Take(material);
        double real_val;
        NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "E", &real_val));
        material->SetYoungsModulus(real_val);
        if (nc_inq_attlen(ncids[i], NC_GLOBAL, "nu", &att_len) != NC_NOERR)
          {
          vtkErrorMacro(<< "LinearIsotropic material " << &name[0] << " has no 'nu' attribute.");
          return VTK_ERROR;
          }
        if (att_len != 1)
          {
          vtkErrorMacro(<< "LinearIsotropic material " << &name[0] << " 'nu' attribute does not have length 1.");
          return VTK_ERROR;
          }
        NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "nu", &real_val));
        material->SetPoissonsRatio(real_val);
        }
      }

    else if (type == "LinearOrthotropic")
      {
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "E", &att_len) != NC_NOERR)
        {
        // No attributes -> values must be in a variable.
        vtkboneLinearOrthotropicMaterialArray* material = vtkboneLinearOrthotropicMaterialArray::New();
        material->SetName(&name[0]);
        matdefs[std::string(&name[0])] = vtkSmartPointer<vtkboneMaterial>::Take(material);
        int varid;
        if (nc_inq_varid (ncids[i], "E", &varid) != NC_NOERR)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has no 'E' attribute or variable.");
          return VTK_ERROR;
          }
        int ndims = 0;
        NC_SAFE_CALL (nc_inq_varndims(ncids[i], varid, &ndims));
        if (ndims != 2)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has 'E' variable of incorrect dimension.");
          return VTK_ERROR;
          }
        int dimids[2];
        NC_SAFE_CALL (nc_inq_vardimid(ncids[i], varid, dimids));
        size_t size = 0;
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[0], &size));
        size_t components = 0;
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[1], &components));
        if (components != 3)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has 'E' variable of incorrect dimension.");
          return VTK_ERROR;
          }
        vtkSmartPointer<vtkFloatArray> E = vtkSmartPointer<vtkFloatArray>::New();
        E->SetNumberOfComponents(3);
        E->SetNumberOfTuples(size);
        NC_SAFE_CALL (nc_get_var_float(ncids[i], varid, (float*)E->GetPointer(0)));
        material->SetYoungsModulus(E);
        if (nc_inq_varid (ncids[i], "nu", &varid) != NC_NOERR)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has no 'nu' attribute or variable.");
          return VTK_ERROR;
          }
        ndims = 0;
        NC_SAFE_CALL (nc_inq_varndims(ncids[i], varid, &ndims));
        if (ndims != 2)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has 'nu' variable of incorrect dimension.");
          return VTK_ERROR;
          }
        NC_SAFE_CALL (nc_inq_vardimid(ncids[i], varid, dimids));
        size_t size2 = 0;
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[0], &size2));
        if (size2 != size)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has inconsistent variable lengths");
          return VTK_ERROR;
          }
        components = 0;
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[1], &components));
        if (components != 3)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has 'nu' variable of incorrect dimension.");
          return VTK_ERROR;
          }
        vtkSmartPointer<vtkFloatArray> nu = vtkSmartPointer<vtkFloatArray>::New();
        nu->SetNumberOfComponents(3);
        nu->SetNumberOfTuples(size);
        NC_SAFE_CALL (nc_get_var_float(ncids[i], varid, (float*)nu->GetPointer(0)));
        material->SetPoissonsRatio(nu);
        if (nc_inq_varid (ncids[i], "G", &varid) != NC_NOERR)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has no 'G' attribute or variable.");
          return VTK_ERROR;
          }
        ndims = 0;
        NC_SAFE_CALL (nc_inq_varndims(ncids[i], varid, &ndims));
        if (ndims != 2)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has 'G' variable of incorrect dimension.");
          return VTK_ERROR;
          }
        NC_SAFE_CALL (nc_inq_vardimid(ncids[i], varid, dimids));
        size2 = 0;
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[0], &size2));
        if (size2 != size)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has inconsistent variable lengths");
          return VTK_ERROR;
          }
        components = 0;
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[1], &components));
        if (components != 3)
          {
          vtkErrorMacro (<< "LinearOrthotropic material " << &name[0] << " has 'G' variable of incorrect dimension.");
          return VTK_ERROR;
          }
        vtkSmartPointer<vtkFloatArray> G = vtkSmartPointer<vtkFloatArray>::New();
        G->SetNumberOfComponents(3);
        G->SetNumberOfTuples(size);
        NC_SAFE_CALL (nc_get_var_float(ncids[i], varid, (float*)G->GetPointer(0)));
        material->SetShearModulus(G);
        }
      else
        {
        // Values in attribute -> single value
        if (att_len != 3)
          {
          vtkErrorMacro(<< "LinearOrthotropic material " << &name[0] << " E attribute does not have length 3.");
          return VTK_ERROR;
          }
        vtkboneLinearOrthotropicMaterial* material = vtkboneLinearOrthotropicMaterial::New();
        material->SetName(&name[0]);
        matdefs[std::string(&name[0])] = vtkSmartPointer<vtkboneMaterial>::Take(material);
        double real_vals[3];
        NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "E", real_vals));
        material->SetYoungsModulusX(real_vals[0]);
        material->SetYoungsModulusY(real_vals[1]);
        material->SetYoungsModulusZ(real_vals[2]);
        if (nc_inq_attlen(ncids[i], NC_GLOBAL, "nu", &att_len) != NC_NOERR)
          {
          vtkErrorMacro(<< "LinearOrthotropic material " << &name[0] << " has no 'nu' attribute.");
          return VTK_ERROR;
          }
        if (att_len != 3)
          {
          vtkErrorMacro(<< "LinearOrthotropic material " << &name[0] << " 'nu' attribute does not have length 3.");
          return VTK_ERROR;
          }
        NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "nu", real_vals));
        material->SetPoissonsRatioYZ(real_vals[0]);
        material->SetPoissonsRatioZX(real_vals[1]);
        material->SetPoissonsRatioXY(real_vals[2]);
        if (nc_inq_attlen(ncids[i], NC_GLOBAL, "G", &att_len) != NC_NOERR)
          {
          vtkErrorMacro(<< "LinearOrthotropic material " << &name[0] << " has no G attribute.");
          return VTK_ERROR;
          }
        if (att_len != 3)
          {
          vtkErrorMacro(<< "LinearOrthotropic material " << &name[0] << " G attribute does not have length 3.");
          return VTK_ERROR;
          }
        NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "G", real_vals));
        material->SetShearModulusYZ(real_vals[0]);
        material->SetShearModulusZX(real_vals[1]);
        material->SetShearModulusXY(real_vals[2]);
        }
      }

    else if (type == "LinearAnisotropic")
      {
      if ((nc_inq_attlen(ncids[i], NC_GLOBAL, "StressStrainMatrix", &att_len) != NC_NOERR) ||
          (nc_inq_attlen(ncids[i], NC_GLOBAL, "StiffnessMatrix", &att_len) != NC_NOERR))
        {
        // No attributes -> values must be in a variable.
        vtkboneLinearAnisotropicMaterialArray* material = vtkboneLinearAnisotropicMaterialArray::New();
        material->SetName(&name[0]);
        matdefs[std::string(&name[0])] = vtkSmartPointer<vtkboneMaterial>::Take(material);
        int varid;
        if ((nc_inq_varid (ncids[i], "StressStrain", &varid) != NC_NOERR) ||
            (nc_inq_varid (ncids[i], "StiffnessMatrix", &varid) != NC_NOERR))
          {
          vtkErrorMacro (<< "LinearAnisotropic material " << &name[0] << " has no 'StressStrainMatrix' attribute or variable.");
          return VTK_ERROR;
          }
        int ndims = 0;
        NC_SAFE_CALL (nc_inq_varndims(ncids[i], varid, &ndims));
        if (ndims != 2)
          {
          vtkErrorMacro (<< "LinearAnisotropic material " << &name[0] << " has 'StressStrainMatrix' variable of incorrect dimension.");
          return VTK_ERROR;
          }
        int dimids[2];
        NC_SAFE_CALL (nc_inq_vardimid(ncids[i], varid, dimids));
        size_t size = 0;
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[0], &size));
        size_t components = 0;
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[1], &components));
        if (components != 21)
          {
          vtkErrorMacro (<< "LinearAnisotropic material " << &name[0] << " has 'StressStrainMatrix' variable of incorrect dimension.");
          return VTK_ERROR;
          }
        vtkSmartPointer<vtkFloatArray> K = vtkSmartPointer<vtkFloatArray>::New();
        K->SetNumberOfComponents(21);
        K->SetNumberOfTuples(size);
        NC_SAFE_CALL (nc_get_var_float(ncids[i], varid, (float*)K->GetPointer(0)));
        material->SetStressStrainMatrixUpperTriangular(K);
        }
      else
        {
        // Values in attribute -> single value
        vtkboneLinearAnisotropicMaterial* material = vtkboneLinearAnisotropicMaterial::New();
        material->SetName(&name[0]);
        matdefs[std::string(&name[0])] = vtkSmartPointer<vtkboneMaterial>::Take(material);
        if (att_len != 6*6)
          {
          vtkErrorMacro(<< "LinearAnisotropic material " << &name[0] << " StressStrainMatrix attribute does not have length 36.");
          return VTK_ERROR;
          }
        NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "StressStrainMatrix", material->GetStressStrainMatrix()));
        }
      }

    else if (type == "VonMisesIsotropic")
      {
      vtkboneVonMisesIsotropicMaterial* material = vtkboneVonMisesIsotropicMaterial::New();
      material->SetName(&name[0]);
      matdefs[std::string(&name[0])] = vtkSmartPointer<vtkboneMaterial>::Take(material);
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "E", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "VonMisesIsotropic material " << &name[0] << " has no E attribute.");
        return VTK_ERROR;
        }
      if (att_len != 1)
        {
        vtkErrorMacro(<< "VonMisesIsotropic material " << &name[0] << " E attribute does not have length 1.");
        return VTK_ERROR;
        }
      double real_val;
      NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "E", &real_val));
      material->SetYoungsModulus(real_val);
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "nu", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "VonMisesIsotropic material " << &name[0] << " has no 'nu' attribute.");
        return VTK_ERROR;
        }
      if (att_len != 1)
        {
        vtkErrorMacro(<< "VonMisesIsotropic material " << &name[0] << " 'nu' attribute does not have length 1.");
        return VTK_ERROR;
        }
      NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "nu", &real_val));
      material->SetPoissonsRatio(real_val);
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "Y", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "VonMisesIsotropic material " << &name[0] << " has no 'Y' attribute.");
        return VTK_ERROR;
        }
      if (att_len != 1)
        {
        vtkErrorMacro(<< "VonMisesIsotropic material " << &name[0] << " 'Y' attribute does not have length 1.");
        return VTK_ERROR;
        }
      NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "Y", &real_val));
      material->SetYieldStrength(real_val);
      }

    else if (type == "MaximumPrincipalStrainIsotropic")
      {
      vtkboneMaximumPrincipalStrainIsotropicMaterial* material = vtkboneMaximumPrincipalStrainIsotropicMaterial::New();
      material->SetName(&name[0]);
      matdefs[std::string(&name[0])] = vtkSmartPointer<vtkboneMaterial>::Take(material);
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "E", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "MaximumPrincipalStrainIsotropic material " << &name[0] << " has no E attribute.");
        return VTK_ERROR;
        }
      if (att_len != 1)
        {
        vtkErrorMacro(<< "MaximumPrincipalStrainIsotropic material " << &name[0] << " E attribute does not have length 1.");
        return VTK_ERROR;
        }
      double real_val;
      NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "E", &real_val));
      material->SetYoungsModulus(real_val);
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "nu", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "MaximumPrincipalStrainIsotropic material " << &name[0] << " has no 'nu' attribute.");
        return VTK_ERROR;
        }
      if (att_len != 1)
        {
        vtkErrorMacro(<< "MaximumPrincipalStrainIsotropic material " << &name[0] << " 'nu' attribute does not have length 1.");
        return VTK_ERROR;
        }
      NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "nu", &real_val));
      material->SetPoissonsRatio(real_val);
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "epsilon_YT", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "MaximumPrincipalStrainIsotropic material " << &name[0] << " has no 'epsilon_YT' attribute.");
        return VTK_ERROR;
        }
      if (att_len != 1)
        {
        vtkErrorMacro(<< "MaximumPrincipalStrainIsotropic material " << &name[0] << " 'epsilon_YT' attribute does not have length 1.");
        return VTK_ERROR;
        }
      double epsilon_YT;
      NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "epsilon_YT", &epsilon_YT));
      // This value is optional: if not present, use epsilon_YT
      double epsilon_YC = epsilon_YT;
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "epsilon_YC", &att_len) == NC_NOERR)
        {
        if (att_len != 1)
          {
          vtkErrorMacro(<< "MaximumPrincipalStrainIsotropic material " << &name[0] << " 'epsilon_YC' attribute does not have length 1.");
          return VTK_ERROR;
          }
        NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "epsilon_YC", &epsilon_YC));
        }
      material->SetYieldStrains(epsilon_YT, epsilon_YC);
      }

    else if (type == "MohrCoulombIsotropic")
      {
      vtkboneMohrCoulombIsotropicMaterial* material = vtkboneMohrCoulombIsotropicMaterial::New();
      material->SetName(&name[0]);
      matdefs[std::string(&name[0])] = vtkSmartPointer<vtkboneMaterial>::Take(material);
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "E", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "MohrCoulombIsotropic material " << &name[0] << " has no E attribute.");
        return VTK_ERROR;
        }
      if (att_len != 1)
        {
        vtkErrorMacro(<< "MohrCoulombIsotropic material " << &name[0] << " E attribute does not have length 1.");
        return VTK_ERROR;
        }
      double real_val;
      NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "E", &real_val));
      material->SetYoungsModulus(real_val);
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "nu", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "MohrCoulombIsotropic material " << &name[0] << " has no 'nu' attribute.");
        return VTK_ERROR;
        }
      if (att_len != 1)
        {
        vtkErrorMacro(<< "MohrCoulombIsotropic material " << &name[0] << " 'nu' attribute does not have length 1.");
        return VTK_ERROR;
        }
      NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "nu", &real_val));
      material->SetPoissonsRatio(real_val);
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "c", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "MohrCoulombIsotropic material " << &name[0] << " has no 'c' attribute.");
        return VTK_ERROR;
        }
      if (att_len != 1)
        {
        vtkErrorMacro(<< "MohrCoulombIsotropic material " << &name[0] << " 'c' attribute does not have length 1.");
        return VTK_ERROR;
        }
      NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "c", &real_val));
      material->SetC(real_val);
      if (nc_inq_attlen(ncids[i], NC_GLOBAL, "phi", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "MohrCoulombIsotropic material " << &name[0] << " has no 'phi' attribute.");
        return VTK_ERROR;
        }
      if (att_len != 1)
        {
        vtkErrorMacro(<< "MohrCoulombIsotropic material " << &name[0] << " 'phi' attribute does not have length 1.");
        return VTK_ERROR;
        }
      NC_SAFE_CALL (nc_get_att_double (ncids[i], NC_GLOBAL, "phi", &real_val));
      material->SetPhi(real_val);
      }
    else
      {
      vtkErrorMacro(<< "Material " << &name[0] << " has unrecognized type " << type << ".");
      return VTK_ERROR;
      }
    }

  // -------------------------------------------------------------------
  // Now read the MaterialTable for the active part and store in the model material table.
  // Materials need to be looked up from MaterialDefinitions data.

  int parts_ncid;
  if (nc_inq_ncid(ncid, "Parts", &parts_ncid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Parts group not found.");
    return VTK_ERROR;
    }
  int activePart_ncid;
  if (nc_inq_ncid (parts_ncid, this->GetActivePart(), &activePart_ncid) != NC_NOERR)
    {
    vtkErrorMacro (<< "ActivePart group " << this->GetActivePart() << " not found.");
    return VTK_ERROR;
    }
  int materialTable_ncid;
  if (nc_inq_ncid (activePart_ncid, "MaterialTable", &materialTable_ncid) != NC_NOERR)
    {
    vtkErrorMacro (<< "MaterialTable group not found for Part " << this->GetActivePart() << ".");
    return VTK_ERROR;
    }

  int matid_varid;
  if (nc_inq_varid (materialTable_ncid, "ID", &matid_varid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Unable to find variable ID for MaterialTable for Part " << this->GetActivePart() << ".");
    return VTK_ERROR;
    }
  int ndims = 0;
  NC_SAFE_CALL (nc_inq_varndims(materialTable_ncid, matid_varid, &ndims));
  if (ndims != 1)
    {
    vtkErrorMacro (<< "ID variable in MaterialTable must be 1-dimensional.");
    return VTK_ERROR;
    }
  int dimid;
  NC_SAFE_CALL (nc_inq_vardimid(materialTable_ncid, matid_varid, &dimid));
  size_t matid_len = 0;
  NC_SAFE_CALL (nc_inq_dimlen(materialTable_ncid, dimid, &matid_len));

  std::vector<int> input_ids;
  input_ids.resize(matid_len);
  // The following call crashes on Linux with netCDF 4.2.  No idea why.
  // The nc_get_vara variation seems to be OK though.
//   NC_SAFE_CALL (nc_get_var_int(materialTable_ncid, matid_varid, &input_ids[0]));
  size_t start[1] = {0};
  size_t count[2];
  count[0] = matid_len;
  NC_SAFE_CALL (nc_get_vara_int(materialTable_ncid, matid_varid, start, count, &input_ids[0]));

  int matname_varid;
  if (nc_inq_varid (materialTable_ncid, "MaterialName", &matname_varid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Unable to find variable MaterialName for MaterialTable for Part " << this->GetActivePart() << ".");
    return VTK_ERROR;
    }
  NC_SAFE_CALL (nc_inq_varndims(materialTable_ncid, matname_varid, &ndims));
  if (ndims != 1)
    {
    vtkErrorMacro (<< "MaterialName variable in MaterialTable must be 1-dimensional.");
    return VTK_ERROR;
    }
  NC_SAFE_CALL (nc_inq_vardimid(materialTable_ncid, matname_varid, &dimid));
  size_t matname_len = 0;
  NC_SAFE_CALL (nc_inq_dimlen(materialTable_ncid, dimid, &matname_len));
  if (matname_len != matid_len)
    {
    vtkErrorMacro(<< "ID and MaterialName in MaterialTable must have same length.");
    return VTK_ERROR;
    }
  // NOTE: This is a memory leak if this method exits in any way before
  //       the call to nc_free_string.  We'll consider this not serious.
  char** input_names = (char**)malloc(matname_len*sizeof(char**));
  memset(input_names, 0, matname_len*sizeof(char**));   // not really necessary
  NC_SAFE_CALL (nc_get_var_string(materialTable_ncid, matname_varid, input_names));

  vtkSmartPointer<vtkboneMaterialTable> materialTable =
                                vtkSmartPointer<vtkboneMaterialTable>::New();
  model->SetMaterialTable(materialTable);

  for (int i=0; i<matid_len; ++i)
    {
    if (input_names[i] == NULL)
      {
      vtkErrorMacro(<< "Missing Material Name for material ID " << input_ids[i] <<  ".");
      return VTK_ERROR;
      }
    std::string matname = input_names[i];
    if (matdefs.count(matname) == 0)
      {
      vtkErrorMacro(<< "Material Name \"" << matname <<  "\" not found in Material Definitions.");
      return VTK_ERROR;
      }
    materialTable->AddMaterial(input_ids[i], matdefs[matname]);
    }

  nc_free_string (matname_len, input_names);
  free (input_names);

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadElements
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  int parts_ncid;
  if (nc_inq_ncid(ncid, "Parts", &parts_ncid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Parts group not found.");
    return VTK_ERROR;
    }
  int activePart_ncid;
  if (nc_inq_ncid (parts_ncid, this->GetActivePart(), &activePart_ncid) != NC_NOERR)
    {
    vtkErrorMacro (<< "ActivePart group " << this->GetActivePart() << " not found.");
    return VTK_ERROR;
    }
  int elements_ncid;
  if (nc_inq_ncid (activePart_ncid, "Elements", &elements_ncid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Elements group not found for Part " << this->GetActivePart() << ".");
    return VTK_ERROR;
    }
  int hexahedrons_ncid;
  if (nc_inq_ncid (elements_ncid, "Hexahedrons", &hexahedrons_ncid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Hexahedrons group not found in Elements group for Part " << this->GetActivePart() << ".");
    return VTK_ERROR;
    }

  int elementNumber_varid;
  if (nc_inq_varid (hexahedrons_ncid, "ElementNumber", &elementNumber_varid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Unable to find variable ElementNumber for Hexahedrons group for Part " << this->GetActivePart() << ".");
    return VTK_ERROR;
    }
  int ndims = 0;
  NC_SAFE_CALL (nc_inq_varndims(hexahedrons_ncid, elementNumber_varid, &ndims));
  if (ndims != 1)
    {
    vtkErrorMacro (<< "ElementNumber variable in Hexahedrons group must be 1-dimensional.");
    return VTK_ERROR;
    }
  int dimid[2];
  NC_SAFE_CALL (nc_inq_vardimid(hexahedrons_ncid, elementNumber_varid, dimid));
  size_t elementNumber_len = 0;
  NC_SAFE_CALL (nc_inq_dimlen(hexahedrons_ncid, dimid[0], &elementNumber_len));
  std::vector<vtkIdType> ids_input;
  ids_input.resize(elementNumber_len);
  // The following call crashes on Linux with netCDF 4.2.  No idea why.
  // The nc_get_vara variation seems to be OK though.
//   NC_SAFE_CALL (nc_get_var_longlong(hexahedrons_ncid, elementNumber_varid, &ids_input[0]));
  size_t start[2] = {0,0};
  size_t count[2];
  count[0] = elementNumber_len;
  NC_SAFE_CALL (nc_get_vara_longlong(hexahedrons_ncid, elementNumber_varid, start, count, &ids_input[0]));
  for (vtkIdType i=0; i<elementNumber_len; ++i)
    {
    if (ids_input[i] != i+1)
      {
      vtkErrorMacro (<< "ElementNumbers must start at 1 and be consecutive.");
      return VTK_ERROR;
      }
    }
  ids_input.clear();  // no longer need the data.

  int nodeNumbers_varid;
  if (nc_inq_varid (hexahedrons_ncid, "NodeNumbers", &nodeNumbers_varid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Unable to find variable NodeNumbers for Hexahedrons group for Part " << this->GetActivePart() << ".");
    return VTK_ERROR;
    }
  NC_SAFE_CALL (nc_inq_varndims(hexahedrons_ncid, nodeNumbers_varid, &ndims));
  if (ndims != 2)
    {
    vtkErrorMacro (<< "NodeNumbers variable in Hexahedrons group must be 2-dimensional.");
    return VTK_ERROR;
    }
  NC_SAFE_CALL (nc_inq_vardimid(hexahedrons_ncid, nodeNumbers_varid, dimid));
  size_t nodesPerElement = 0;
  NC_SAFE_CALL (nc_inq_dimlen(hexahedrons_ncid, dimid[1], &nodesPerElement));
  if (nodesPerElement != 8)
    {
    vtkErrorMacro (<< "NodeNumbers variable must have second dimension size of 8 in Hexahedrons.");
    return VTK_ERROR;
    }
  size_t nodeNumbers_len = 0;
  NC_SAFE_CALL (nc_inq_dimlen(hexahedrons_ncid, dimid[0], &nodeNumbers_len));
  if (nodeNumbers_len != elementNumber_len)
    {
    vtkErrorMacro(<< "ElementNumber and NodeNumbers in Elements group must have same length.");
    return VTK_ERROR;
    }
  ids_input.resize(nodeNumbers_len*nodesPerElement);
  // The following call crashes on Linux with netCDF 4.2.  No idea why.
  // The nc_get_vara variation seems to be OK though.
//   NC_SAFE_CALL (nc_get_var_longlong(hexahedrons_ncid, nodeNumbers_varid, &ids_input[0]));
  count[0] = nodeNumbers_len;
  count[1] = nodesPerElement;
  NC_SAFE_CALL (nc_get_vara_longlong(hexahedrons_ncid, nodeNumbers_varid, start, count, &ids_input[0]));
  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  // The following allocation is exact
  cells->Allocate(cells->EstimateSize(nodeNumbers_len, nodesPerElement));
  vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
  pointIds->SetNumberOfIds(nodesPerElement);
  for (vtkIdType newCellId=0; newCellId < nodeNumbers_len; ++newCellId)
    {
    // Convert from 1-indexed to 0-indexed.
    for (int j=0; j<nodesPerElement ; ++j)
      {
      pointIds->SetId(j, ids_input[nodesPerElement*newCellId + j] - 1);
      }
    cells->InsertNextCell(pointIds);
    }
  ids_input.clear();  // no longer need the data.
  model->SetCells(VTK_VOXEL, cells);

  int materialID_varid;
  if (nc_inq_varid (hexahedrons_ncid, "MaterialID", &materialID_varid) != NC_NOERR)
    {
    vtkWarningMacro (<< "Unable to find variable MaterialID for Hexahedrons group for Part " << this->GetActivePart() << ".");
    return VTK_OK;
    }
  NC_SAFE_CALL (nc_inq_varndims(hexahedrons_ncid, materialID_varid, &ndims));
  if (ndims != 1)
    {
    vtkErrorMacro (<< "MaterialID variable in Hexahedrons group must be 1-dimensional.");
    return VTK_ERROR;
    }
  NC_SAFE_CALL (nc_inq_vardimid(hexahedrons_ncid, materialID_varid, dimid));
  size_t materialID_len = 0;
  NC_SAFE_CALL (nc_inq_dimlen(hexahedrons_ncid, dimid[0], &materialID_len));
  if (materialID_len != elementNumber_len)
    {
    vtkErrorMacro(<< "ElementNumber and MaterialID in Elements group must have same length.");
    return VTK_ERROR;
    }
  vtkSmartPointer<vtkIntArray> scalars = vtkSmartPointer<vtkIntArray>::New();
  scalars->SetName("MaterialID");
  scalars->SetNumberOfTuples(materialID_len);
  // The following call crashes on Linux with netCDF 4.2.  No idea why.
  // The nc_get_vara variation seems to be OK though.
//   NC_SAFE_CALL (nc_get_var_int(hexahedrons_ncid, materialID_varid, scalars->GetPointer(0)));
  count[0] = materialID_len;
  NC_SAFE_CALL (nc_get_vara_int(hexahedrons_ncid, materialID_varid, start, count, scalars->GetPointer(0)));
  model->GetCellData()->SetScalars(scalars);

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadConstraint
  (
  int constraints_ncid,
  const char* name,
  vtkboneConstraint*& constraint
  )
  {
  int constraint_ncid;
  if (nc_inq_ncid (constraints_ncid, name, &constraint_ncid) != NC_NOERR)
    {
    vtkErrorMacro(<< "Constraint group " << name << " not found in Constraints group.");
    return VTK_ERROR;
    }

  size_t att_len = 0;
  std::string type;
  if (nc_inq_attlen  (constraint_ncid, NC_GLOBAL, "Type", &att_len) != NC_NOERR)
    {
    vtkErrorMacro(<< "Type attribute not found in Constraint group " << name << ".");
    return VTK_ERROR;
    }
  type.resize(att_len);
  NC_SAFE_CALL (nc_get_att_text (constraint_ncid, NC_GLOBAL, "Type", &type[0]));

  int varid;
  if (nc_inq_varid (constraint_ncid, "NodeNumber", &varid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Unable to find variable NodeNumber for Constraint group " << name << ".");
    return VTK_ERROR;
    }
  int ndims = 0;
  NC_SAFE_CALL (nc_inq_varndims(constraint_ncid, varid, &ndims));
  if (ndims != 1)
    {
    vtkErrorMacro (<< "NodeNumber variable in Constraint group must be 1-dimensional.");
    return VTK_ERROR;
    }
  int dimid;
  NC_SAFE_CALL (nc_inq_vardimid(constraint_ncid, varid, &dimid));
  size_t len = 0;
  NC_SAFE_CALL (nc_inq_dimlen(constraint_ncid, dimid, &len));
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetNumberOfValues(len);
  // The following call crashes on Linux with netCDF 4.2.  No idea why.
  // The nc_get_vara variation seems to be OK though.
//     NC_SAFE_CALL (nc_get_var_longlong(constraint_ncid, varid, ids->GetPointer(0)));
  size_t start[1] = {0};
  size_t count[1];
  count[0] = len;
  NC_SAFE_CALL (nc_get_vara_longlong(constraint_ncid, varid, start, count, ids->GetPointer(0)));
  // Convert to 0-indexed
  for (vtkIdType i=0; i<len; ++i)
    { --*(ids->GetPointer(i)); }

  if (nc_inq_varid (constraint_ncid, "Sense", &varid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Unable to find variable Sense for Constraint group " << name << ".");
    return VTK_ERROR;
    }
  NC_SAFE_CALL (nc_inq_varndims(constraint_ncid, varid, &ndims));
  if (ndims != 1)
    {
    vtkErrorMacro (<< "Sense variable in Constraint group must be 1-dimensional.");
    return VTK_ERROR;
    }
  NC_SAFE_CALL (nc_inq_vardimid(constraint_ncid, varid, &dimid));
  NC_SAFE_CALL (nc_inq_dimlen(constraint_ncid, dimid, &len));
  if (len != ids->GetNumberOfTuples())
    {
    vtkErrorMacro(<< "Sense and NodeNumber variables in Constraint group must have same length.");
    return VTK_ERROR;
    }
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetName("SENSE");
  senses->SetNumberOfValues(len);
// The following call crashes on Linux with netCDF 4.2.  No idea why.
// The nc_get_vara variation seems to be OK though.
//     NC_SAFE_CALL (nc_get_var_schar(constraint_ncid, varid,
//                                    reinterpret_cast<signed char*>(senses->GetPointer(0))));
  count[0] = len;
  NC_SAFE_CALL (nc_get_vara_schar(constraint_ncid, varid, start, count,
                                 reinterpret_cast<signed char*>(senses->GetPointer(0))));
  // Convert to 0-indexed
  for (vtkIdType i=0; i<len; ++i)
    { --*(senses->GetPointer(i)); }

  if (nc_inq_varid (constraint_ncid, "Value", &varid) != NC_NOERR)
    {
    vtkErrorMacro (<< "Unable to find variable Value for Constraint group " << name << ".");
    return VTK_ERROR;
    }
  NC_SAFE_CALL (nc_inq_varndims(constraint_ncid, varid, &ndims));
  if (ndims != 1)
    {
    vtkErrorMacro (<< "Value variable in Constraint group must be 1-dimensional.");
    return VTK_ERROR;
    }
  NC_SAFE_CALL (nc_inq_vardimid(constraint_ncid, varid, &dimid));
  NC_SAFE_CALL (nc_inq_dimlen(constraint_ncid, dimid, &len));
  if (len != ids->GetNumberOfTuples())
    {
    vtkErrorMacro(<< "Value and NodeNumber variables in Constraint group must have same length.");
    return VTK_ERROR;
    }
  vtkSmartPointer<vtkFloatArray> values = vtkSmartPointer<vtkFloatArray>::New();
  values->SetName("VALUE");
  values->SetNumberOfValues(len);
  // The following call crashes on Linux with netCDF 4.2.  No idea why.
  // The nc_get_vara variation seems to be OK though.
//     NC_SAFE_CALL (nc_get_var_float(constraint_ncid, varid, values->GetPointer(0)));
  count[0] = len;
  NC_SAFE_CALL (nc_get_vara_float(constraint_ncid, varid, start, count, values->GetPointer(0)));

  constraint = vtkboneConstraint::New();
  constraint->SetName(name);
  constraint->SetIndices(ids);
  if (type == "NodeAxisDisplacement")
    {
    constraint->SetConstraintType(vtkboneConstraint::DISPLACEMENT);
    constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
    }
  else if (type == "NodeAxisForce")
    {
    constraint->SetConstraintType(vtkboneConstraint::FORCE);
    constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
    }
  else
    {
    vtkErrorMacro(<< "Unable to read constraint of type " << type << ".");
    return VTK_ERROR;
    }
  constraint->GetAttributes()->AddArray(senses);
  constraint->GetAttributes()->AddArray(values);
  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadConstraints
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  // Get list of constraints from Active Problem.

  int problems_ncid;
  NC_SAFE_CALL (nc_inq_ncid(ncid, "Problems", &problems_ncid));
  int activeProblem_ncid;
  NC_SAFE_CALL (nc_inq_ncid(problems_ncid, this->ActiveProblem, &activeProblem_ncid));

  size_t att_len = 0;
  if (nc_inq_attlen  (activeProblem_ncid, NC_GLOBAL, "Constraints", &att_len) != NC_NOERR)
    {
    vtkWarningMacro(<< "Constraints attribute not found in ActiveProblem group " << this->ActiveProblem << ".");
    return VTK_OK;
    }
  std::string buffer;
  buffer.resize(att_len);
  NC_SAFE_CALL (nc_get_att_text (activeProblem_ncid, NC_GLOBAL, "Constraints", &buffer[0]));
  std::vector<std::string> tokens;
  n88util::split_arguments(std::string(buffer), tokens, ",");
  // Backwards compatible splitting on ";"
  if (tokens.size() == 1)
    { n88util::split_arguments(std::string(buffer), tokens, ";"); }

 // Look up required constraints in constraints section

  int constraints_ncid;
  if (nc_inq_ncid (ncid, "Constraints", &constraints_ncid) != NC_NOERR)
    {
    vtkErrorMacro(<< "Constraints group not found in NetCDF file.");
    return VTK_ERROR;
    }

  for (size_t c=0; c<tokens.size(); ++c)
    {
    vtkboneConstraint* constraint = NULL;
    int return_value = this->ReadConstraint (constraints_ncid, tokens[c].c_str(), constraint);
    if (return_value != VTK_OK)
      {
      return return_value;
      }
    n88_assert (constraint);
    model->GetConstraints()->AddItem(constraint);
    constraint->Delete();
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadConvergenceSet
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  // Get convergence set from Active Problem.

  int problems_ncid;
  NC_SAFE_CALL (nc_inq_ncid(ncid, "Problems", &problems_ncid));
  int activeProblem_ncid;
  NC_SAFE_CALL (nc_inq_ncid(problems_ncid, this->ActiveProblem, &activeProblem_ncid));

  size_t att_len = 0;
  if (nc_inq_attlen  (activeProblem_ncid, NC_GLOBAL, "ConvergenceSet", &att_len) != NC_NOERR)
    {
    // Just return if isn't defined.
    return VTK_OK;
    }
  std::string name;
  name.resize(att_len);
  NC_SAFE_CALL (nc_get_att_text (activeProblem_ncid, NC_GLOBAL, "ConvergenceSet", &name[0]));

  // Look up in constraints section

  int constraints_ncid;
  if (nc_inq_ncid (ncid, "Constraints", &constraints_ncid) != NC_NOERR)
    {
    vtkErrorMacro(<< "Constraints group not found in NetCDF file.");
    return VTK_ERROR;
    }

  vtkboneConstraint* convergence_set = NULL;
  int return_value = this->ReadConstraint (constraints_ncid, name.c_str(), convergence_set);
  if (return_value != VTK_OK)
    {
    return return_value;
    }
  n88_assert (convergence_set);
  model->SetConvergenceSet(convergence_set);
  convergence_set->Delete();

  return VTK_OK;
  }

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadSets
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  int sets_ncid;
  if (nc_inq_ncid (ncid, "Sets", &sets_ncid) != NC_NOERR)
    {
    return VTK_OK;
    }

  int nodesets_ncid;
  if (nc_inq_ncid (sets_ncid, "NodeSets", &nodesets_ncid) == NC_NOERR)
    {
    int numSets = 0;
    NC_SAFE_CALL (nc_inq_grps(nodesets_ncid, &numSets, NULL));
    std::vector<int> ncids;
    ncids.resize(numSets);
    NC_SAFE_CALL (nc_inq_grps(nodesets_ncid, &numSets, &ncids.front()));
    std::vector<char> name;
    name.resize(NC_MAX_NAME+1);
    for (int i=0; i<numSets; ++i)
      {
      NC_SAFE_CALL (nc_inq_grpname (ncids[i], &name[0]));
      size_t att_len = 0;
      if (nc_inq_attlen  (ncids[i], NC_GLOBAL, "Part", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "Part attribute not found for NodeSet " << &name[0] << ".");
        return VTK_ERROR;
        }
      std::string part;
      name.resize(att_len);
      NC_SAFE_CALL (nc_get_att_text (ncids[i], NC_GLOBAL, "Part", &part[0]));
      if (strcmp(this->ActivePart, &part[0]) != 0)
        { continue; }
      int varid;
      if (nc_inq_varid (ncids[i], "NodeNumber", &varid) != NC_NOERR)
        {
        vtkErrorMacro (<< "Unable to find variable NodeNumber for NodeSet group " << &name[0] << ".");
        return VTK_ERROR;
        }
      int ndims = 0;
      NC_SAFE_CALL (nc_inq_varndims(ncids[i], varid, &ndims));
      if (ndims != 1)
        {
        vtkErrorMacro (<< "NodeNumber variable in NodeSet group must be 1-dimensional.");
        return VTK_ERROR;
        }
      int dimid;
      NC_SAFE_CALL (nc_inq_vardimid(ncids[i], varid, &dimid));
      size_t len = 0;
      NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimid, &len));
      vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
      ids->SetName (&name[0]);
      ids->SetNumberOfValues(len);
      // The following call crashes on Linux with netCDF 4.2.  No idea why.
      // The nc_get_vara variation seems to be OK though.
//       NC_SAFE_CALL (nc_get_var_longlong(ncids[i], varid, ids->GetPointer(0)));
      size_t start[1] = {0};
      size_t count[1];
      count[0] = len;
      NC_SAFE_CALL (nc_get_vara_longlong(ncids[i], varid, start, count, ids->GetPointer(0)));
      // Convert to 0-indexed
      for (vtkIdType i=0; i<len; ++i)
        { --*(ids->GetPointer(i)); }
      model->AddNodeSet(ids);
      }
    }

  int elementsets_ncid;
  if (nc_inq_ncid (sets_ncid, "ElementSets", &elementsets_ncid) == NC_NOERR)
    {
    int numSets = 0;
    NC_SAFE_CALL (nc_inq_grps(elementsets_ncid, &numSets, NULL));
    std::vector<int> ncids;
    ncids.resize(numSets);
    NC_SAFE_CALL (nc_inq_grps(elementsets_ncid, &numSets, &ncids.front()));
    std::vector<char> name;
    name.resize(NC_MAX_NAME+1);
    for (int i=0; i<numSets; ++i)
      {
      NC_SAFE_CALL (nc_inq_grpname (ncids[i], &name[0]));
      size_t att_len = 0;
      if (nc_inq_attlen  (ncids[i], NC_GLOBAL, "Part", &att_len) != NC_NOERR)
        {
        vtkErrorMacro(<< "Part attribute not found for ElementSet " << &name[0] << ".");
        return VTK_ERROR;
        }
      std::string part;
      name.resize(att_len);
      NC_SAFE_CALL (nc_get_att_text (ncids[i], NC_GLOBAL, "Part", &part[0]));
      if (strcmp(this->ActivePart, &part[0]) != 0)
        { continue; }
      int varid;
      if (nc_inq_varid (ncids[i], "ElementNumber", &varid) != NC_NOERR)
        {
        vtkErrorMacro (<< "Unable to find variable ElementNumber for ElementSet group " << &name[0] << ".");
        return VTK_ERROR;
        }
      int ndims = 0;
      NC_SAFE_CALL (nc_inq_varndims(ncids[i], varid, &ndims));
      if (ndims != 1)
        {
        vtkErrorMacro (<< "ElementNumber variable in ElementSet group must be 1-dimensional.");
        return VTK_ERROR;
        }
      int dimid;
      NC_SAFE_CALL (nc_inq_vardimid(ncids[i], varid, &dimid));
      size_t len = 0;
      NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimid, &len));
      vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
      ids->SetName (&name[0]);
      ids->SetNumberOfValues(len);
      // The following call crashes on Linux with netCDF 4.2.  No idea why.
      // The nc_get_vara variation seems to be OK though.
//       NC_SAFE_CALL (nc_get_var_longlong(ncids[i], varid, ids->GetPointer(0)));
      size_t start[1] = {0};
      size_t count[1];
      count[0] = len;
      NC_SAFE_CALL (nc_get_vara_longlong(ncids[i], varid, start, count, ids->GetPointer(0)));
      // Convert to 0-indexed
      for (vtkIdType i=0; i<len; ++i)
        { --*(ids->GetPointer(i)); }
      model->AddElementSet(ids);
      }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneN88ModelReader::ReadSolutions
(
  int ncid,
  vtkboneFiniteElementModel *model
)
{
  // Do nothing if there is no active solution in the data file.
  if (this->ActiveSolution == NULL)
    { return VTK_OK; }

  int solutions_ncid;
  NC_SAFE_CALL (nc_inq_ncid(ncid, "Solutions", &solutions_ncid));
  int activeSolution_ncid;
  NC_SAFE_CALL (nc_inq_ncid(solutions_ncid, this->ActiveSolution, &activeSolution_ncid));

  // Node Values
  int nodevalues_ncid;
  if (nc_inq_ncid(activeSolution_ncid, "NodeValues", &nodevalues_ncid) == NC_NOERR)
    {
    int nvars;
    NC_SAFE_CALL (nc_inq_varids(nodevalues_ncid, &nvars, NULL));
    std::vector<int> varids;
    varids.resize(nvars);
    NC_SAFE_CALL (nc_inq_varids(nodevalues_ncid, NULL, &varids.front()));
    char name[NC_MAX_NAME+1];
    for (int v=0; v<nvars; ++v)
      {
      NC_SAFE_CALL (nc_inq_varname(nodevalues_ncid, varids[v], name));
      vtkSmartPointer<vtkFloatArray> data = vtkSmartPointer<vtkFloatArray>::New();
      data->SetName(name);
      int ndims = 0;
      int dimids[2];
      size_t dims[2];
      NC_SAFE_CALL (nc_inq_varndims(nodevalues_ncid, varids[v], &ndims));
      if (ndims > 2)
        {
        vtkErrorMacro (<< "Variables in NodeValues group must be 1 or 2 dimensional.");
        return VTK_ERROR;
        }
      NC_SAFE_CALL (nc_inq_vardimid(nodevalues_ncid, varids[v], dimids));
      NC_SAFE_CALL (nc_inq_dimlen(nodevalues_ncid, dimids[0], &dims[0]));
      dims[1] = 1;
      if (ndims == 2)
        {
        NC_SAFE_CALL (nc_inq_dimlen(nodevalues_ncid, dimids[1], &dims[1]));
        }
      data->SetNumberOfComponents(dims[1]);
      if (dims[0] != model->GetNumberOfPoints())
        {
        vtkErrorMacro (<< "Dimensions of variable " << name << " in NodeValues does not match number of nodes in model.");
        return VTK_ERROR;
        }
      data->SetNumberOfTuples(dims[0]);
      // The following call crashes on Linux with netCDF 4.2.  No idea why.
      // The nc_get_vara variation seems to be OK though.
//       NC_SAFE_CALL (nc_get_var_float(nodevalues_ncid, varids[v], data->GetPointer(0)));
      size_t start[2] = {0,0};
      NC_SAFE_CALL (nc_get_vara_float(nodevalues_ncid, varids[v], start, dims, data->GetPointer(0)));
      model->GetPointData()->AddArray(data);
      }
    }

  // Element Values
  int elementvalues_ncid;
  if (nc_inq_ncid(activeSolution_ncid, "ElementValues", &elementvalues_ncid) == NC_NOERR)
    {
    int nvars;
    NC_SAFE_CALL (nc_inq_varids(elementvalues_ncid, &nvars, NULL));
    std::vector<int> varids;
    varids.resize(nvars);
    NC_SAFE_CALL (nc_inq_varids(elementvalues_ncid, NULL, &varids.front()));
    char name[NC_MAX_NAME+1];
    for (int v=0; v<nvars; ++v)
      {
      NC_SAFE_CALL (nc_inq_varname(elementvalues_ncid, varids[v], name));
      vtkSmartPointer<vtkFloatArray> data = vtkSmartPointer<vtkFloatArray>::New();
      data->SetName(name);
      int ndims = 0;
      int dimids[2];
      size_t dims[2];
      NC_SAFE_CALL (nc_inq_varndims(elementvalues_ncid, varids[v], &ndims));
      if (ndims > 2)
        {
        vtkErrorMacro (<< "Variables in ElementValues group must be 1 or 2 dimensional.");
        return VTK_ERROR;
        }
      NC_SAFE_CALL (nc_inq_vardimid(elementvalues_ncid, varids[v], dimids));
      NC_SAFE_CALL (nc_inq_dimlen(elementvalues_ncid, dimids[0], &dims[0]));
      dims[1] = 1;
      if (ndims == 2)
        {
        NC_SAFE_CALL (nc_inq_dimlen(elementvalues_ncid, dimids[1], &dims[1]));
        }
      data->SetNumberOfComponents(dims[1]);
      if (dims[0] != model->GetNumberOfCells())
        {
        vtkErrorMacro (<< "Dimensions of variable " << name << " in ElementValues does not match number of elements in model.");
        return VTK_ERROR;
        }
      data->SetNumberOfTuples(dims[0]);
      // The following call crashes on Linux with netCDF 4.2.  No idea why.
      // The nc_get_vara variation seems to be OK though.
//       NC_SAFE_CALL (nc_get_var_float(elementvalues_ncid, varids[v], data->GetPointer(0)));
      size_t start[2] = {0,0};
      NC_SAFE_CALL (nc_get_vara_float(elementvalues_ncid, varids[v], start, dims, data->GetPointer(0)));
      model->GetCellData()->AddArray(data);
      }

    // Gauss Point Values : All subgroups of ElementValues are gauss point values.
    int numGroups = 0;
    NC_SAFE_CALL (nc_inq_grps(elementvalues_ncid, &numGroups, NULL));
    std::vector<int> ncids;
    ncids.resize(numGroups);
    NC_SAFE_CALL (nc_inq_grps(elementvalues_ncid, &numGroups, &ncids.front()));
    for (int i=0; i<numGroups; ++i)
      {
      int nvars;
      NC_SAFE_CALL (nc_inq_varids(ncids[i], &nvars, NULL));
      std::vector<int> varids;
      varids.resize(nvars);
      NC_SAFE_CALL (nc_inq_varids(ncids[i], NULL, &varids.front()));
      char name[NC_MAX_NAME+1];
      for (int v=0; v<nvars; ++v)
        {
        NC_SAFE_CALL (nc_inq_varname(ncids[i], varids[v], name));
        vtkSmartPointer<vtkFloatArray> data = vtkSmartPointer<vtkFloatArray>::New();
        data->SetName(name);
        int ndims = 0;
        int dimids[3];
        size_t dims[3];
        NC_SAFE_CALL (nc_inq_varndims(ncids[i], varids[v], &ndims));
        if (ndims > 3 || ndims < 2)
          {
          vtkErrorMacro (<< "Variables in ElementValues subgroups must be 2 or 3 dimensional.");
          return VTK_ERROR;
          }
        NC_SAFE_CALL (nc_inq_vardimid(ncids[i], varids[v], dimids));
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[0], &dims[0]));
        NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[1], &dims[1]));
        dims[2] = 1;
        if (ndims == 3)
          {
          NC_SAFE_CALL (nc_inq_dimlen(ncids[i], dimids[2], &dims[2]));
          }
        data->SetNumberOfComponents(dims[2]);
        if (dims[0] != model->GetNumberOfCells())
          {
          vtkErrorMacro (<< "Dimensions of variable " << name << " in ElementValues does not match number of elements in model.");
          return VTK_ERROR;
          }
        data->SetNumberOfTuples(dims[0]*dims[1]);
        // The following call crashes on Linux with netCDF 4.2.  No idea why.
        // The nc_get_vara variation seems to be OK though.
  //       NC_SAFE_CALL (nc_get_var_float(ncids[i], varids[v], data->GetPointer(0)));
        size_t start[3] = {0,0,0};
        NC_SAFE_CALL (nc_get_vara_float(ncids[i], varids[v], start, dims, data->GetPointer(0)));
        model->GetGaussPointData()->AddItem(data);
        }  //  loop over variables
      }  // loop over subgroups

    }  // Element Values

  return VTK_OK;
}
