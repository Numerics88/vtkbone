#include "vtkboneSelectionUtilities.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkExtractSelection.h"
#include "vtkConvertSelection.h"
#include "vtkDataSet.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkIdTypeArray.h"
#include "vtkIdList.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkImplicitFunction.h"
#include "vtkExtractGeometry.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro (vtkboneSelectionUtilities);

//----------------------------------------------------------------------------
void vtkboneSelectionUtilities::PointSelectionFromIds (
  vtkSelection* selection,
  const vtkDataSet* data,
  const vtkIdTypeArray* ids )
{
  vtkSmartPointer<vtkSelectionNode> selectionNode = vtkSmartPointer<vtkSelectionNode>::New();
  vtkInformation* info = selectionNode->GetProperties();
  vtkSelectionNode::SOURCE()->Set (info, const_cast<vtkDataSet*>(data));
  selectionNode->SetFieldType (vtkSelectionNode::POINT);
  selectionNode->SetContentType (vtkSelectionNode::INDICES);
  selectionNode->SetSelectionList (const_cast<vtkIdTypeArray*>(ids));
  selection->Initialize();
  selection->AddNode (selectionNode);
}

//----------------------------------------------------------------------------
void vtkboneSelectionUtilities::CellSelectionFromIds (
  vtkSelection* selection,
  const vtkDataSet* data,
  const vtkIdTypeArray* ids )
{
  vtkSmartPointer<vtkSelectionNode> selectionNode = vtkSmartPointer<vtkSelectionNode>::New();
  vtkInformation* info = selectionNode->GetProperties();
  vtkSelectionNode::SOURCE()->Set (info, const_cast<vtkDataSet*>(data));
  selectionNode->SetFieldType (vtkSelectionNode::CELL);
  selectionNode->SetContentType (vtkSelectionNode::INDICES);
  selectionNode->SetSelectionList (const_cast<vtkIdTypeArray*>(ids));
  selection->Initialize();
  selection->AddNode (selectionNode);
}

//----------------------------------------------------------------------------
void vtkboneSelectionUtilities::GetContainingCellsFromUnstructuredGrid
(
  vtkSelection* selection,
  vtkUnstructuredGrid* data,
  vtkIdTypeArray* cellIds
)
{
  // Build links so we can get cells associated with given points.
  data->BuildLinks();

  vtkIdType totalNumCells = data->GetNumberOfCells();

  // Get a list of points in the selection.
  vtkSmartPointer<vtkIdTypeArray> pointIds = vtkSmartPointer<vtkIdTypeArray>::New();
  vtkConvertSelection::GetSelectedPoints (selection, data, pointIds);
  vtkIdType numSelectedPoints = pointIds->GetNumberOfTuples();

  // Make a list of flags for each cell. We're going to walk through all
  // points in the selection and flag any cell associated with a point in
  // the selection.
  char* flagCellAsIncluded = new char[totalNumCells];
  for (int i=0; i < totalNumCells; i++)
    {
    flagCellAsIncluded[i] = 0;
    }
  
  vtkSmartPointer<vtkIdList> pointCellIds = vtkSmartPointer<vtkIdList>::New();
  for (vtkIdType i=0; i < numSelectedPoints; i++)
    {
    vtkIdType pointId = pointIds->GetValue(i);
    data->GetPointCells (pointId, pointCellIds);
    for (vtkIdType j=0; j < pointCellIds->GetNumberOfIds(); j++)
      {
      flagCellAsIncluded[pointCellIds->GetId(j)] = 1;
      }
    }

  // Count up the number of marked cells
  vtkIdType numSelectedCells = 0;
  for (int i=0; i < totalNumCells; i++)
    {
    if (flagCellAsIncluded[i])
      {
      numSelectedCells++;
      }
    }

  // Write the cellIds of the selected cells to cellIds.
  cellIds->Initialize();
  cellIds->SetNumberOfValues (numSelectedCells);
  vtkIdType j=0;
  for (int i=0; i < totalNumCells; i++)
    {
    if (flagCellAsIncluded[i])
      {
      cellIds->SetValue (j, i);
      j++;
      }
    }

  delete[] flagCellAsIncluded;
}

