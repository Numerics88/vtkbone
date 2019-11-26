#include "vtkboneFiniteElementModel.h"
#include "vtkboneMacros.h"
#include "vtkboneConstraintCollection.h"
#include "vtkboneConstraint.h"
#include "vtkboneConstraintUtilities.h"
#include "vtkboneMaterialTable.h"
#include "vtkObjectFactory.h"
#include "vtkIdTypeArray.h"
#include "vtkDataArrayCollection.h"
#include "vtkPolyData.h"
#include "vtkCell.h"
#include "vtkGenericCell.h"
#include "vtkCellArray.h"
#include "vtkDataArray.h"
#include "vtkIntArray.h"
#include "vtkFloatArray.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#include "vtkCharArray.h"
#include "vtkSelection.h"
#include "vtkExtractSelection.h"
#include "vtkCellData.h"
#include "vtkConvertSelection.h"
#include "vtkboneSelectionUtilities.h"
#include "vtkDataSetAttributes.h"
#include "vtkGeometryFilter.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkboneMacros.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>
#include <cassert>

vtkStandardNewMacro (vtkboneFiniteElementModel);

vtkCxxSetObjectMacro (vtkboneFiniteElementModel, Constraints, vtkboneConstraintCollection);
vtkCxxSetObjectMacro (vtkboneFiniteElementModel, MaterialTable, vtkboneMaterialTable);
vtkCxxSetObjectMacro (vtkboneFiniteElementModel, ConvergenceSet, vtkboneConstraint);
vtkCxxSetObjectMacro (vtkboneFiniteElementModel, NodeSets, vtkDataArrayCollection);
vtkCxxSetObjectMacro (vtkboneFiniteElementModel, ElementSets, vtkDataArrayCollection);
vtkCxxSetObjectMacro (vtkboneFiniteElementModel, GaussPointData, vtkDataArrayCollection);

const char* const ElementType_s[] = {
    "UNKNOWN",
    "TETRAHEDRON",
    "HEXAHEDRON",
    "MIXED"};
vtkboneGetAsStringMacro (vtkboneFiniteElementModel, ElementType);


//----------------------------------------------------------------------------
vtkboneFiniteElementModel::vtkboneFiniteElementModel()
{
  this->Constraints = vtkboneConstraintCollection::New();
  this->Constraints->Register(this);
  this->Constraints->Delete();  // Otherwise reference count is 2
  this->MaterialTable = vtkboneMaterialTable::New();
  this->MaterialTable->Register(this);
  this->MaterialTable->Delete();  // Otherwise reference count is 2
  this->ConvergenceSet = NULL;
  this->Name = NULL;
  this->History = NULL;
  this->Log = NULL;
  this->NodeSets = vtkDataArrayCollection::New();
  this->NodeSets->Register(this);
  this->NodeSets->Delete();  // Otherwise reference count is 2
  this->ElementSets = vtkDataArrayCollection::New();
  this->ElementSets->Register(this);
  this->ElementSets->Delete();  // Otherwise reference count is 2
  this->GaussPointData = vtkDataArrayCollection::New();
  this->GaussPointData->Register(this);
  this->GaussPointData->Delete();  // Otherwise reference count is 2
}

//----------------------------------------------------------------------------
vtkboneFiniteElementModel::~vtkboneFiniteElementModel()
  {
  this->SetConstraints(NULL);
  this->SetMaterialTable(NULL);
  this->SetConvergenceSet(NULL);
  this->SetName(NULL);
  this->SetHistory(NULL);
  this->SetLog(NULL);
  this->SetNodeSets(NULL);
  this->SetElementSets(NULL);
  this->SetGaussPointData(NULL);
  }

