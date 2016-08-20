#include "vtkboneAbaqusInputWriter.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkboneMaterialTable.h"
#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkboneLinearOrthotropicMaterial.h"
#include "vtkboneConstraint.h"
#include "vtkboneConstraintCollection.h"
#include "vtkboneConstraintUtilities.h"
#include "vtkboneSolverParameters.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkLongLongArray.h"
#include "vtkUnsignedLongLongArray.h"
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
#include "vtkDoubleArray.h"
#include "vtkSmartPointer.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationStringVectorKey.h"
#include "n88util/text.hpp"
#include "n88util/exception.hpp"
#include <boost/format.hpp>

using boost::format;

vtkStandardNewMacro(vtkboneAbaqusInputWriter);


//----------------------------------------------------------------------------
vtkboneAbaqusInputWriter::vtkboneAbaqusInputWriter()
:
  FileName (NULL)
{
}

//----------------------------------------------------------------------------
vtkboneAbaqusInputWriter::~vtkboneAbaqusInputWriter()
{
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
void vtkboneAbaqusInputWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkboneFiniteElementModel");
  return 1;
}

//----------------------------------------------------------------------------
void vtkboneAbaqusInputWriter::WriteData()
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
  std::ofstream f (this->FileName);
  if (f.fail())
    {
    vtkErrorMacro(<< "Unable to open file " << this->FileName);
    return;
    }

  if (this->WriteHeading(f,model) != VTK_OK) { return; }
  if (this->WriteNodes(f,model) != VTK_OK) { return; }
  if (this->WriteElements(f,model) != VTK_OK) { return; }
  if (this->WriteMaterials(f,model) != VTK_OK) { return; }
  if (this->WriteNodeSets(f,model) != VTK_OK) { return; }
  if (this->WriteElementSets(f,model) != VTK_OK) { return; }
  if (this->WriteSolidSections(f,model) != VTK_OK) { return; }
  if (this->WriteStep(f,model) != VTK_OK) { return; }

  return;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteHeading
