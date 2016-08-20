#include "vtkboneMaterial.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "boost/format.hpp"

vtkStandardNewMacro (vtkboneMaterial);


// Initialize static variables.
int vtkboneMaterial::nameCounter = 0;


//----------------------------------------------------------------------------
vtkboneMaterial::vtkboneMaterial()
:
  Name                  (NULL)
  {
  using boost::format;
  ++nameCounter;
  this->SetName((format("NewMaterial%d") % nameCounter).str().c_str());
  }

//----------------------------------------------------------------------------
vtkboneMaterial::~vtkboneMaterial()
  {
  this->SetName(NULL);
  }

//----------------------------------------------------------------------------
void vtkboneMaterial::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Name: " << this->Name << "\n";
  }

//----------------------------------------------------------------------------
// Actually, this function should really be pure virtual without an
// implementation, but pure virtual classes can't compile in VTK.
vtkboneMaterial* vtkboneMaterial::Copy()
  {
  vtkboneMaterial* new_mat = vtkboneMaterial::New();
  new_mat->SetName(this->Name);
  return new_mat;
  }

//----------------------------------------------------------------------------
// Actually, this function should really be pure virtual without an
// implementation, but pure virtual classes can't compile in VTK.
vtkboneMaterial* vtkboneMaterial::ScaledCopy(double factor)
  {
  vtkboneMaterial* new_mat = vtkboneMaterial::New();
  new_mat->SetName(this->Name);
  return new_mat;
  }