//----------------------------------------------------------------------------
void vtkboneFiniteElementModel::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if (this->History)
    { os << indent << "History :\n" << this->History << "\n"; }
  else
    { os << indent << "History : (None)\n"; }
  if (this->NodeSets)
    {
    os << indent << "Node Sets :\n";
    this->NodeSets->PrintSelf(os, indent.GetNextIndent());
    }
  else
    { os << indent << "Node Sets : (None)\n"; }
  if (this->ElementSets)
    {
    os << indent << "Element Sets :\n";
    this->ElementSets->PrintSelf(os, indent.GetNextIndent());
    }
  else
    { os << indent << "Element Sets : (None)\n"; }
  if (this->GaussPointData)
    {
    os << indent << "Gauss Point Data :\n";
    this->GaussPointData->PrintSelf(os, indent.GetNextIndent());
    }
  else
    { os << indent << "Gauss Point Data : (None)\n"; }

    {
    vtkboneConstraint *tmp;
    vtkIdType num = this->Constraints->GetNumberOfItems();
    os << indent << "Number Of Contraints : " << num << endl;
    for (vtkIdType i=0; i<num; i++)
      {
      if ((tmp = vtkboneConstraint::SafeDownCast(this->Constraints->GetItemAsObject(i))))
        {
        if (tmp->GetName())
          os << indent << indent << i << " : " << tmp->GetName() << endl;
        else
          os << indent << indent << i << " : " << tmp << endl;
        }
      }
    }

  if (this->MaterialTable)
    {
    this->MaterialTable->PrintSelf(os, indent.GetNextIndent());
    }

}

//----------------------------------------------------------------------------
void vtkboneFiniteElementModel::AppendHistory(const char* s)
{
  if (s == NULL) return;
  std::ostringstream newHistory;
  if (this->History != NULL)
    {
    size_t len = strlen(this->History);
    if (len > 0)
      {
      newHistory << this->History;
      if (this->History[len - 1] != '\n')
        { newHistory << "\n"; }
      }
    }
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
  newHistory << now << " " << s;
  this->SetHistory(newHistory.str().c_str());
}

//----------------------------------------------------------------------------
void vtkboneFiniteElementModel::AppendLog(const char* s)
{
  if (s == NULL) return;
  std::ostringstream newLog;
  if (this->Log != NULL)
    {
    size_t len = strlen(this->Log);
    if (len > 0)
      {
      newLog << this->Log;
      if (this->Log[len - 1] != '\n')
        { newLog << "\n"; }
      newLog << "\n";
      }
    }
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
  newLog << now << "\n" << s;
  this->SetLog(newLog.str().c_str());
}

//----------------------------------------------------------------------------
void vtkboneFiniteElementModel::AddNodeSet (vtkIdTypeArray *ids)
{
  if (ids->GetName() == NULL || strlen(ids->GetName()) == 0)
    {
    vtkErrorMacro(<<"Attempt to add NodeSet with no name.");
    return;
    }
  if (vtkIdTypeArray* existingNodeSet = this->GetNodeSet(ids->GetName()))
    {
    this->NodeSets->RemoveItem (existingNodeSet);
    }
  this->NodeSets->AddItem (ids);
}

//----------------------------------------------------------------------------
void vtkboneFiniteElementModel::AddElementSet (vtkIdTypeArray *ids)
{
  if (ids->GetName() == NULL || strlen(ids->GetName()) == 0)
    {
    vtkErrorMacro(<<"Attempt to add ElementSet with no name.");
    return;
    }
  if (vtkIdTypeArray* existingElementSet = this->GetElementSet(ids->GetName()))
    {
    this->ElementSets->RemoveItem (existingElementSet);
    }
  this->ElementSets->AddItem (ids);
}

