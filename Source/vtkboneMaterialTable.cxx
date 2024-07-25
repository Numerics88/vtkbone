#include "vtkboneMaterialTable.h"
#include "vtkObjectFactory.h"
#include "vtkboneMaterial.h"

vtkStandardNewMacro(vtkboneMaterialTable);

//----------------------------------------------------------------------------
vtkboneMaterialTable::vtkboneMaterialTable()
  :
  current_material (NULL)
{
}

//----------------------------------------------------------------------------
vtkboneMaterialTable::~vtkboneMaterialTable()
{
  this->RemoveAll();
}

//----------------------------------------------------------------------------
void vtkboneMaterialTable::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Number of materials: " << this->GetNumberOfMaterials() << "\n";
  os << indent << "Maximum index: " << this->GetMaximumIndex() << "\n";
}

//----------------------------------------------------------------------------
int vtkboneMaterialTable::GetNumberOfMaterials()
{
  return this->materials.size();
}

//----------------------------------------------------------------------------
int vtkboneMaterialTable::GetMaximumIndex()
{
  if (this->materials.size() == 0)
  {
    return 0;
  }
  else
  {
    return this->materials.rbegin()->first;
  }
}

//----------------------------------------------------------------------------
void vtkboneMaterialTable::AddMaterial(int index, vtkboneMaterial* material)
{
  if (index < 1)
  {
    vtkErrorMacro (<< "Invalid Material ID: " << index);
    return;
  }
  this->RemoveMaterial(index);   // In order to UnRegister if required.
  this->materials[index] = material;
  material->Register(this);
}

//----------------------------------------------------------------------------
int vtkboneMaterialTable::AppendMaterial (vtkboneMaterial* material)
{
  int index = this->GetMaximumIndex() + 1;
  this->AddMaterial (index, material);
  return index;
}

//----------------------------------------------------------------------------
void vtkboneMaterialTable::RemoveMaterial(int index)
{
  if (this->materials.count(index))
  {
    this->materials[index]->UnRegister(this);
    materials.erase(index);
  }
}

//----------------------------------------------------------------------------
void vtkboneMaterialTable::RemoveAll()
{
  material_table_t::reverse_iterator m = this->materials.rbegin();
  while (m != this->materials.rend())
  {
    m->second->UnRegister(this);
    m++;
  }
  this->materials.clear();
}

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneMaterialTable::GetMaterial(int index)
{
  if (this->materials.count(index))
  {
    return this->materials[index];
  }
  else
  {
    return NULL;
  }
}

//----------------------------------------------------------------------------
void vtkboneMaterialTable::GetMaterialOrArray
  (
  int index,
  vtkboneMaterial*& material,
  int& offset
  )
{
  material_table_t::const_iterator it = this->materials.upper_bound(index);
  if (it == this->materials.begin())
  {
    material = NULL;
    return;
  }
  --it;  // Want the one before or the same.
  offset = index - it->first;
  material = it->second;
}

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneMaterialTable::GetMaterialOrArray
  (
  int index
  )
{
  vtkboneMaterial* material = NULL;
  this->GetMaterialOrArray (index, material, this->array_offset);
  return material;
}

//----------------------------------------------------------------------------
int vtkboneMaterialTable::GetArrayOffset ()
{
  return this->array_offset;
}

//----------------------------------------------------------------------------
int vtkboneMaterialTable::GetIndex (const char* name)
{
  for (material_table_t::iterator iter = this->materials.begin();
       iter != this->materials.end();
       ++iter)
  {
    vtkboneMaterial* material = iter->second;
    if (strcmp(material->GetName(), name) == 0)
    {
      return iter->first;
    }
  }
  return 0;
}

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneMaterialTable::GetMaterial(const char* name)
{
  for (material_table_t::iterator iter = this->materials.begin();
       iter != this->materials.end();
       ++iter)
  {
    vtkboneMaterial* material = iter->second;
    if (strcmp(material->GetName(), name) == 0)
    {
      return material;
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkboneMaterialTable::InitTraversal()
{
  this->traversal_iterator = this->materials.begin();
  this->visited_materials.clear();
}

//----------------------------------------------------------------------------
int vtkboneMaterialTable::GetNextIndex()
{
  if (this->traversal_iterator == this->materials.end())
  {
    this->current_material = NULL;
    return 0;
  }
  int index = this->traversal_iterator->first;
  this->current_material = this->traversal_iterator->second;
  ++traversal_iterator;
  return index;
}

//----------------------------------------------------------------------------
int vtkboneMaterialTable::GetNextUniqueIndex()
{
  int index = this->GetNextIndex();
  if (index == 0) {return 0;}
  while (this->visited_materials.count(this->current_material))
  {
    index = this->GetNextIndex();
    if (index == 0) {return 0;}
  }
  this->visited_materials.insert(this->current_material);
  return index;
}

//----------------------------------------------------------------------------
vtkboneMaterial* vtkboneMaterialTable::GetCurrentMaterial()
{
  return this->current_material;
}

//----------------------------------------------------------------------------
int vtkboneMaterialTable::CheckNames()
{
  // Map of names to object pointers.
  typedef std::map<std::string,vtkboneMaterial* > object_index_t;
  object_index_t object_index;
  for (material_table_t::iterator iter = this->materials.begin();
       iter != this->materials.end();
       ++iter)
  {
    vtkboneMaterial* material = iter->second;
    const char* name = material->GetName();
    if (strlen(name) == 0)
      { return 0; }
    if (object_index.count(name))
    {
      // Same name OK if this is actually the same material
      if (material != object_index[name])
        { return 0; }
    }
    object_index[material->GetName()] = material;
  }
  return 1;
}
