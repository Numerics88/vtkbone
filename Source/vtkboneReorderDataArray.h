/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneReorderDataArray - provides methods for reordering an array
// in place.
//
// .SECTION Description
// This is typically used to apply an identical permutation for arrays
// associated with an array being sorted.  The reordering is done in-place.
// (Out of place reordering is simple.)
// For example, if we sort with
// @verbatim
//   vtkSortDaraArray::Sort (A, permutation)
// @endverbatim
// where initially permutation has values 0, 1, ... N-1, then we can apply
// the same permutation to other arrays like this:
// @verbatim
//   vtkReorderData::Reorder (B, permutation);
// @endverbatim
// Where several associated arrays need to be reordered, it is more
// efficient to call GenerateClosedLoopsList first and then to pass the
// resultant closedLoopList to each call of Reorder.


#ifndef __vtkboneReorderDataArray_h
#define __vtkboneReorderDataArray_h

#include "vtkObject.h"
#include "vtkboneWin32Header.h"
#include "vtkSetGet.h"

// Forward declarations
class vtkDataArray;
class vtkIdList;

class VTKBONE_EXPORT vtkboneReorderDataArray : public vtkObject
{
public:
  static vtkboneReorderDataArray* New();
  vtkTypeMacro(vtkboneReorderDataArray, vtkObject);

  // Description:
  // Reorder A according the given permutation.
  // permutation must be the same length as A and contain every integer from
  // 0 to N-1 exactly once.
  static int Reorder(vtkDataArray* A, vtkIdList* permutation);

  // Description:
  // Generate a closed loop list for the given permutation.
  // Once this is generated, subsequent calls to Reorder with the same
  // permutation can be performed optimally, with no additional memory
  // allocation.
  static int GenerateClosedLoopsList(vtkIdList* closedLoopList,
                                     vtkIdList* permutation);

  // Description:
  // Reorder A according the given permutation.
  // permutation must be the same length as A and contain every integer from
  // 0 to N-1 exactly once.
  // This method required that closedLoopList be previously generated by
  // a call to GenerateClosedLoopsList.
  static int Reorder(vtkDataArray* A,
                     vtkIdList* permutation,
                     vtkIdList* closedLoopList);

protected:
  vtkboneReorderDataArray();
  ~vtkboneReorderDataArray();

private:
  vtkboneReorderDataArray(const vtkboneReorderDataArray&);  // Not implemented.
  void operator=(const vtkboneReorderDataArray&);  // Not implemented.
};

#endif
