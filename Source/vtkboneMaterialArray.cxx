#include "vtkboneMaterialArray.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro (vtkboneMaterialArray);


//----------------------------------------------------------------------------
vtkboneMaterialArray::vtkboneMaterialArray()
{
}

//----------------------------------------------------------------------------
vtkboneMaterialArray::~vtkboneMaterialArray()
{
}

//----------------------------------------------------------------------------
void vtkboneMaterialArray::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkboneMaterialArray::Resize (vtkIdType size)
{
  vtkErrorMacro(<< "vtkboneMaterialArray::Resize must be reimplemented by subclasses");
}

//----------------------------------------------------------------------------
vtkIdType vtkboneMaterialArray::GetSize ()
{
  vtkErrorMacro(<< "vtkboneMaterialArray::GetSize must be reimplemented by subclasses");
  return 0;
}
