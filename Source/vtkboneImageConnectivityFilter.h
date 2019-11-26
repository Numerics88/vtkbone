/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneImageConnectivityFilter - mask regions of image data based on geometric connectivity
//
// .SECTION Description
//
// This filter uses vtkboneImageConnectivityMap to generate a mask based on
// connectivity of the scalar values of the image.  Specified components
// can then be zeroed out of the image.
//
// This filter accepts image scalars on either the cells or the points; the
// output will have scalars with the same association.
//

#ifndef __vtkboneImageConnectivityFilter_h
#define __vtkboneImageConnectivityFilter_h

#include "vtkSimpleImageToImageFilter.h"
#include "vtkboneWin32Header.h"

// forward declarations
class vtkUnsignedIntArray;
class vtkIdList;
class vtkIdTypeArray;

class VTKBONE_EXPORT vtkboneImageConnectivityFilter : public vtkSimpleImageToImageFilter
{
public:
  static vtkboneImageConnectivityFilter* New();
  vtkTypeMacro(vtkboneImageConnectivityFilter,
                       vtkSimpleImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // These match the defines in vtkConnectivityFilter
  enum ExtractionMode_t {
    EXTRACT_SEEDED_REGIONS = 2,
    EXTRACT_SPECIFIED_REGIONS = 3,
    EXTRACT_LARGEST_REGION = 4,
    EXTRACT_ALL_REGIONS = 5,
    EXTRACT_CLOSEST_POINT_REGION = 6,
    EXTRACT_REGIONS_OF_SPECIFIED_SIZE = 7
  };

  // Description:
  // Control the extraction of connected regions.
  // The default is EXTRACT_LARGEST_REGION.
  vtkSetClampMacro(ExtractionMode,int,
            EXTRACT_SEEDED_REGIONS,EXTRACT_REGIONS_OF_SPECIFIED_SIZE);
  vtkGetMacro(ExtractionMode,int);
  void SetExtractionModeToSeededRegions()
    {this->SetExtractionMode(EXTRACT_SEEDED_REGIONS);};
  void SetExtractionModeToLargestRegion()
    {this->SetExtractionMode(EXTRACT_LARGEST_REGION);};
  void SetExtractionModeToSpecifiedRegions()
    {this->SetExtractionMode(EXTRACT_SPECIFIED_REGIONS);};
  void SetExtractionModeToClosestPointRegion()
    {this->SetExtractionMode(EXTRACT_CLOSEST_POINT_REGION);};
  void SetExtractionModeToAllRegions()
    {this->SetExtractionMode(EXTRACT_ALL_REGIONS);};
  void SetExtractionModeToRegionsOfSpecifiedSize()
    {this->SetExtractionMode(EXTRACT_REGIONS_OF_SPECIFIED_SIZE);};
  const char *GetExtractionModeAsString();

  // Description:
  // Initialize list of point ids/cell ids used to seed regions.
  void InitializeSeedList();

  // Description:
  // Add a seed id (point or cell id). Note: ids are 0-offset.
  void AddSeed(vtkIdType id);

  // Description:
  // Delete a seed id (point or cell id). Note: ids are 0-offset.
  void DeleteSeed(vtkIdType id);

  // Description:
  // Initialize list of region ids to extract.
  void InitializeSpecifiedRegionList();

  // Description:
  // Add a region id to extract. Note: ids are 0-offset.
  void AddSpecifiedRegion(int id);

  // Description:
  // Delete a region id to extract. Note: ids are 0-offset.
  void DeleteSpecifiedRegion(int id);

  // Description:
  // Use to specify x-y-z point coordinates when extracting the region
  // closest to a specified point.
  vtkSetVector3Macro(ClosestPoint,double);
  vtkGetVectorMacro(ClosestPoint,double,3);

  // Description:
  // Use to specify the minimum region size when extracting the regions
  // by specific size.
  vtkSetMacro(MinimumRegionSize,vtkIdType);
  vtkGetMacro(MinimumRegionSize,vtkIdType);

  // Description:
  // Obtain the number of connected regions.
  vtkGetMacro(NumberOfExtractedRegions,unsigned int);

protected:
  vtkboneImageConnectivityFilter();
  ~vtkboneImageConnectivityFilter();

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);

  virtual void SimpleExecute(vtkImageData*, vtkImageData*);

  int CalculateRegionSizes(
    vtkUnsignedIntArray* cmap_data,
    unsigned int numRegions,
    vtkIdTypeArray* regionSizes);

  int ExtractionMode; //how to extract regions
  vtkIdList *Seeds; //id's of points or cells used to seed regions
  vtkIdList *SpecifiedRegionIds; //regions specified for extraction
  double ClosestPoint[3];
  vtkIdType MinimumRegionSize;

  unsigned int NumberOfExtractedRegions;

private:
  // Prevent compiler from making default versions of these.
  vtkboneImageConnectivityFilter(const vtkboneImageConnectivityFilter&);
  void operator=(const vtkboneImageConnectivityFilter&);
};

#endif