//----------------------------------------------------------------------------
void vtkboneSelectionUtilities::GetContainingCellsFromPolyData
(
  vtkSelection* selection,
  vtkPolyData* data,
  vtkIdTypeArray* cellIds
)
{
  // Build links so we can get cells associated with given points.
  data->BuildLinks();

  vtkIdType totalNumCells =  data->GetNumberOfCells();

  // Get a list of points in the selection.
  vtkSmartPointer<vtkIdTypeArray> pointIds = vtkSmartPointer<vtkIdTypeArray>::New();
  vtkConvertSelection::GetSelectedPoints (selection, data, pointIds);
  vtkIdType numSelectedPoints = pointIds->GetNumberOfTuples();

  // Make a list of flags for each cell. We're going to walk through all
  // points in the selection and flag any cell associated with a point in
  // the selection.
  char* flagCellAsIncluded = new char[totalNumCells];
  for (int i=0; i < totalNumCells; i++)
    {
    flagCellAsIncluded[i] = 0;
    }
  
  vtkSmartPointer<vtkIdList> pointCellIds = vtkSmartPointer<vtkIdList>::New();
  for (vtkIdType i=0; i < numSelectedPoints; i++)
    {
    vtkIdType pointId = pointIds->GetValue(i);
    unsigned short ncells;
    vtkIdType* pointCellIds;
    data->GetPointCells (pointId, ncells, pointCellIds);
    for (vtkIdType j=0; j<ncells; j++)
      {
      flagCellAsIncluded[pointCellIds[j]] = 1;
      }
    }

  // Count up the number of marked cells
  vtkIdType numSelectedCells = 0;
  for (int i=0; i < totalNumCells; i++)
    {
    if (flagCellAsIncluded[i])
      {
      numSelectedCells++;
      }
    }

  // Write the cellIds of the selected cells to cellIds.
  cellIds->Initialize();
  cellIds->SetNumberOfValues (numSelectedCells);
  vtkIdType j=0;
  for (int i=0; i < totalNumCells; i++)
    {
    if (flagCellAsIncluded[i])
      {
      cellIds->SetValue (j, i);
      j++;
      }
    }

  delete[] flagCellAsIncluded;
}

//----------------------------------------------------------------------------
void vtkboneSelectionUtilities::GetContainingCellsFromGeneric
(
  vtkSelection* selection,
  vtkDataSet* data,
  vtkIdTypeArray* cellIds
)
{
  // We make a shallow copy of the data.
  // This way, we won't inadvertently trigger an update of data.  This can
  // lead to infinite loops in certain cases, for example, when we are creating
  // a vtkboneSelectionUtilities object, which contains a Selection, and we want to change
  // the selection.
  vtkSmartPointer<vtkDataSet> dataCopy =
      vtkSmartPointer<vtkDataSet>::Take (data->NewInstance());
  dataCopy->ShallowCopy (data);

  vtkSmartPointer<vtkSelection> cellSelection = 
      vtkSmartPointer<vtkSelection>::Take (selection->NewInstance());
  ConvertToContainingCellsSelection (selection, dataCopy, cellSelection);

  // Make an extractor and run it.
  vtkSmartPointer<vtkExtractSelection> cellExtractor =
      vtkSmartPointer<vtkExtractSelection>::New();
  cellExtractor->SetInputData (0, dataCopy);
  cellExtractor->SetInputData (1, cellSelection);
  cellExtractor->Update();
  vtkDataSet* output = vtkDataSet::SafeDownCast (cellExtractor->GetOutput());
  if (output->GetCellData()->GetPedigreeIds())
    {
    cellIds->DeepCopy (output->GetCellData()->GetPedigreeIds());
    }
  else
    {
    // Shouldn't ever get here, because the selector should always attach
    // at least a PedigreeIds array.
    cerr << "Internal Error.  No PedigreeIds found\n";
    exit (-1);
    }
}

