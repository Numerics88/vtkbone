#include "vtkboneFaimVersion5OutputReader.h"
#include "vtkObjectFactory.h"
#include "vtkFloatArray.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkbone_version.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include <sstream>

#ifdef read
#warning That is weird. Why is read defined?
#undef read
#endif

vtkStandardNewMacro(vtkboneFaimVersion5OutputReader);

//-----------------------------------------------------------------------
// Utility function
inline int ReadUntilString (std::istream& stream, const char *searchstring)
{
  std::string line;
  int i=0;
  while (getline(stream,line).good())
    {
    i++;
    if (line.find(searchstring) != std::string::npos)
      {
      return i;
      }
    }
  return -1;
}


//-----------------------------------------------------------------------
vtkboneFaimVersion5OutputReader::vtkboneFaimVersion5OutputReader()
{
  this->FileName = NULL;
  this->ReadNodalDisplacements = 1;
  this->ReadNodalReactionForces = 1;
  this->ReadElementStresses = 1;
  this->ReadElementStrains = 1;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------
vtkboneFaimVersion5OutputReader::~vtkboneFaimVersion5OutputReader()
{
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
void vtkboneFaimVersion5OutputReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "ReadNodalDisplacements: " << (this->ReadNodalDisplacements ? "On\n" : "Off\n");
  os << indent << "ReadNodalReactionForces: " << (this->ReadNodalReactionForces ? "On\n" : "Off\n");
  os << indent << "ReadElementStresses: " << (this->ReadElementStresses ? "On\n" : "Off\n");
  os << indent << "ReadElementStrains: " << (this->ReadElementStrains ? "On\n" : "Off\n");
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5OutputReader::RequestData(
                               vtkInformation *,
                               vtkInformationVector **,
                               vtkInformationVector *outputVector)
{

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkboneFiniteElementModel *output = vtkboneFiniteElementModel::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
  {
    vtkErrorMacro(<<"No output object.");
    return 0;
  }

  // Make sure we have a file to read.
  if (!this->FileName)
  {
    vtkErrorMacro(<<"FileName not set.");
    return 0;
  }

  // Open the input file.
  std::ifstream fin (this->FileName);
  if (!fin)
    {
    vtkErrorMacro(<<"Error opening file " << this->FileName);
    return 0;
    }
  vtkDebugMacro(<<"Reading " << this->FileName << " ...");

  std::string line;
  int nPoints = 0;
  int nCells = 0;

  std::ostringstream history;
  history << "Model read from Faim version 5 output file \"" << this->FileName << "\" using vtkbone version " << VTKBONE_VERSION;
  output->AppendHistory(history.str().c_str());

  // Determine the number of points and cells
  if (ReadUntilString(fin, "Model Information") == -1) {
    vtkErrorMacro(<<"Model information not found in file.  This file is probably corrupt!");
    return 0;
  }

  if (ReadUntilString(fin, "Element dimensions") == -1) {
    vtkErrorMacro(<<"Line \"Element dimensions\" -> not found in file.  This file is probably corrupt!");
    return 0;
  }
  getline (fin, line);

  sscanf (line.c_str(),"%*s %*s %*s %*s %d",&nPoints);
  getline (fin, line);
  sscanf (line.c_str(),"%*s %*s %*s %*s %d",&nCells);

  // Read in the points
  fin.seekg (0, std::ios::beg);   // rewind to beginning of file.
  vtkDebugMacro(<<"Reading points. " << nPoints << " to read.");
  if (ReadUntilString(fin, "# Global coordinates") == -1) {
    vtkErrorMacro(<<"Nodal data not found in file. "
                    "Ensure that the original model data is contained in .dat file.");
    return 0;
  }
  vtkSmartPointer<vtkFloatArray> points = vtkSmartPointer<vtkFloatArray>::New();
  points->SetNumberOfComponents(3);
  points->SetNumberOfTuples(nPoints);
  for (int i=0; i<nPoints; i++) {
    float x[3];
    getline (fin, line);
    sscanf(line.c_str(),"%*s %*d %f %f %f",x,x+1,x+2);
    points->SetTuple(i,x);
  vtkSmartPointer<vtkPoints> ugPoints = vtkSmartPointer<vtkPoints>::New();
  ugPoints->SetData(points);
  output->SetPoints(ugPoints);
  }

  // Read in the cells
  vtkDebugMacro(<<"Reading cells. " << nCells << " to read.");
  if (ReadUntilString(fin, "# Global node numbers") == -1) {
    vtkErrorMacro(<<"Element connectivity data not found in file.");
    return 0;
  }
  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  cells->Allocate(cells->EstimateSize(8,nCells));
  for (int i=0; i<nCells; i++)
    {
    long int buffer[8];  // required because vtkIdType is not necessarily long.
    vtkIdType pts[8];
    getline (fin, line);
    sscanf (line.c_str(),"%*s %*d %ld %ld %ld %ld %ld %ld %ld %ld",
      buffer,buffer+1,buffer+2,buffer+3,buffer+4,buffer+5,buffer+6,buffer+7);
     // from FE topology to VTK_VOXEL topology
     // also from 1-offset to 0-offset
    pts[0] = buffer[0] - 1;
    pts[1] = buffer[3] - 1;
    pts[2] = buffer[4] - 1;
    pts[3] = buffer[7] - 1;
    pts[4] = buffer[1] - 1;
    pts[5] = buffer[2] - 1;
    pts[6] = buffer[5] - 1;
    pts[7] = buffer[6] - 1;
    cells->InsertNextCell(8, pts);
    }
  output->SetCells(VTK_VOXEL, cells);

  // Read in the nodal displacements
  vtkDebugMacro(<<"\n Reading nodal displacements. " << nPoints << " to read.");
  if (this->ReadNodalDisplacements) {
  if (ReadUntilString(fin, "# Nodal displacements") == -1) {
      vtkWarningMacro(<<"\n  Nodal displacement data not found in file.");
    }
    vtkSmartPointer<vtkFloatArray> nodaldisplacements = vtkSmartPointer<vtkFloatArray>::New();
    nodaldisplacements->SetNumberOfComponents(3);
    nodaldisplacements->SetNumberOfTuples(nPoints);
    nodaldisplacements->SetName("Displacement");
    output->GetPointData()->AddArray(nodaldisplacements);
    for (int i=0; i<nPoints; i++) {
      getline (fin, line);
      float x[3];
      sscanf (line.c_str(),"%*d %f %f %f",x,x+1,x+2);
      nodaldisplacements->SetTuple(i,x);
    }
  }

  // Read in the element stresses
  vtkDebugMacro(<<"\n Reading element stresses. " << nCells << " to read.");
  if (this->ReadElementStresses) {
    if (ReadUntilString(fin,"# Element stress") == -1) {
      vtkWarningMacro(<<"\n  Element stress data not found in file.");
    }
    vtkSmartPointer<vtkFloatArray> stress = vtkSmartPointer<vtkFloatArray>::New();
    stress->SetNumberOfComponents(6);
    stress->SetNumberOfTuples(nCells);
    stress->SetName("Stress");
    output->GetCellData()->AddArray(stress);
    vtkSmartPointer<vtkFloatArray> vm_stress = vtkSmartPointer<vtkFloatArray>::New();
    vm_stress->SetNumberOfComponents(1);
    vm_stress->SetNumberOfTuples(nCells);
    vm_stress->SetName("VonMisesStress");
    output->GetCellData()->AddArray(vm_stress);
    for (int i=0; i<nCells; i++) {
      getline (fin, line);
      float f7tuple[8];
      sscanf (line.c_str(),"%*d %f %f %f %f %f %f %f",
        f7tuple,f7tuple+1,f7tuple+2,f7tuple+5,f7tuple+3,f7tuple+4,f7tuple+6);
      stress->SetTuple(i,f7tuple);
      vm_stress->SetValue(i,*(f7tuple+6));
    }
  }

  // Read in the element strains
  vtkDebugMacro(<<"\n Reading element strains. " << nCells << " to read.");
  if (this->ReadElementStrains) {
    if (ReadUntilString(fin,"# Element strain") == -1) {
      vtkWarningMacro(<<"\n  Element strain data not found in file.");
    }
    vtkSmartPointer<vtkFloatArray> strain = vtkSmartPointer<vtkFloatArray>::New();
    strain->SetNumberOfComponents(6);
    strain->SetNumberOfTuples(nCells);
    strain->SetName("Strain");
    output->GetCellData()->AddArray(strain);
    vtkSmartPointer<vtkFloatArray> ed_strain = vtkSmartPointer<vtkFloatArray>::New();
    ed_strain->SetNumberOfComponents(1);
    ed_strain->SetNumberOfTuples(nCells);
    ed_strain->SetName("StrainEnergyDensity");
    output->GetCellData()->AddArray(ed_strain);
    for (int i=0; i<nCells; i++) {
      getline (fin, line);
      float f7tuple[8];
      sscanf (line.c_str(),"%*d %f %f %f %f %f %f %f",
        f7tuple,f7tuple+1,f7tuple+2,f7tuple+5,f7tuple+3,f7tuple+4,f7tuple+6);
      strain->SetTuple(i,f7tuple);
      ed_strain->SetValue(i,*(f7tuple+6));
    }
  }

  // Read in the nodal reaction forces
  vtkDebugMacro(<<"\n Reading nodal reaction forces. " << nPoints << " to read.");
  if (this->ReadNodalReactionForces) {
    if (ReadUntilString(fin,"# Nodal reaction forces") == -1) {
      vtkWarningMacro(<<"\n  Nodal reaction force data not found in file.");
    }
    vtkSmartPointer<vtkFloatArray> nodalreactionforces = vtkSmartPointer<vtkFloatArray>::New();
    nodalreactionforces->SetNumberOfComponents(3);
    nodalreactionforces->SetNumberOfTuples(nPoints);
    nodalreactionforces->SetName("ReactionForce");
    output->GetPointData()->AddArray(nodalreactionforces);
    for (int i=0; i<nPoints; i++) {
      getline (fin, line);
      float f3tuple[8];
      sscanf (line.c_str(),"%*d %f %f %f",f3tuple,f3tuple+1,f3tuple+2);
      nodalreactionforces->SetTuple(i,f3tuple);
    }
  }

  return 1;
}
