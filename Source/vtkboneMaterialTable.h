/*=========================================================================

  Copyright 2010-2016, Numerics88 Solutions Ltd.
  http://www.numerics88.com/

  Copyright (c) Eric Nodwell and Steven K. Boyd
  See Copyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

// .NAME vtkboneMaterialTable - Material Table finite element mesh
//
// .SECTION Description
//  Stores the material table for a vtkboneFiniteElementModel.
//  The material table consists of any number of vtkboneMaterial objects.
//  vtkboneMaterial is an abstract class that has concrete derived
//  classes of various types (e.g. vtkboneLinearIsotropicMaterial and
//  vtkboneOrthotropicMaterial).  Each vtkboneMaterial object is stored indexed
//  with an integer value (these will typically be associated with the
//  segmentation values).  It is not required to use consecutive index
//  values.  Furthermore, materials can be added and removed from the table
//  in any order.
//
// .SECTION See Also
// vtkboneMaterial vtkboneLinearIsotropicMaterial vtkboneFiniteElementModel
// vtkboneGenerateHomogeneousLinearIsotropicMaterialTable
// vtkboneGenerateHommingaMaterialTable

#ifndef __vtkboneMaterialTable_h
#define __vtkboneMaterialTable_h

#include "vtkDataObject.h"
#include "vtkboneWin32Header.h"
#include <map>
#include <set>

// Forward declarations
class vtkboneMaterial;
class vtkboneMaterialArray;

class VTKBONE_EXPORT vtkboneMaterialTable : public vtkDataObject
{
  public:
    static vtkboneMaterialTable* New();
    vtkTypeMacro(vtkboneMaterialTable, vtkDataObject);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    // Description:
    // Get the number of materials.
    int GetNumberOfMaterials();

    // Description:
    // Get the maximum index.
    int GetMaximumIndex();

    // Description:
    // Set an entry in the table.
    // If the entry exists already, will replace it.
    void AddMaterial(int index, vtkboneMaterial* material);
    // The sole reason for the following overloaded method is that the Python
    // wrapping in VTK (at least for version 6.3) fails to identify that subclasses
    // of vtkboneMaterialArray are also subclasses of vtkboneMaterial.
    void AddMaterial(int index, vtkboneMaterialArray* material)
      { this->AddMaterial(index,(vtkboneMaterial*)material); }

    // Description:
    // Adds a material to the table.  An unique index will be assigned to
    // the material.
    // Returns the assigned index.
    int AppendMaterial(vtkboneMaterial* material);

    // Description:
    // Removes an entry in the table.
    // If the entry does not exist, silently does nothing.
    void RemoveMaterial(int index);

    // Description:
    // Clears the material table.
    void RemoveAll();

    // Description:
    // Get an the index of a material.
    // Returns 0 if no such entry exists.
    int GetIndex(const char* name);

    // Description:
    // Get an entry.
    // Returns NULL if no such entry exists.
    vtkboneMaterial* GetMaterial(int index);
    vtkboneMaterial* GetMaterial(const char* name);

    // Description:
    // This returns the material at index if it exists, or else material
    // with the largest index less than specified, together with
    // the difference in indices (the offset). In the case of a non-zero
    // offset, it is your responsibility to check (via a dynamic cast),
    // that the returned material is in fact a material array.
    void GetMaterialOrArray(
        int index,
        vtkboneMaterial*& material,
        int& offset);

    // Description:
    // This returns the material at index if it exists, or else material
    // with the largest index less than specified, together with
    // the difference in indices, obtained by an immediately following
    // call to GetArrayOffset. In this case it is your responsibility
    // to check (via a dynamic cast), that the returned material is
    // in fact a material array if offset in non-zero. This version
    // is for use in Python.
    vtkboneMaterial* GetMaterialOrArray(int index);
    int GetArrayOffset ();

    // Description:
    // Initialize the traversal of the table. This means the data pointer is
    // set at the beginning of the list.
    // This method is not thread-safe.
    void InitTraversal();

    // Description:
    // Get the next item in the table. NULL is returned if the table
    // is exhausted.
    // Note that immediately after a call to InitTraversal, this returns the
    // first item in the list.
    // This method is not thread-safe.
    int GetNextIndex();

    // Description:
    // Get the next item in the table, that has not already been returned.
    // NULL is returned if the table is exhausted.
    // Note that immediately after a call to InitTraversal, this returns the
    // first item in the list.
    // Note that you cannot mix calls to GetNextIndex with calls to
    // GetNextUniqueIndex: the results are undetermined.
    // This method is not thread-safe.
    int GetNextUniqueIndex();

    // Description:
    // Get the current material in the traversal.
    // This method is only valid after a call to GetNextIndex()!
    vtkboneMaterial* GetCurrentMaterial();

    // Description:
    // Verifies that all the materials have unique names.
    // Returns 1 if all materials have unique names, 0 otherwise.
    // Also returns 0 if any material does not have a name.
    int CheckNames();

  protected:
    vtkboneMaterialTable();
    ~vtkboneMaterialTable();

    //BTX
    typedef std::map<int,vtkboneMaterial*,std::less<int> > material_table_t;
    material_table_t materials;
    material_table_t::iterator traversal_iterator;
    std::set<vtkboneMaterial*> visited_materials;
    vtkboneMaterial* current_material;
    //ETX

    int array_offset;

  private:
    vtkboneMaterialTable(const vtkboneMaterialTable&);  // Not implemented.
    void operator=(const vtkboneMaterialTable&);  // Not implemented.
};

#endif
