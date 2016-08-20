#include "vtkboneFaimVersion5InputReader.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkboneMaterialTable.h"
#include "vtkboneLinearIsotropicMaterial.h"
#include "vtkboneLinearOrthotropicMaterial.h"
#include "vtkboneConstraint.h"
#include "vtkboneConstraintCollection.h"
#include "vtkboneSolverParameters.h"
#include "vtkbone_version.h"
#include "vtkObjectFactory.h"
#include "vtkIntArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkCharArray.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationStringVectorKey.h"
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <ios>
#include <cassert>

using boost::format;
using boost::lexical_cast;
using std::string;
using std::ios_base;

#ifdef read
#warning That is weird. Why is read defined?
#undef read
#endif

vtkStandardNewMacro(vtkboneFaimVersion5InputReader);

class FAIMInputReaderHelper
{
public:
  FAIMInputReaderHelper(const char* FileName, istream& in);
  int IsIgnorable(const string& l);
  // Returns the next line, that is non-ignorable
  string GetNextLine();
  string& GetLine();
  void SplitLine(const string& l, std::vector<string>& tokens);
  void GetLineAndParse(int N, std::vector<string>& tokens);
  string GetNextToken();
  void GetNTokens(int N, std::vector<string>& tokens);
  template <typename T> void ReadArray(vtkIdType N, T* data);

  const char* FileName;
  istream& in;
  long line_count;
  std::vector<string> UnusedTokens;
  string line;
  string next_line;
};

//-----------------------------------------------------------------------
FAIMInputReaderHelper::FAIMInputReaderHelper(
  const char* FileName_,
  istream& in_)
:
  FileName (FileName_),
  in (in_),
  line_count (-1)
{}

//-----------------------------------------------------------------------
int FAIMInputReaderHelper::IsIgnorable(const string& l)
{
  if (l.empty())
    {
    return 1;
    }
  if ((l[0] == '\n') || (l[0] == '#'))
    {
    return 1;
    }
  return 0;
}

//-----------------------------------------------------------------------
string FAIMInputReaderHelper::GetNextLine()
{
  string l;
  while (1)
    {
    getline(in, l);
    this->line_count += 1;
    if (in.eof())
      {
      return string();  // Return blank string
      }
    if (in.fail())
      {
      throw ios_base::failure((format("IO Error reading %s at line %d") 
                               % this->FileName % this->line_count).str());
      }
    if (!this->IsIgnorable(l))
      {
      break;
      }
    }
  return l;
}

//-----------------------------------------------------------------------
string& FAIMInputReaderHelper::GetLine()
{
  if (this->UnusedTokens.size() > 0)
    {
    throw ios_base::failure((format("Left over values at end of line %d") 
                              % this->line_count).str());
    }
  if (this->line_count == -1)
    {
    this->next_line = this->GetNextLine();
    }
  this->line = this->next_line;
  if (this->line.empty())
    {
    throw ios_base::failure((format("Unexpected end of file at line %d") 
                            % this->line_count).str());
    }
  this->next_line = this->GetNextLine();
  return this->line;
}

//-----------------------------------------------------------------------
void FAIMInputReaderHelper::SplitLine(
  const string& l,
  std::vector<string>& tokens)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separators(" \t,");

  tokens.clear();
  tokenizer tok(l, separators);
  for(tokenizer::iterator beg=tok.begin(); beg!=tok.end();++beg)
    {
    tokens.push_back(*beg);
    }
}

//-----------------------------------------------------------------------
void FAIMInputReaderHelper::GetLineAndParse(
  int N,
  std::vector<string>& tokens)
{
  this->GetLine();
  this->SplitLine(this->line, tokens);
  if (N != -1 && tokens.size() != N)
    {
    throw ios_base::failure((format("Unexpected number of tokens at line %d") 
                             % this->line_count).str());
    }
}

//-----------------------------------------------------------------------
string FAIMInputReaderHelper::GetNextToken()
{
  if (this->UnusedTokens.size() == 0)
    {
    this->GetLine();
    this->SplitLine(this->line, this->UnusedTokens);
    if (this->UnusedTokens.size() == 0)
      {
      throw ios_base::failure((format("Insufficient number of values at line %d") 
                              % this->line_count).str());
      }
    }
  string token = this->UnusedTokens[0];
  this->UnusedTokens.erase(this->UnusedTokens.begin());
  return token;
}