//----------------------------------------------------------------------------
void vtkboneSelectionUtilities::GetContainingCells
(
  vtkSelection* selection,
  vtkDataSet* data,
  vtkIdTypeArray* cellIds
)
{
  // Try fastest methods first
  if (vtkPolyData* dataAsPolyData = vtkPolyData::SafeDownCast (data))
    {
    GetContainingCellsFromPolyData (selection, dataAsPolyData, cellIds);    
    }
  else if (vtkUnstructuredGrid* dataAsUG = vtkUnstructuredGrid::SafeDownCast (data))
    {
    GetContainingCellsFromUnstructuredGrid (selection, dataAsUG, cellIds);    
    }
  else
    {
    GetContainingCellsFromGeneric (selection, data, cellIds);
    }
}

//----------------------------------------------------------------------------
void vtkboneSelectionUtilities::ConvertToContainingCellsSelection
(
  vtkSelection* selectionIn,
  vtkDataSet* data,
  vtkSelection* selectionOut
)
{
  // We make a shallow copy of the data (and to be certain, the selection object).
  // This way, we won't inadvertently trigger an update of data.  This can
  // lead to infinite loops in certain cases, for example, when we are creating
  // a vtkboneSelectionUtilities object, which contains a Selection, and we want to change
  // the selection.
  vtkSmartPointer<vtkDataSet> dataCopy =
      vtkSmartPointer<vtkDataSet>::Take (data->NewInstance());
  dataCopy->ShallowCopy (data);
  vtkSmartPointer<vtkSelection> selectionCopy = 
      vtkSmartPointer<vtkSelection>::Take (selectionIn->NewInstance());
  selectionCopy->ShallowCopy (selectionIn);

  // Get the points of the selection
  vtkSmartPointer<vtkIdTypeArray> pointIds = vtkSmartPointer<vtkIdTypeArray>::New();
  vtkConvertSelection::GetSelectedPoints (selectionCopy, dataCopy, pointIds);

  // Now make a new vtkSelectionNode that will select the containing cells
  vtkSmartPointer<vtkSelectionNode> selectionNode = vtkSmartPointer<vtkSelectionNode>::New();
  vtkInformation* info = selectionNode->GetProperties();
  selectionNode->SetFieldType (vtkSelectionNode::POINT);
  selectionNode->SetContentType (vtkSelectionNode::INDICES);
  vtkSelectionNode::CONTAINING_CELLS()->Set (info, 1);
  selectionNode->SetSelectionList (pointIds);

  // Stick the vtkSelectionNode into the output vtkSelection.
  selectionOut->Initialize();
  selectionOut->AddNode (selectionNode);
}

//----------------------------------------------------------------------------
void vtkboneSelectionUtilities::AddPointPedigreeIdsArray (vtkDataSet* data, int replace)
{
  vtkPointData* pData = data->GetPointData();
  // Check if already have PedigreeIds defined
  if (pData->GetPedigreeIds())
    {
    if (!replace)
      return;  // No nothing - exists already and we aren't replacing
    }

  vtkIdType N = data->GetNumberOfPoints();
  vtkSmartPointer<vtkIdTypeArray> idArray = vtkSmartPointer<vtkIdTypeArray>::New();
  // WARNING: If you name it "vtkOriginalPointIds", it will get clobbered
  // by vtkExtractSelection.
  idArray->SetName ("vtkboneOriginalPointIds");
  idArray->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; i++)
    {
    idArray->SetValue(i,i);
    }
  pData->SetPedigreeIds(idArray);
  pData->CopyPedigreeIdsOn();
}

