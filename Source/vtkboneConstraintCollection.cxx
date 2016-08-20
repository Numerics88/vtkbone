#include "vtkboneConstraintCollection.h"
#include "vtkboneConstraint.h"
#include "vtkObjectFactory.h"
#include <cstring>

vtkStandardNewMacro(vtkboneConstraintCollection);

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintCollection::GetItem(const char* name)
{
  for (int i=0; i < this->GetNumberOfItems(); i++)
  {
  vtkboneConstraint* constraint = this->GetItem(i);
  if (strcmp(constraint->GetName(), name) == 0)
    return constraint;
  }
  return NULL;
}
