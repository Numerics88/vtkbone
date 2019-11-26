#include "vtkboneReorderDataArray.h"
#include "vtkObjectFactory.h"
#include "vtkIdList.h"
#include "vtkSignedCharArray.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro (vtkboneReorderDataArray);

//----------------------------------------------------------------------------
vtkboneReorderDataArray::vtkboneReorderDataArray()
{
}

//----------------------------------------------------------------------------
vtkboneReorderDataArray::~vtkboneReorderDataArray()
{
}

//----------------------------------------------------------------------------
int vtkboneReorderDataArray::GenerateClosedLoopsList
(
  vtkIdList* closedLoopList,
  vtkIdList* permutation
)
{
  closedLoopList->Initialize();
  vtkIdType N = permutation->GetNumberOfIds();

#ifdef TRACE_VTKREORDERDATAARRAY
  cout << "permutation: ";
  for (int i=0; i<N; i++)
    {
    cout << permutation->GetId(i) << ", ";
    }
  cout << "\n";
#endif

  if (N==0) return 1;

  vtkSmartPointer<vtkSignedCharArray> used = vtkSmartPointer<vtkSignedCharArray>::New();
  used->SetNumberOfValues (N);
  for (int i=0; i<N; i++)
    {
    used->SetValue(i,0);
    }

  vtkIdType i=0;
  // Find first permutation entry that does not point to itself
  while (permutation->GetId(i) == i)
    {
#ifdef TRACE_VTKREORDERDATAARRAY
    cout << "Index " << i << " is stationary: ignoring\n";
#endif
    used->SetValue(i,1);
    i++;
    if (i==N) return 1;
    }
  closedLoopList->InsertNextId (i);
  vtkIdType startIndex = i;
  used->SetValue(i,1);
  vtkIdType count = i+1;
  if (count == N) return 1;

  // Follow the loops.
  while (1)
    {
    i = permutation->GetId(i);     // Pointer to next step in loop - go there.
    if (i == startIndex)
      {
      // Closed loop terminated - restart at unused position in permutation
#ifdef TRACE_VTKREORDERDATAARRAY
      cout << "Loop terminated at " << i << "\n";
#endif
      // Search for the next unused entry in permutation that does not point to itself
      while (1)
        {
        i++;
        if (i==N) i=0;    // circular index
        // If already used - just ignore it
        if (used->GetValue(i))
          {
            continue;
          }
        // If points to itself - mark as used and carry on looking
        if (permutation->GetId(i) == i)
          {
#ifdef TRACE_VTKREORDERDATAARRAY
          cout << "Index " << i << " is stationary: ignoring\n";
#endif
          used->SetValue(i,1);
          count++;
          if (count == N) return 1;
          continue;
          }
        // This i belongs to a new closed loop.
#ifdef TRACE_VTKREORDERDATAARRAY
        cout << "New closed loop element starting at " << i << "\n";
#endif
        closedLoopList->InsertNextId (i);
        startIndex=i;
        used->SetValue(i,1);
        count++;
        if (count == N) return 1;
        break;
        }
      }
    else
      {
#ifdef TRACE_VTKREORDERDATAARRAY
      cout << "Next position is " << i << "\n";
#endif
      used->SetValue(i,1);
      count++;
      if (count == N) return 1;
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkboneReorderDataArray::Reorder
(
  vtkDataArray* A,
  vtkIdList* permutation,
  vtkIdList* closedLoopList
)
{
  vtkIdType N = permutation->GetNumberOfIds();
  if (N==0) return 1;

  for (int loopIndex=0; loopIndex < closedLoopList->GetNumberOfIds(); loopIndex++)
    {

    // A progressive swap or reordering requires that we save one temporary value
    vtkIdType i = closedLoopList->GetId(loopIndex);
    vtkIdType tmpIndex = i;
    double tmpValue[9];  // 9 is the maximum possible tuple size
    A->GetTuple (i, tmpValue);
#ifdef TRACE_VTKREORDERDATAARRAY
    cout << "Setting tmp index to " << i << " and tmp value to " << *tmpValue << "\n";
#endif

    while (1)
      {
      vtkIdType j = permutation->GetId(i);
      if (j == tmpIndex)
        {
#ifdef TRACE_VTKREORDERDATAARRAY
        cout << "End of closed loop: Putting " << *tmpValue << " from tmp into position " << i << "\n";
#endif
        A->SetTuple (i,tmpValue);
        // Closed loop terminated - restart at unused position in permutation
        break;
        }
      else
        {
#ifdef TRACE_VTKREORDERDATAARRAY
        cout << "Putting " << *(A->GetTuple(j)) << " from position " << j
             << " into position " << i << "\n";
#endif
        A->SetTuple (i,j,A);
        i = j;
        }
      }
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkboneReorderDataArray::Reorder
(
  vtkDataArray* A,
  vtkIdList* permutation
)
{
  vtkSmartPointer<vtkIdList> closedLoopList = vtkSmartPointer<vtkIdList>::New();
  GenerateClosedLoopsList (closedLoopList, permutation);
  Reorder (A, permutation, closedLoopList);
  return 1;
}
