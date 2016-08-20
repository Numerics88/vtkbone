#include "vtkbonePrettyReports.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkImageData.h"
#include "vtkMassProperties.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkSmartPointer.h"
#include <boost/format.hpp>

using boost::format;

namespace vtkbonePrettyReports
{

  //-----------------------------------------------------------------------
  void Print (std::ostream& s, vtkMassProperties* obj)
  {
    s << "!-----------------------------------------------------------------------------\n";
    s << (format(" %-30s\n") % "vtkMassProperties").str();
    s << "\n";
    s << (format("   %-30s%-12.4e\n") % "Volume" % obj->GetVolume()).str();
    s << (format("   %-30s%-12.4e\n") % "SurfaceArea" % obj->GetSurfaceArea()).str();
    s << (format("   %-30s%-12.4e%-12.4e%-12.4e\n") % "VolumeX, VolumeY, VolumeZ" % obj->GetVolumeX() % obj->GetVolumeY() % obj->GetVolumeZ()).str();
    s << (format("   %-30s%-12.4e%-12.4e%-12.4e\n") % "Kx, Ky, Kz" % obj->GetKx() % obj->GetKy() % obj->GetKz()).str();
    s << (format("   %-30s%-12.4e\n") % "NormalizedShapeIndex" % obj->GetNormalizedShapeIndex()).str();
    s << "\n";
    s << (format("   %-30s\n") % "Units are usually mm").str();
    s << "\n";
  }

}  // namespace vtkbonePrettyReports