(std::ostream& f, vtkboneFiniteElementModel* model)
{
  f << "*HEADING\n";
  if (model->GetName())
    { f << model->GetName() << "\n"; }
  f << "Abaqus export from vtkbone\n";

  if (model->GetHistory())
    {
    std::vector<std::string> lines;
    n88util::split_arguments(model->GetHistory(), lines, "\n");
    for (int i=0; i<lines.size(); ++i)
      { f << "** " << lines[i] << "\n"; }
    }
  f << "**\n";
  if (model->GetLog())
    {
    std::vector<std::string> lines;
    n88util::split_arguments(model->GetLog(), lines, "\n");
    for (int i=0; i<lines.size(); ++i)
      { f << "** " << lines[i] << "\n"; }
    }
  
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteNodes
(std::ostream& f, vtkboneFiniteElementModel* model)
{
  f << "*NODE\n";
  vtkPoints* points = model->GetPoints();
  vtkIdType N = points->GetNumberOfPoints();
  for (vtkIdType i=0; i<N; ++i)
    {
    double* p = points->GetPoint(i);
    f << format("%i, %.8g, %.8g, %.8g\n")
           % (i+1) % p[0] % p[1] % p[2];
    }
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteElements
(std::ostream& f, vtkboneFiniteElementModel* model)
{
  f << "*ELEMENT, TYPE=C3D8\n";
  vtkCellArray* cells = model->GetCells();
  cells->InitTraversal();
  vtkIdType npts = 0;
  vtkIdType* pts = NULL;
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
        transform[2] = 3;
        transform[3] = 2;
        transform[4] = 4;
        transform[5] = 5;
        transform[6] = 7;
        transform[7] = 6;
        break;
      case VTK_HEXAHEDRON:
        transform[0] = 0;
        transform[1] = 1;
        transform[2] = 2;
        transform[3] = 3;
        transform[4] = 4;
        transform[5] = 5;
        transform[6] = 6;
        transform[7] = 7;
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
    ++cellid;
    f << cellid;
    // Convert to 1-indexed
    for (int i=0; i<8; ++i)
      { f << ", " << pts[transform[i]] + 1; }
    f << "\n";
    }
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteMaterials
(std::ostream& f, vtkboneFiniteElementModel* model)
{
  vtkboneMaterialTable* materialTable = model->GetMaterialTable();
  if (!materialTable || materialTable->GetNumberOfMaterials() == 0)
    {
    // Ignore if no MaterialTable
    return VTK_OK;
    }

  if (materialTable->CheckNames() == 0)
    {
    vtkErrorMacro(<< "Material names are not unique.");
    return VTK_ERROR;
    }
  materialTable->InitTraversal();
  
  while (int index = materialTable->GetNextUniqueIndex())
    {
    vtkboneMaterial* material = materialTable->GetCurrentMaterial();
    std::string name = this->SpacesToUnderscores(material->GetName());
    if (vtkboneLinearIsotropicMaterial* isomat =
        vtkboneLinearIsotropicMaterial::SafeDownCast(material))
      {
      f << "*MATERIAL, NAME=" << name << "\n"
        << "*ELASTIC, TYPE=ISOTROPIC\n"
        << isomat->GetYoungsModulus() << ", " << isomat->GetPoissonsRatio() << "\n";
      }
    else if (vtkboneLinearOrthotropicMaterial* orthomat =
        vtkboneLinearOrthotropicMaterial::SafeDownCast(material))
      {
      f << "*MATERIAL, NAME=" << name << "\n"
        << "*ELASTIC, TYPE=ORTHOTROPIC\n"
        << (1.0/orthomat->GetYoungsModulusX()) << ", "
        << (-orthomat->GetPoissonsRatioXY()/orthomat->GetYoungsModulusX()) << ", "
        << (1.0/orthomat->GetYoungsModulusY()) << ", "
        << (-orthomat->GetPoissonsRatioZX()/orthomat->GetYoungsModulusZ()) << ", "
        << (-orthomat->GetPoissonsRatioYZ()/orthomat->GetYoungsModulusY()) << ", "
        << (1.0/orthomat->GetYoungsModulusZ()) << ", "
        << (1.0/orthomat->GetShearModulusXY()) << ", "
        << (1.0/orthomat->GetShearModulusZX()) << "\n"
        << (1.0/orthomat->GetShearModulusYZ()) << "\n";
      }
    else
      {
      vtkWarningMacro(<<"Unsupported material type.");
      }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteNodeSets
(std::ostream& f, vtkboneFiniteElementModel* model)
{
  if (model->GetNodeSets()->GetNumberOfItems() == 0)
    {
    return VTK_OK;
    }

  for (int n=0; n<model->GetNodeSets()->GetNumberOfItems(); n++)
    {
    // No error checking required on next 2 calls; did that already in DefineSets.
    vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast (model->GetNodeSets()->GetItem(n));
    std::string setName = this->SpacesToUnderscores(ids->GetName());
    f << "*NSET, NSET=" << setName << "\n";
    if (this->WriteIndexArray (f, ids) != VTK_OK)
      { return VTK_ERROR; }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteElementSets
(std::ostream& f, vtkboneFiniteElementModel* model)
{
  if (model->GetElementSets()->GetNumberOfItems() == 0)
    {
    return VTK_OK;
    }

  for (int n=0; n<model->GetElementSets()->GetNumberOfItems(); n++)
    {
    // No error checking required on next 2 calls; did that already in DefineSets.
    vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast (model->GetElementSets()->GetItem(n));
    std::string setName = this->SpacesToUnderscores(ids->GetName());
    f << "*ELSET, ELSET=" << setName << "\n";
    if (this->WriteIndexArray (f, ids) != VTK_OK)
      { return VTK_ERROR; }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteSolidSections
(std::ostream& f, vtkboneFiniteElementModel* model)
{
  vtkboneMaterialTable* materialTable = model->GetMaterialTable();
  if (!materialTable || materialTable->GetNumberOfMaterials() == 0)
    {
    // Ignore if no MaterialTable
    return VTK_OK;
    }

  vtkDataArray* scalars = model->GetCellData()->GetScalars();
  if (scalars == NULL)
    {
    // Ignore if no scalars
    return VTK_OK;
    }
  
  materialTable->InitTraversal();
  
  while (int index = materialTable->GetNextUniqueIndex())
    {
    vtkboneMaterial* material = materialTable->GetCurrentMaterial();
    std::string materialName = this->SpacesToUnderscores(material->GetName());
    std::string setName = materialName + "_ele";
    // Ensure that no element set has this name.
      {
      int i=1;
      while (model->GetElementSet(setName.c_str()))
        {
        setName = (format("%s_ele_%d") % materialName % i).str();
        ++i;
        }
      }
    // Traverse all elements to find which are this material.
    vtkIdType count = 0;
    for (vtkIdType i=0; i<scalars->GetNumberOfTuples(); ++i)
      {
      if (materialTable->GetMaterial(int(scalars->GetTuple1(i))) == material)
        {
        if (count == 0)
          {
          f << "*ELSET, ELSET=" << setName << "\n";
          }
        else
          {
          f << ",";
          if ((count % 10) == 0)
            { f << "\n"; }
          else
            { f << " "; }
          }
        f << i+1;
        ++count;
        }
      }
      f << "\n";
      f << "*SOLID SECTION, ELSET=" << setName
        << ", MATERIAL=" << materialName << "\n";
    }
  
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteStep
(std::ostream& f, vtkboneFiniteElementModel* model)
{
  f << "*STEP\n"
    << "*STATIC\n";
  if (this->WriteBoundaries (f, model) != VTK_OK) { return VTK_ERROR; }
  if (this->WriteLoads (f, model) != VTK_OK) { return VTK_ERROR; }
  f << "*END STEP\n";
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteBoundaries
(std::ostream& f, vtkboneFiniteElementModel* model)
{
  vtkboneConstraintCollection* constraints = model->GetConstraints();
  constraints->InitTraversal();
  while (vtkboneConstraint* constraint = constraints->GetNextItem())
    {
    if (constraint->GetConstraintType() != vtkboneConstraint::DISPLACEMENT)
      { continue; }
    std::string name = this->SpacesToUnderscores(constraint->GetName());
    if (constraint->GetConstraintAppliedTo() == vtkboneConstraint::NODES)
      {
      vtkIdTypeArray* indices = constraint->GetIndices();
      vtkDataArray* senses = constraint->GetAttributes()->GetArray("SENSE");
      if (senses == NULL)
        {
        vtkErrorMacro(<<"Missing constraint attribute array SENSE.");
        return VTK_ERROR;
        }
      vtkDataArray* values = constraint->GetAttributes()->GetArray("VALUE");
      if (values == NULL)
        {
        vtkErrorMacro(<<"Missing constraint attribute array VALUE.");
        return VTK_ERROR;
        }
      if (indices->GetNumberOfTuples() != senses->GetNumberOfTuples() ||
          indices->GetNumberOfTuples() != values->GetNumberOfTuples())
        {
        vtkErrorMacro(<<"Indices and SENSE and VALUE not all same length.");
        return VTK_ERROR;
        }
      f << "** Name: " << name << "\n";
      f << "*BOUNDARY, TYPE=DISPLACEMENT\n";
      for (int i=0; i<indices->GetNumberOfTuples(); ++i)
        {
        int sense = senses->GetTuple1(i) + 1;
        f << indices->GetValue(i) + 1 << ", "
          << sense << ", , "
          << values->GetTuple1(i) << "\n";
        }
      }
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteLoads
(std::ostream& f, vtkboneFiniteElementModel* model)
{
  vtkboneConstraintCollection* constraints = model->GetConstraints();
  constraints->InitTraversal();
  while (vtkboneConstraint* constraint = constraints->GetNextItem())
    {
    if (constraint->GetConstraintType() != vtkboneConstraint::FORCE)
      { continue; }
    std::string name = this->SpacesToUnderscores(constraint->GetName());
    if (constraint->GetConstraintAppliedTo() == vtkboneConstraint::ELEMENTS)
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
      f << "** Name: " << name << "\n";
      f << "*CLOAD\n";
      for (int i=0; i<ids->GetNumberOfTuples(); ++i)
        {
        int sense = senses->GetTuple1(i) + 1;
        f << ids->GetValue(i) + 1 << ", "
          << sense << ", "
          << values->GetTuple1(i) << "\n";
        }
      }
    else   // constraints are applied to Nodes
      {
      vtkIdTypeArray* indices = constraint->GetIndices();
      vtkDataArray* senses = constraint->GetAttributes()->GetArray("SENSE");
      if (senses == NULL)
        {
        vtkErrorMacro(<<"Missing constraint attribute array SENSE.");
        return VTK_ERROR;
        }
      vtkDataArray* values = constraint->GetAttributes()->GetArray("VALUE");
      if (values == NULL)
        {
        vtkErrorMacro(<<"Missing constraint attribute array VALUE.");
        return VTK_ERROR;
        }
      if (indices->GetNumberOfTuples() != senses->GetNumberOfTuples() ||
          indices->GetNumberOfTuples() != values->GetNumberOfTuples())
        {
        vtkErrorMacro(<<"Indices and SENSE and VALUE not all same length.");
        return VTK_ERROR;
        }
      f << "** Name: " << name << "\n";
      f << "*CLOAD\n";
      for (int i=0; i<indices->GetNumberOfTuples(); ++i)
        {
        int sense = senses->GetTuple1(i) + 1;
        f << indices->GetValue(i) + 1 << ", "
          << sense << ", "
          << values->GetTuple1(i) << "\n";
        }
      }
      
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneAbaqusInputWriter::WriteIndexArray
(std::ostream& f, vtkIdTypeArray* data)
{
  for (vtkIdType i=0; i<data->GetNumberOfTuples(); ++i)
    {
    if (i != 0)
      {
      f << ",";
      if ((i % 10) == 0)
        { f << "\n"; }
      else
        { f << " "; }
      }
    f << data->GetValue(i)+1;
    }
  f << "\n";
  return VTK_OK;
}

//----------------------------------------------------------------------------
std::string vtkboneAbaqusInputWriter::SpacesToUnderscores (std::string s)
{
  size_t found = s.find_first_of(' ');
  while (found != std::string::npos)
    {
    s[found] = '_';
    found = s.find_first_of(' ', found+1);
    }
  return s;
}