//----------------------------------------------------------------------------
vtkIdTypeArray* vtkboneFiniteElementModel::GetNodeSet (const char* nodeSetName)
{
  vtkCollectionSimpleIterator cookie;
  this->NodeSets->InitTraversal(cookie);
  while (vtkDataArray* d = this->NodeSets->GetNextDataArray(cookie))
    {
    if (strcmp(d->GetName(),nodeSetName) == 0)
      {
      return vtkIdTypeArray::SafeDownCast(d);
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkIdTypeArray* vtkboneFiniteElementModel::GetElementSet (const char* elementSetName)
{
  vtkCollectionSimpleIterator cookie;
  this->ElementSets->InitTraversal(cookie);
  while (vtkDataArray* d = this->ElementSets->GetNextDataArray(cookie))
    {
    if (strcmp(d->GetName(),elementSetName) == 0)
      {
      return vtkIdTypeArray::SafeDownCast(d);
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::RemoveNodeSet (const char* nodeSetName)
{
  vtkIdTypeArray* nodeSet = this->GetNodeSet(nodeSetName);
  if (nodeSet)
    {
    this->NodeSets->RemoveItem(nodeSet);
    return VTK_OK;
    }
  else
    {
    return VTK_ERROR;
    }
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::RemoveElementSet (const char* elementSetName)
{
  vtkIdTypeArray* elementSet = this->GetElementSet(elementSetName);
  if (elementSet)
    {
    this->ElementSets->RemoveItem(elementSet);
    return VTK_OK;
    }
  else
    {
    return VTK_ERROR;
    }
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::GetAssociatedElementsFromNodeSet
(const char *nodeSetName, vtkIdTypeArray* ids)
{
  ids->Initialize();
  vtkIdTypeArray* nodeIds = this->GetNodeSet (nodeSetName);
  if (!nodeIds)
    {
    return VTK_ERROR;
    }
  ids->SetName (nodeIds->GetName());
  vtkSmartPointer<vtkSelection> selection = vtkSmartPointer<vtkSelection>::New();
  vtkboneSelectionUtilities::PointSelectionFromIds (selection, this, nodeIds);
  vtkboneSelectionUtilities::GetContainingCells (selection, this, ids);
  return VTK_OK;
}

//----------------------------------------------------------------------------
vtkIdTypeArray* vtkboneFiniteElementModel::GetAssociatedElementsFromNodeSet
(const char *nodeSetName)
{
  vtkIdTypeArray* ids = vtkIdTypeArray::New();
  int returnVal = this->GetAssociatedElementsFromNodeSet (nodeSetName, ids);
  if (returnVal != VTK_OK)
    {
    ids->Delete();
    return NULL;
    }
  return ids;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::DataSetFromNodeSet
(const vtkIdTypeArray *nodeSet, vtkUnstructuredGrid* data)
{
  vtkSmartPointer<vtkSelection> selection = vtkSmartPointer<vtkSelection>::New();
  vtkboneSelectionUtilities::PointSelectionFromIds (selection, this, nodeSet);
  vtkSmartPointer<vtkExtractSelection> extractor = vtkSmartPointer<vtkExtractSelection>::New();
  extractor->SetInputData (0, this);
  extractor->SetInputData (1, selection);
  extractor->Update();
  data->ShallowCopy (extractor->GetOutput());
  return VTK_OK;
}

//----------------------------------------------------------------------------
vtkUnstructuredGrid* vtkboneFiniteElementModel::DataSetFromNodeSet
(const vtkIdTypeArray *nodeSet)
{
  vtkUnstructuredGrid* data = vtkUnstructuredGrid::New();
  int returnVal = this->DataSetFromNodeSet (nodeSet, data);
  if (returnVal != VTK_OK)
    {
    data->Delete();
    return NULL;
    }
  return data;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::DataSetFromNodeSet
(const char *nodeSetName, vtkUnstructuredGrid* data)
{
  vtkIdTypeArray* nodeIds = this->GetNodeSet (nodeSetName);
  if (!nodeIds)
    {
    return VTK_ERROR;
    }
  return this->DataSetFromNodeSet (nodeIds, data);
}

//----------------------------------------------------------------------------
vtkUnstructuredGrid* vtkboneFiniteElementModel::DataSetFromNodeSet
(const char *nodeSetName)
{
  vtkUnstructuredGrid* data = vtkUnstructuredGrid::New();
  int returnVal = this->DataSetFromNodeSet (nodeSetName, data);
  if (returnVal != VTK_OK)
    {
    data->Delete();
    return NULL;
    }
  return data;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::DataSetFromElementSet
(const vtkIdTypeArray *elementSet, vtkUnstructuredGrid* data)
{
  vtkSmartPointer<vtkSelection> selection = vtkSmartPointer<vtkSelection>::New();
  vtkboneSelectionUtilities::CellSelectionFromIds (selection, this, elementSet);
  vtkSmartPointer<vtkExtractSelection> extractor = vtkSmartPointer<vtkExtractSelection>::New();
  extractor->SetInputData (0, this);
  extractor->SetInputData (1, selection);
  extractor->Update();
  data->ShallowCopy (extractor->GetOutput());
  return VTK_OK;
}

//----------------------------------------------------------------------------
vtkUnstructuredGrid* vtkboneFiniteElementModel::DataSetFromElementSet
(const vtkIdTypeArray *elementSet)
{
  vtkUnstructuredGrid* data = vtkUnstructuredGrid::New();
  int returnVal = this->DataSetFromElementSet (elementSet, data);
  if (returnVal != VTK_OK)
    {
    data->Delete();
    return NULL;
    }
  return data;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::DataSetFromElementSet
(const char *elementSetName, vtkUnstructuredGrid* data)
{
  vtkIdTypeArray* elementIds = this->GetElementSet (elementSetName);
  if (!elementIds)
    {
    return VTK_ERROR;
    }
  return this->DataSetFromElementSet (elementIds, data);
}

//----------------------------------------------------------------------------
vtkUnstructuredGrid* vtkboneFiniteElementModel::DataSetFromElementSet
(const char *elementSetName)
{
  vtkUnstructuredGrid* data = vtkUnstructuredGrid::New();
  int returnVal = this->DataSetFromElementSet (elementSetName, data);
  if (returnVal != VTK_OK)
    {
    data->Delete();
    return NULL;
    }
  return data;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::GetElementType()
{
  vtkSmartPointer<vtkCellTypes> cellTypes = vtkSmartPointer<vtkCellTypes>::New();
  this->GetCellTypes(cellTypes);
  if (cellTypes->GetNumberOfTypes() == 0)
    {
    return N88_UNKNOWN;
    }
  else if (cellTypes->GetNumberOfTypes() == 1)   // single type
    {
    if (cellTypes->IsType(VTK_TETRA))
      {
      return N88_TETRAHEDRON;
      }
    else if (cellTypes->IsType(VTK_HEXAHEDRON) || cellTypes->IsType(VTK_VOXEL))
      {
      return N88_HEXAHEDRON;
      }
    else
      {
      return N88_UNKNOWN;
      }
    }
  else if (cellTypes->GetNumberOfTypes() == 2)
    {
    if (cellTypes->IsType(VTK_HEXAHEDRON) && cellTypes->IsType(VTK_VOXEL))
      {
      return N88_HEXAHEDRON;
      }
    else
      {
      return N88_MIXED;
      }
    }
  else
    {
    return N88_MIXED;
    }
}


//----------------------------------------------------------------------------
void vtkboneFiniteElementModel::GetAllCellPoints (vtkIdTypeArray* allCellPoints)
{
  int types = this->GetElementType();
  vtkIdType nodesPerElement = 8; // default
  if (types == N88_TETRAHEDRON)
    {
    vtkIdType nodesPerElement = 4;
    }
  vtkIdType numCells = this->GetNumberOfCells();
  allCellPoints->Reset();
  allCellPoints->Allocate (numCells*nodesPerElement, numCells*nodesPerElement/4);
  vtkIdType npts = 0;
  vtkIdType* pts = NULL;
  for (vtkIdType cellId=0; cellId < numCells; ++cellId)
    {
    this->GetCellPoints(cellId, npts, pts);
    for (int i=0; i<npts; ++i)
      {
      allCellPoints->InsertNextValue(pts[i]);
      }
    }
}

//----------------------------------------------------------------------------

vtkIdTypeArray* vtkboneFiniteElementModel::GetAllCellPoints ()
{
  vtkIdTypeArray* allCellPoints = vtkIdTypeArray::New();
  this->GetAllCellPoints (allCellPoints);
  return allCellPoints;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ApplyBoundaryCondition
(
  vtkIdTypeArray* ids,
  vtkDataArray* senses,
  vtkDataArray* displacements,
  const char* constraintName
)
{
  if (!vtkFloatArray::SafeDownCast(displacements) &&
      !vtkDoubleArray::SafeDownCast(displacements))
    {
    vtkErrorMacro(<<"Displacements must be either type vtkFloatArray or vtkDoubleArray.");
    return VTK_ERROR;
    }

  vtkIdType N = ids->GetNumberOfTuples();
  if (senses->GetNumberOfTuples() != N ||
      displacements->GetNumberOfTuples() != N)
    {
    vtkErrorMacro(<<"ids/senses/displacements not all the same length.");
    return VTK_ERROR;
    }

  vtkSmartPointer<vtkboneConstraint> newConstraint =
      vtkSmartPointer<vtkboneConstraint>::Take(
        vtkboneConstraintUtilities::CreateBoundaryCondition(ids,senses,displacements,constraintName));

  if (this->GetConstraints()->GetItem(constraintName))
    {
    vtkboneConstraint* constraint = this->GetConstraints()->GetItem(constraintName);
    if ((constraint->GetConstraintType() != vtkboneConstraint::DISPLACEMENT) ||
        (constraint->GetConstraintAppliedTo() != vtkboneConstraint::NODES))
      {
      vtkErrorMacro(<<"Inconsistent constraint types.");
      return VTK_ERROR;
      }
    constraint->Merge(newConstraint);
    }
  else  // No existing constraint with this name.
    {
    this->GetConstraints()->AddItem(newConstraint);
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ApplyBoundaryCondition
(
  vtkIdTypeArray* ids,
  int sense,
  double displacement,
  const char* constraintName
)
{
  vtkIdType N = ids->GetNumberOfTuples();
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; i++)
    {
    senses->SetValue(i, sense);
    }
  vtkSmartPointer<vtkDoubleArray> displacements = vtkSmartPointer<vtkDoubleArray>::New();
  displacements->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; i++)
    {
    displacements->SetValue(i, displacement);
    }

  return this->ApplyBoundaryCondition(ids, senses, displacements, constraintName);
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ApplyBoundaryCondition
(
  vtkIdType id,
  int sense,
  double displacement,
  const char* constraintName
)
{
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetNumberOfValues(1);
  ids->SetValue(0, id);
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetNumberOfValues(1);
  senses->SetValue(0, sense);
  vtkSmartPointer<vtkDoubleArray> displacements = vtkSmartPointer<vtkDoubleArray>::New();
  displacements->SetNumberOfValues(1);
  displacements->SetValue(0, displacement);

  return this->ApplyBoundaryCondition(ids, senses, displacements, constraintName);
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ApplyBoundaryCondition
(
  const char* nodeSetName,
  int sense,
  double displacement,
  const char* constraintName
)
{
  vtkIdTypeArray* ids = this->GetNodeSet (nodeSetName);
  if (!ids)
    {
    vtkErrorMacro(<<"No such node set: " << nodeSetName);
    return VTK_ERROR;
    }
  return this->ApplyBoundaryCondition(ids, sense, displacement, constraintName);
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::FixNodes
(
  vtkIdTypeArray* ids,
  const char* constraintName
)
{
  vtkIdType N = ids->GetNumberOfTuples();
  // Used to be possible to set these: currently we set them all, since
  // that is nearly always the desired usage of FixNodes.
  const int xFlag = 1;
  const int yFlag = 1;
  const int zFlag = 1;
  int senseCount = (xFlag != 0) + (yFlag != 0) + (zFlag != 0);

  // Create new larger Ids array and add senses array
  vtkSmartPointer<vtkIdTypeArray> expandedIds = vtkSmartPointer<vtkIdTypeArray>::New();
  expandedIds->SetNumberOfValues(N*senseCount);
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetNumberOfValues(N*senseCount);
  vtkIdType j=0;
  for (vtkIdType i=0; i<N; i++)
    {
    vtkIdType id = ids->GetValue(i);
    if (xFlag)
      {
      expandedIds->SetValue(j, id);
      senses->SetValue(j, 0);
      j++;
      }
    if (yFlag)
      {
      expandedIds->SetValue(j, id);
      senses->SetValue(j, 1);
      j++;
      }
    if (zFlag)
      {
      expandedIds->SetValue(j, id);
      senses->SetValue(j, 2);
      j++;
      }
    }
  assert(j == N*senseCount);

  vtkSmartPointer<vtkDoubleArray> displacements = vtkSmartPointer<vtkDoubleArray>::New();
  displacements->SetNumberOfValues(senseCount*N);
  for (vtkIdType i=0; i<senseCount*N; i++)
    {
    displacements->SetValue(i, 0.0);
    }

  return this->ApplyBoundaryCondition(expandedIds, senses, displacements, constraintName);
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::FixNodes
(
  vtkIdType id,
  const char* constraintName
)
{
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetNumberOfValues(1);
  ids->SetValue(0, id);

  return this->FixNodes(ids, constraintName);
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::FixNodes
(
  const char* nodeSetName,
  const char* constraintName
)
{
  vtkIdTypeArray* ids = this->GetNodeSet (nodeSetName);
  if (!ids)
    {
    vtkErrorMacro(<<"No such node set: " << nodeSetName);
    return VTK_ERROR;
    }
  return this->FixNodes(ids, constraintName);
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ApplyLoad
(
  vtkIdTypeArray* ids,
  vtkDataArray* distributions,
  vtkDataArray* senses,
  vtkDataArray* forces,
  const char* constraintName
)
{
  if (!vtkFloatArray::SafeDownCast(forces) &&
      !vtkDoubleArray::SafeDownCast(forces))
    {
    vtkErrorMacro(<<"Forces must be either type vtkFloatArray or vtkDoubleArray.");
    return VTK_ERROR;
    }

  vtkIdType N = ids->GetNumberOfTuples();
  if (distributions->GetNumberOfTuples() != N ||
      senses->GetNumberOfTuples() != N ||
      forces->GetNumberOfTuples() != N)
    {
    vtkErrorMacro(<<"ids/distributions/senses/forces not all the same length.");
    return VTK_ERROR;
    }

  vtkSmartPointer<vtkboneConstraint> newConstraint =
      vtkSmartPointer<vtkboneConstraint>::Take(
        vtkboneConstraintUtilities::CreateAppliedLoad(ids,distributions,senses,forces,constraintName));

  if (this->GetConstraints()->GetItem(constraintName))
    {
    vtkboneConstraint* constraint = this->GetConstraints()->GetItem(constraintName);
    if ((constraint->GetConstraintType() != vtkboneConstraint::FORCE) ||
        (constraint->GetConstraintAppliedTo() != vtkboneConstraint::ELEMENTS))
      {
      vtkErrorMacro(<<"Inconsistent constraint types.");
      return VTK_ERROR;
      }
    constraint->Merge(newConstraint);
    }
  else  // No existing constraint with this name.
    {
    this->GetConstraints()->AddItem(newConstraint);
    }

  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ApplyLoad
  (
  vtkIdTypeArray* ids,
  int distribution,
  vtkDataArray* senses,
  vtkDataArray* forces,
  const char* constraintName
  )
  {
  vtkIdType N = ids->GetNumberOfTuples();

  vtkSmartPointer<vtkCharArray> distributions = vtkSmartPointer<vtkCharArray>::New();
  distributions->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; i++)
    {
    distributions->SetValue(i, distribution);
    }

  return this->ApplyLoad(ids, distributions, senses, forces, constraintName);
  }

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ApplyLoad
  (
  vtkIdTypeArray* ids,
  int distribution,
  int sense,
  double total_force,
  const char* constraintName
  )
  {
  vtkIdType N = ids->GetNumberOfTuples();

  vtkSmartPointer<vtkCharArray> distributions = vtkSmartPointer<vtkCharArray>::New();
  distributions->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; i++)
    {
    distributions->SetValue(i, distribution);
    }
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; i++)
    {
    senses->SetValue(i, sense);
    }
  double force = total_force / N;
  vtkSmartPointer<vtkDoubleArray> forces = vtkSmartPointer<vtkDoubleArray>::New();
  forces->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; i++)
    {
    forces->SetValue(i, force);
    }

  return this->ApplyLoad(ids, distributions, senses, forces, constraintName);
  }

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ApplyLoad
  (
  vtkIdType id,
  int distribution,
  int sense,
  double force,
  const char* constraintName
  )
  {
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetNumberOfValues(1);
  ids->SetValue(0, id);
  vtkSmartPointer<vtkCharArray> distributions = vtkSmartPointer<vtkCharArray>::New();
  distributions->SetNumberOfValues(1);
  distributions->SetValue(0, distribution);
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetNumberOfValues(1);
  senses->SetValue(0, sense);
  vtkSmartPointer<vtkDoubleArray> forces = vtkSmartPointer<vtkDoubleArray>::New();
  forces->SetNumberOfValues(1);
  forces->SetValue(0, force);

  return this->ApplyLoad(ids, distributions, senses, forces, constraintName);
  }

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ApplyLoad
  (
  const char* elementSetName,
  int distribution,
  int sense,
  double total_force,
  const char* constraintName
  )
  {
  vtkIdTypeArray* ids = this->GetElementSet (elementSetName);
  if (!ids)
    {
    vtkErrorMacro(<<"No such element set: " << elementSetName);
    return 0;
    }
  return this->ApplyLoad(ids, distribution, sense, total_force, constraintName);
  }

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ConvergenceSetFromConstraint(vtkboneConstraint* input_constraint)
  {
  // If constraint is applied to elements, distribute it to nodes.
  vtkSmartPointer<vtkboneConstraint> constraint;
  if (input_constraint->GetConstraintAppliedTo() == vtkboneConstraint::ELEMENTS)
    {
    constraint = vtkSmartPointer<vtkboneConstraint>::Take(
        vtkboneConstraintUtilities::DistributeConstraintToNodes(this, input_constraint));
    if (constraint.GetPointer() == NULL)
      {
      vtkErrorMacro (<< "Error converting to convergence set.");
      return VTK_ERROR;
      }
    constraint->SetConstraintType (input_constraint->GetConstraintType());
    }
  else
    {
    constraint = input_constraint;
    }

  vtkDataArray* values = constraint->GetAttributes()->GetArray("VALUE");
  if (!values)
    {
    vtkErrorMacro(<< "Constraint has no VALUES attribute.");
    return VTK_ERROR;
    }
  bool all_zero = true;
  for (vtkIdType i=0; i<values->GetNumberOfTuples(); ++i)
    {
    if (values->GetTuple1(i) != 0)
      {
      all_zero = false;
      break;
      }
    }
  if (all_zero)
    {
    vtkErrorMacro(<< "A constraint with all zero values cannot be specified as the convergence set.");
    return VTK_ERROR;
    }

  vtkSmartPointer<vtkboneConstraint> convergence_set = vtkSmartPointer<vtkboneConstraint>::New();
  convergence_set->DeepCopy (constraint);
  if (convergence_set->GetConstraintType() == vtkboneConstraint::FORCE)
    {
    convergence_set->SetConstraintType(vtkboneConstraint::DISPLACEMENT);
    }
  else
    {
    convergence_set->SetConstraintType(vtkboneConstraint::FORCE);
    }
  convergence_set->SetName("convergence_set");
  this->SetConvergenceSet (convergence_set);
  return VTK_OK;
  }


//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::ConvergenceSetFromConstraint(const char* constraintName)
  {
  return this->ConvergenceSetFromConstraint(
           this->GetConstraints()->GetItem(constraintName));
  }


//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::DataSetFromConstraint(
  vtkboneConstraint* constraint,
  vtkUnstructuredGrid* data)
{
  vtkSmartPointer<vtkSelection> selection = vtkSmartPointer<vtkSelection>::New();
  if (constraint->GetConstraintAppliedTo() == vtkboneConstraint::NODES)
    {
    this->DataSetFromNodeSet(constraint->GetIndices(), data);
    }
  else
    {
    this->DataSetFromElementSet(constraint->GetIndices(), data);
    }
  // Can copy all attribute arrays to Cell Data
  for (int i=0; i<constraint->GetAttributes()->GetNumberOfArrays(); i++)
    {
    data->GetCellData()->AddArray(constraint->GetAttributes()->GetArray(i));
    // If is a node constraint, then can also add the attribute arrays to the
    // Point Data.
    if (constraint->GetConstraintAppliedTo() == vtkboneConstraint::NODES)
      {
      data->GetPointData()->AddArray(constraint->GetAttributes()->GetArray(i));
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
vtkUnstructuredGrid* vtkboneFiniteElementModel::DataSetFromConstraint(
  vtkboneConstraint* constraint)
{
  vtkUnstructuredGrid* data = vtkUnstructuredGrid::New();
  this->DataSetFromConstraint(constraint, data);
  return data;
}

//----------------------------------------------------------------------------
int vtkboneFiniteElementModel::DataSetFromConstraint(
  const char *constraintName,
  vtkUnstructuredGrid* data)
{
  vtkboneConstraint* constraint = this->GetConstraints()->GetItem(constraintName);
  if (!constraint)
    {
    return 0;
    }
  return this->DataSetFromConstraint(constraint, data);
}

//----------------------------------------------------------------------------
vtkUnstructuredGrid* vtkboneFiniteElementModel::DataSetFromConstraint(
  const char *constraintName)
{
  vtkboneConstraint* constraint = this->GetConstraints()->GetItem(constraintName);
  if (!constraint)
    {
    return 0;
    }
  return this->DataSetFromConstraint(constraint);
}

//----------------------------------------------------------------------------
void vtkboneFiniteElementModel::ShallowCopy (vtkDataObject *dataObject)
{
  vtkboneFiniteElementModel *meshModel = vtkboneFiniteElementModel::SafeDownCast(dataObject);

  if (meshModel != NULL)
    {

    this->SetName (meshModel->GetName());
    this->SetHistory (meshModel->GetHistory());
    this->SetLog (meshModel->GetLog());

    if (this->NodeSets)
      {
      this->NodeSets->UnRegister(this);
      }
    this->NodeSets = meshModel->NodeSets;
    if (this->NodeSets)
      {
      this->NodeSets->Register(this);
      }

    if (this->ElementSets)
      {
      this->ElementSets->UnRegister(this);
      }
    this->ElementSets = meshModel->ElementSets;
    if (this->ElementSets)
      {
      this->ElementSets->Register(this);
      }

    if (this->Constraints)
      {
      this->Constraints->UnRegister(this);
      }
    this->Constraints = meshModel->Constraints;
    if (this->Constraints)
      {
      this->Constraints->Register(this);
      }

    if (this->MaterialTable)
      {
      this->MaterialTable->UnRegister(this);
      }
    this->MaterialTable = meshModel->MaterialTable;
    if (this->MaterialTable)
      {
      this->MaterialTable->Register(this);
      }

    if (this->ConvergenceSet)
      {
      this->ConvergenceSet->UnRegister(this);
      }
    this->ConvergenceSet = meshModel->ConvergenceSet;
    if (this->ConvergenceSet)
      {
      this->ConvergenceSet->Register(this);
      }

    if (this->GaussPointData)
      {
      this->GaussPointData->UnRegister(this);
      }
    this->GaussPointData = meshModel->GaussPointData;
    if (this->GaussPointData)
      {
      this->GaussPointData->Register(this);
      }

    }

  // Do superclass
  this->vtkUnstructuredGrid::ShallowCopy(dataObject);
}

//----------------------------------------------------------------------------
void vtkboneFiniteElementModel::DeepCopy (vtkDataObject *dataObject)
{
  cerr << "vtkboneFiniteElementModel::DeepCopy not implemented yet.\n";
  exit(1);
}
