#include "vtkboneVersion.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkbone_version.h"

vtkStandardNewMacro(vtkboneVersion);

//----------------------------------------------------------------------------
const char* vtkboneVersion::GetVTKBONEVersion()
{
  return VTKBONE_VERSION;
}

//----------------------------------------------------------------------------
const char* vtkboneVersion::GetvtkboneVersion()
{
  return VTKBONE_VERSION;
}

//----------------------------------------------------------------------------
int vtkboneVersion::GetVTKBONEMajorVersion()
{
  return VTKBONE_VERSION_MAJOR;
}

//----------------------------------------------------------------------------
int vtkboneVersion::GetvtkboneMajorVersion()
{
  return VTKBONE_VERSION_MAJOR;
}

//----------------------------------------------------------------------------
int vtkboneVersion::GetVTKBONEMinorVersion()
{
  return VTKBONE_VERSION_MINOR;
}

//----------------------------------------------------------------------------
int vtkboneVersion::GetvtkboneMinorVersion()
{
  return VTKBONE_VERSION_MINOR;
}

//----------------------------------------------------------------------------
int vtkboneVersion::GetVTKBONEMinorMinorVersion()
{
#ifdef VTKBONE_VERSION_MINOR_MINOR
  return VTKBONE_VERSION_MINOR_MINOR;
#else
  return 0;
#endif
}

//----------------------------------------------------------------------------
int vtkboneVersion::GetvtkboneMinorMinorVersion()
{
#ifdef VTKBONE_VERSION_MINOR_MINOR
  return VTKBONE_VERSION_MINOR_MINOR;
#else
  return 0;
#endif
}