//-----------------------------------------------------------------------
void FAIMInputReaderHelper::GetNTokens(int N, std::vector<string>& tokens)
{
  tokens.clear();
  while (tokens.size() != N)
    {
    tokens.push_back(this->GetNextToken());
    }
}  

//-----------------------------------------------------------------------
template <typename T>
void FAIMInputReaderHelper::ReadArray(vtkIdType N, T* data)
{
  for (vtkIdType i=0; i<N; i++)
    {
    data[i] = lexical_cast<T>(this->GetNextToken());
    }
}

//-----------------------------------------------------------------------
vtkboneFaimVersion5InputReader::vtkboneFaimVersion5InputReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------
vtkboneFaimVersion5InputReader::~vtkboneFaimVersion5InputReader()
{
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
void vtkboneFaimVersion5InputReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";  
}

//----------------------------------------------------------------------------
int vtkboneFaimVersion5InputReader::RequestData(
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
    return 0;
  }

  // Make sure we have a file to read.
  if (!this->FileName)
  {
    vtkErrorMacro("FileName not set.");
    return 0;
  }

  // Open the input file.
  std::ifstream fin (this->FileName);
  if (!fin)
    {
    vtkErrorMacro("Error opening file " << this->FileName);
    return 0;
    }
  vtkDebugMacro(<<"\n Reading " << this->FileName << " ...");

  std::ostringstream history;
  history << "Model read from Faim version 5 input file \"" << this->FileName << "\" using vtkbone version " << VTKBONE_VERSION;
  output->AppendHistory(history.str().c_str());

  // Variables that we are going to read.
  vtkIdType numberOfElements;
  vtkIdType numberOfNodes;
  double spacing[3];
  int numberOfMaterials;

  vtkInformation* info = output->GetInformation();

  FAIMInputReaderHelper reader(this->FileName, fin);

  try
    {
    
    // --------------------------------------------------------------
    // Read Model Parameters

    std::vector<string> tokens;
    reader.GetLineAndParse(8, tokens);
    numberOfElements = lexical_cast<vtkIdType>(tokens[1]);
    numberOfNodes = lexical_cast<vtkIdType>(tokens[2]);
    int nip = lexical_cast<int>(tokens[3]);
    int nodof = lexical_cast<int>(tokens[4]);
    if (nodof != 3)
      {
      vtkErrorMacro("Unsupported number of degrees of freedom in " << this->FileName);
      return 0;
      }
    int nod = lexical_cast<int>(tokens[5]);
    if (nod != 8)
      {
      vtkErrorMacro("Unsupported number of nodes per cell in " << this->FileName);
      return 0;
      }
    int nst = lexical_cast<int>(tokens[6]);
    int ndim = lexical_cast<int>(tokens[7]);
    if (ndim != 3)
      {
      vtkErrorMacro("Unsupported number of dimensions in " << this->FileName);
      return 0;
      }
    
    reader.GetLineAndParse(3, tokens);
    for (int i=0; i<3; i++)
      {
      spacing[i] = lexical_cast<double>(tokens[i]);
      }

    reader.GetLineAndParse(2, tokens);
    int nprops = lexical_cast<int>(tokens[0]);
    numberOfMaterials = lexical_cast<int>(tokens[1]);
    if (nprops != 9)
      {
      vtkErrorMacro(<<"Only material property type 9 (orthotropic) currently handled.");
      return 0;
      }

    // --------------------------------------------------------------
    // Read Material Table
    
    vtkSmartPointer<vtkboneMaterialTable> materialTable = 
                               vtkSmartPointer<vtkboneMaterialTable>::New();
    for (int i=0; i<numberOfMaterials; i++)
      {
      reader.GetLineAndParse(10, tokens);
      int materialIndex = lexical_cast<int>(tokens[0]);
      if (materialIndex < 1)
        {
        vtkErrorMacro(<<"Invalid index in material table line " << reader.line_count);
        return 0;
        }
      if (tokens[2] == tokens[1] && tokens[3] == tokens[1] &&
          tokens[6] == tokens[4] && tokens[5] == tokens[4])
        {
        vtkSmartPointer<vtkboneLinearIsotropicMaterial> material = vtkSmartPointer<vtkboneLinearIsotropicMaterial>::New();
        try
          {
          material->SetYoungsModulus(lexical_cast<double>(tokens[1]));
          material->SetPoissonsRatio(lexical_cast<double>(tokens[4]));
          }
        catch(boost::bad_lexical_cast &)
          {
          vtkErrorMacro(<<"Unable to parse material properties at line " << reader.line_count);
          return 0;
          }
        materialTable->AddMaterial(materialIndex, material);
        }
      else
        {
        vtkSmartPointer<vtkboneLinearOrthotropicMaterial> material = vtkSmartPointer<vtkboneLinearOrthotropicMaterial>::New();
        try
          {
          material->SetYoungsModulusX(lexical_cast<double>(tokens[1]));
          material->SetYoungsModulusY(lexical_cast<double>(tokens[2]));
          material->SetYoungsModulusZ(lexical_cast<double>(tokens[3]));
          material->SetPoissonsRatioXY(lexical_cast<double>(tokens[4]));
          material->SetPoissonsRatioYZ(lexical_cast<double>(tokens[5]));
          material->SetPoissonsRatioZX(lexical_cast<double>(tokens[6]));
          material->SetShearModulusXY(lexical_cast<double>(tokens[7]));
          material->SetShearModulusYZ(lexical_cast<double>(tokens[8]));
          material->SetShearModulusZX(lexical_cast<double>(tokens[9]));
          }
        catch(boost::bad_lexical_cast &)
          {
          vtkErrorMacro(<<"Unable to parse material properties at line " << reader.line_count);
          return 0;
          }
        materialTable->AddMaterial(materialIndex, material);
        }
      }
    output->SetMaterialTable(materialTable);

    // --------------------------------------------------------------
    // Read Solver Parameters
    
    reader.GetLineAndParse(6, tokens);
    double convergenceTolerance = lexical_cast<double>(tokens[0]);
    vtkboneSolverParameters::CONVERGENCE_TOLERANCE()->Set (info, convergenceTolerance);
    int iterationLimit = lexical_cast<int>(tokens[1]);
    vtkboneSolverParameters::MAXIMUM_ITERATIONS()->Set (info, iterationLimit);
    
    // --------------------------------------------------------------
    // Read Nodes (Points)
    
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToFloat();
    points->SetNumberOfPoints(numberOfNodes);
    reader.ReadArray(numberOfNodes*3,
                     reinterpret_cast<float*>(points->GetVoidPointer(0)));
    output->SetPoints(points);

    // --------------------------------------------------------------
    // Read Elements (Cells)
    
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    // The following allocation is exact
    const int pointsPerCell = 8;
    cells->Allocate(cells->EstimateSize(numberOfElements, pointsPerCell));
    {  // scope
    vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
    pointIds->SetNumberOfIds(pointsPerCell);    
    for (vtkIdType newCellId=0; newCellId < numberOfElements; newCellId++)
      {
      reader.GetLineAndParse(pointsPerCell, tokens);
      // Convert from FAIM topology to VTK_VOXEL topolgy.
      // Note: Also convert from 1-indexed to 0-indexed.
      pointIds->SetId(0, lexical_cast<vtkIdType>(tokens[0])-1);
      pointIds->SetId(1, lexical_cast<vtkIdType>(tokens[3])-1);
      pointIds->SetId(2, lexical_cast<vtkIdType>(tokens[4])-1);
      pointIds->SetId(3, lexical_cast<vtkIdType>(tokens[7])-1);
      pointIds->SetId(4, lexical_cast<vtkIdType>(tokens[1])-1);
      pointIds->SetId(5, lexical_cast<vtkIdType>(tokens[2])-1);
      pointIds->SetId(6, lexical_cast<vtkIdType>(tokens[5])-1);
      pointIds->SetId(7, lexical_cast<vtkIdType>(tokens[6])-1);
      cells->InsertNextCell(pointIds);
      }
    }
    output->SetCells(VTK_VOXEL, cells);

    // --------------------------------------------------------------
    // Read Material Indices (Cell Scalars)
    
    vtkSmartPointer<vtkIntArray> scalars = vtkSmartPointer<vtkIntArray>::New();
    scalars->SetNumberOfTuples(numberOfElements);
    reader.ReadArray(numberOfElements,
                     reinterpret_cast<int*>(scalars->GetVoidPointer(0)));
    output->GetCellData()->SetScalars(scalars);
    
    // --------------------------------------------------------------
    // Read Fixed Nodes Constraint Set

    reader.GetLineAndParse(1, tokens);
    vtkIdType numberOfFixedFreedoms = lexical_cast<vtkIdType>(tokens[0]);
    if (numberOfFixedFreedoms > 0)
      {
      vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
      ids->Allocate(numberOfFixedFreedoms);
      vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
      senses->Allocate(numberOfFixedFreedoms);
      vtkSmartPointer<vtkDoubleArray> values = vtkSmartPointer<vtkDoubleArray>::New();
      values->Allocate(numberOfFixedFreedoms);
      for (vtkIdType i=0; i<numberOfFixedFreedoms; i++)
        {
        reader.GetLineAndParse(4, tokens);
        vtkIdType nodeId = lexical_cast<vtkIdType>(tokens[0]) - 1;
        if (lexical_cast<int>(tokens[1]) == 0)
          {
          ids->InsertNextValue(nodeId);
          senses->InsertNextValue(0);
          values->InsertNextValue(0.0);
          }
        if (lexical_cast<int>(tokens[2]) == 0)
          {
          ids->InsertNextValue(nodeId);
          senses->InsertNextValue(1);
          values->InsertNextValue(0.0);
          }
        if (lexical_cast<int>(tokens[3]) == 0)
          {
          ids->InsertNextValue(nodeId);
          senses->InsertNextValue(2);
          values->InsertNextValue(0.0);
          }
        }
      if (ids->GetNumberOfTuples() != senses->GetNumberOfTuples() ||
          ids->GetNumberOfTuples() != values->GetNumberOfTuples())
        {
        vtkErrorMacro(<<"Internal Error");
        return 0;
        }
      output->ApplyBoundaryCondition(ids, senses, values, "FixedConstraints");
      }

    // --------------------------------------------------------------
    // Read Force Constraints (on nodes)

    reader.GetLineAndParse(1, tokens);
    vtkIdType numberOfForceConstraints = lexical_cast<vtkIdType>(tokens[0]);
    if (numberOfForceConstraints > 0)
      {
      vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
      ids->Allocate(numberOfForceConstraints);
      vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
      senses->SetName("SENSE");
      senses->Allocate(numberOfForceConstraints);
      vtkSmartPointer<vtkFloatArray> values = vtkSmartPointer<vtkFloatArray>::New();
      values->SetName("VALUE");
      values->Allocate(numberOfForceConstraints);
      for (vtkIdType i=0; i<numberOfForceConstraints; i++)
        {
        reader.GetLineAndParse(3, tokens);
        ids->InsertNextValue(lexical_cast<vtkIdType>(tokens[0]) - 1);
        senses->InsertNextValue(lexical_cast<vtkIdType>(tokens[1]) - 1);
        values->InsertNextValue(lexical_cast<float>(tokens[2]));
        }
      vtkSmartPointer<vtkboneConstraint> constraint = vtkSmartPointer<vtkboneConstraint>::New();
      constraint->SetName("ForceConstraints");
      constraint->SetIndices(ids);
      constraint->SetConstraintType(vtkboneConstraint::FORCE);
      constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
      constraint->GetAttributes()->AddArray(senses);
      constraint->GetAttributes()->AddArray(values);
      output->GetConstraints()->AddItem(constraint);
      }

    // --------------------------------------------------------------
    // Read Displacement Constraints

    reader.GetLineAndParse(1, tokens);
    vtkIdType numberOfDisplacementConstraints = lexical_cast<vtkIdType>(tokens[0]);
    if (numberOfDisplacementConstraints > 0)
      {
      vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
      ids->Allocate(numberOfDisplacementConstraints);
      vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
      senses->SetName("SENSE");
      senses->Allocate(numberOfDisplacementConstraints);
      vtkSmartPointer<vtkFloatArray> values = vtkSmartPointer<vtkFloatArray>::New();
      values->SetName("VALUE");
      values->Allocate(numberOfDisplacementConstraints);
      for (vtkIdType i=0; i<numberOfDisplacementConstraints; i++)
        {
        reader.GetLineAndParse(3, tokens);
        ids->InsertNextValue(lexical_cast<vtkIdType>(tokens[0]) - 1);
        senses->InsertNextValue(lexical_cast<vtkIdType>(tokens[1]) - 1);
        values->InsertNextValue(lexical_cast<float>(tokens[2]));
        }
      vtkSmartPointer<vtkboneConstraint> constraint = vtkSmartPointer<vtkboneConstraint>::New();
      constraint->SetName("DisplacementConstraints");
      constraint->SetIndices(ids);
      constraint->SetConstraintType(vtkboneConstraint::DISPLACEMENT);
      constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
      constraint->GetAttributes()->AddArray(senses);
      constraint->GetAttributes()->AddArray(values);
      output->GetConstraints()->AddItem(constraint);
      }

    // --------------------------------------------------------------
    // Read Node Sets

    reader.GetLineAndParse(1, tokens);
    vtkIdType numberOfNodeSets = lexical_cast<vtkIdType>(tokens[0]);
    for (int nodeSetId=0; nodeSetId<numberOfNodeSets; nodeSetId++)
      {
      reader.GetLineAndParse(1, tokens);
      vtkIdType numberOfIds = lexical_cast<vtkIdType>(tokens[0]);
      string selectionName = (format("NodeSet%d") % (nodeSetId+1)).str();
      vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
      ids->SetName (selectionName.c_str());
      ids->SetNumberOfTuples(numberOfIds);
      reader.ReadArray(numberOfIds,
                       reinterpret_cast<vtkIdType*>(ids->GetVoidPointer(0)));
      // Convert to 0-indexed from 1-indexed.
      for (vtkIdType i=0; i<numberOfIds; i++)
        {
        ids->SetValue(i,ids->GetValue(i)-1);
        }
      output->AddNodeSet(ids);
      vtkboneSolverParameters::POST_PROCESSING_NODE_SETS()->Append (
                                                info, selectionName.c_str());
      }

    // --------------------------------------------------------------
    // Read Element Sets

    reader.GetLineAndParse(1, tokens);
    vtkIdType numberOfElementSets = lexical_cast<vtkIdType>(tokens[0]);
    for (int elementSetId=0; elementSetId<numberOfElementSets; elementSetId++)
      {
      reader.GetLineAndParse(1, tokens);
      vtkIdType numberOfIds = lexical_cast<vtkIdType>(tokens[0]);
      string selectionName = (format("ElementSet%d") % (elementSetId+1)).str();
      vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
      ids->SetName (selectionName.c_str());
      ids->SetNumberOfTuples(numberOfIds);
      reader.ReadArray(numberOfIds,
                       reinterpret_cast<vtkIdType*>(ids->GetVoidPointer(0)));
      // Convert to 0-indexed from 1-indexed.
      for (vtkIdType i=0; i<numberOfIds; i++)
        {
        ids->SetValue(i,ids->GetValue(i)-1);
        }
      output->AddElementSet(ids);
      vtkboneSolverParameters::POST_PROCESSING_ELEMENT_SETS()->Append (
                                                info, selectionName.c_str());
      }

    }
  catch (boost::bad_lexical_cast&)
    {
    vtkErrorMacro(<<"Error reading file " << this->FileName
                  << ". Parse error line " << reader.line_count);
    return 0;
    }
  catch (std::exception& e)
    {
    vtkErrorMacro(<<"Error reading file " << this->FileName << ". " << e.what());
    return 0;
    }

  return 1;
}