//----------------------------------------------------------------------------
void vtkboneSelectionUtilities::AddCellPedigreeIdsArray (vtkDataSet* data, int replace)
{
  vtkCellData* cData = data->GetCellData();
  // Check if already have PedigreeIds defined
  if (cData->GetPedigreeIds())
    {
    if (!replace)
      return;  // No nothing - exists already and we aren't replacing
    }

  vtkIdType N = data->GetNumberOfCells();
  vtkSmartPointer<vtkIdTypeArray> idArray = vtkSmartPointer<vtkIdTypeArray>::New();
  // WARNING: If you name it "vtkOriginalCellIds", it will get clobbered
  // by vtkExtractSelection.
  idArray->SetName ("vtkboneOriginalCellIds");
  idArray->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; i++)
    {
    idArray->SetValue(i,i);
    }
  cData->SetPedigreeIds(idArray);
  cData->CopyPedigreeIdsOn();
}

//----------------------------------------------------------------------------
int vtkboneSelectionUtilities::ExtractPointsAsPolyData(
  vtkIdTypeArray* ids,
  vtkDataSet* data,
  vtkPolyData* out)
{
  // We make a shallow copy of the data.
  // This way, we won't inadvertently trigger an update of data.  This can
  // lead to infinite loops in certain cases, for example, when we are creating
  // a vtkboneSelectionUtilities object, which contains a Selection, and we want to change
  // the selection.
  vtkSmartPointer<vtkDataSet> dataCopy =
      vtkSmartPointer<vtkDataSet>::Take (data->NewInstance());
  dataCopy->ShallowCopy (data);

  // Now create points and verts and add to out.

  vtkIdType selectionCount = ids->GetNumberOfTuples();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->Allocate(selectionCount);
  vtkSmartPointer<vtkCellArray> verts = vtkSmartPointer<vtkCellArray>::New();
  verts->Allocate(selectionCount);
  
  for (vtkIdType i=0; i<selectionCount; i++)
    {
    vtkIdType id = ids->GetValue(i);
    points->InsertNextPoint(dataCopy->GetPoint(id));
    verts->InsertNextCell(1);
    verts->InsertCellPoint(i);
    }

  out->Initialize();
  out->SetPoints(points);
  out->SetVerts(verts);

  // Now copy all the arrays that belong to the points to BOTH the
  // output point and cell data (which are the same sizes).
  out->GetCellData()->CopyPedigreeIdsOn();
  out->GetPointData()->CopyPedigreeIdsOn();
  out->GetCellData()->CopyAllocate(dataCopy->GetCellData(), selectionCount);
  out->GetPointData()->CopyAllocate(dataCopy->GetCellData(), selectionCount);
  for (vtkIdType i=0; i<selectionCount; i++)
    {
    vtkIdType id = ids->GetValue(i);
    out->GetCellData()->CopyData(dataCopy->GetCellData(), id, i);
    out->GetPointData()->CopyData(dataCopy->GetCellData(), id, i);
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneSelectionUtilities::ExtractPointsAsPolyData(
  vtkSelection* selection,
  vtkDataSet* data,
  vtkPolyData* out)
{
  if (selection->GetNumberOfNodes() != 1)
    {
    // Right now can only deal with exactly one selection node.
    return 0;  
    }
  
  vtkSelectionNode* selectionNode = selection->GetNode(0);
  
  if (selectionNode->GetContentType() != vtkSelectionNode::INDICES)
    {
    // Right now can only deal with selection by indices.
    return 0;
    }
  if (selectionNode->GetFieldType() != vtkSelectionNode::POINT)
    {
    // Right now can only deal with point selection.
    return 0;
    }
  
  vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast(selectionNode->GetSelectionList());
  if (!ids)
    {
    // Right now can only deal with id list as vtkIdTypeArray
    return 0;
    }

  return vtkboneSelectionUtilities::ExtractPointsAsPolyData(ids, data, out);
}
