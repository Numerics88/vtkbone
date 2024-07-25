#include "vtkboneConstraintUtilities.h"
#include "vtkboneConstraintCollection.h"
#include "vtkboneConstraint.h"
#include "vtkUnstructuredGrid.h"
#include "vtkboneFiniteElementModel.h"
#include "vtkDataSetAttributes.h"
#include "vtkIdList.h"
#include "vtkCellType.h"
#include "vtkCharArray.h"
#include "vtkIdTypeArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"
#include <map>
#include <exception>
#include <cassert>


vtkStandardNewMacro (vtkboneConstraintUtilities);

//----------------------------------------------------------------------------
vtkboneConstraintUtilities::vtkboneConstraintUtilities()
{
}

//----------------------------------------------------------------------------
vtkboneConstraintUtilities::~vtkboneConstraintUtilities()
{
}

//----------------------------------------------------------------------------
void vtkboneConstraintUtilities::PrintSelf (ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::CreateBoundaryCondition
(
  vtkIdTypeArray* ids,
  vtkDataArray* senses,
  vtkDataArray* displacements,
  const char* name
)
{
  if (!vtkFloatArray::SafeDownCast(displacements) &&
      !vtkDoubleArray::SafeDownCast(displacements))
  {
    return NULL;
  }

  vtkIdType N = ids->GetNumberOfTuples();
  if (senses->GetNumberOfTuples() != N ||
      displacements->GetNumberOfTuples() != N)
  {
    return NULL;
  }

  vtkboneConstraint* constraint = vtkboneConstraint::New();
  constraint->SetName(name);
  // Make copy so that we don't possibly stomp on node sets.
  vtkSmartPointer<vtkIdTypeArray> ids_copy = vtkSmartPointer<vtkIdTypeArray>::New();
  ids_copy->DeepCopy(ids);
  constraint->SetIndices(ids_copy);
  constraint->SetConstraintType(vtkboneConstraint::DISPLACEMENT);
  constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
  senses->SetName("SENSE");
  displacements->SetName("VALUE");
  constraint->GetAttributes()->AddArray(senses);
  constraint->GetAttributes()->AddArray(displacements);

  return constraint;
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::CreateBoundaryCondition
(
  vtkIdTypeArray* ids,
  int sense,
  double displacement,
  const char* name
)
{
  vtkIdType N = ids->GetNumberOfTuples();
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; ++i)
  {
    senses->SetValue(i, sense);
  }
  vtkSmartPointer<vtkDoubleArray> displacements = vtkSmartPointer<vtkDoubleArray>::New();
  displacements->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; ++i)
  {
    displacements->SetValue(i, displacement);
  }

  return CreateBoundaryCondition(ids, senses, displacements, name);
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::CreateBoundaryCondition
(
  vtkIdType id,
  int sense,
  double displacement,
  const char* name
)
{
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetNumberOfValues(1);
  ids->SetValue(0, id);
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetNumberOfValues(1);
  senses->SetValue(0, sense);
  vtkSmartPointer<vtkDoubleArray> displacements = vtkSmartPointer<vtkDoubleArray>::New();
  displacements->SetNumberOfValues(1);
  displacements->SetValue(0, displacement);

  return CreateBoundaryCondition(ids, senses, displacements, name);
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::CreateFixedNodes
(
  vtkIdTypeArray* ids,
  const char* name
)
{
  vtkIdType N = ids->GetNumberOfTuples();
  // Used to be possible to set these: currently we set them all, since
  // that is nearly always the desired usage of CreateFixedNodes.
  const int xFlag = 1;
  const int yFlag = 1;
  const int zFlag = 1;
  int senseCount = (xFlag != 0) + (yFlag != 0) + (zFlag != 0);

  // Create new larger Ids array and add senses array
  vtkSmartPointer<vtkIdTypeArray> expandedIds = vtkSmartPointer<vtkIdTypeArray>::New();
  expandedIds->SetNumberOfValues(N*senseCount);
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetNumberOfValues(N*senseCount);
  vtkIdType j=0;
  for (vtkIdType i=0; i<N; ++i)
  {
    vtkIdType id = ids->GetValue(i);
    if (xFlag)
    {
      expandedIds->SetValue(j, id);
      senses->SetValue(j, 0);
      ++j;
    }
    if (yFlag)
    {
      expandedIds->SetValue(j, id);
      senses->SetValue(j, 1);
      ++j;
    }
    if (zFlag)
    {
      expandedIds->SetValue(j, id);
      senses->SetValue(j, 2);
      ++j;
    }
  }
  assert(j == N*senseCount);

  vtkSmartPointer<vtkDoubleArray> displacements = vtkSmartPointer<vtkDoubleArray>::New();
  displacements->SetNumberOfValues(senseCount*N);
  for (vtkIdType i=0; i<senseCount*N; ++i)
  {
    displacements->SetValue(i, 0.0);
  }

  return CreateBoundaryCondition(expandedIds, senses, displacements, name);
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::CreateFixedNodes
(
  vtkIdType id,
  const char* name
)
{
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetNumberOfValues(1);
  ids->SetValue(0, id);

  return CreateFixedNodes(ids, name);
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::CreateAppliedLoad
  (
  vtkIdTypeArray* ids,
  vtkDataArray* distributions,
  vtkDataArray* senses,
  vtkDataArray* forces,
  const char* name
  )
{
  if (!vtkFloatArray::SafeDownCast(forces) &&
      !vtkDoubleArray::SafeDownCast(forces))
  {
    return NULL;
  }

  vtkIdType N = ids->GetNumberOfTuples();
  if (distributions->GetNumberOfTuples() != N ||
      senses->GetNumberOfTuples() != N ||
      forces->GetNumberOfTuples() != N)
  {
    return NULL;
  }

  vtkboneConstraint* constraint = vtkboneConstraint::New();
  constraint->SetName(name);
  // Make copy so that we don't possibly stomp on element sets.
  vtkSmartPointer<vtkIdTypeArray> ids_copy = vtkSmartPointer<vtkIdTypeArray>::New();
  ids_copy->DeepCopy(ids);
  constraint->SetIndices(ids_copy);
  constraint->SetConstraintType(vtkboneConstraint::FORCE);
  constraint->SetConstraintAppliedTo(vtkboneConstraint::ELEMENTS);
  distributions->SetName("DISTRIBUTION");
  senses->SetName("SENSE");
  forces->SetName("VALUE");
  constraint->GetAttributes()->AddArray(distributions);
  constraint->GetAttributes()->AddArray(senses);
  constraint->GetAttributes()->AddArray(forces);

  return constraint;
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::CreateAppliedLoad
  (
  vtkIdTypeArray* ids,
  int distribution,
  vtkDataArray* senses,
  vtkDataArray* forces,
  const char* name
  )
{
  vtkIdType N = ids->GetNumberOfTuples();

  vtkSmartPointer<vtkCharArray> distributions = vtkSmartPointer<vtkCharArray>::New();
  distributions->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; ++i)
  {
    distributions->SetValue(i, distribution);
  }

  return CreateAppliedLoad(ids, distributions, senses, forces, name);
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::CreateAppliedLoad
  (
  vtkIdTypeArray* ids,
  int distribution,
  int sense,
  double force,
  const char* name
  )
{
  vtkIdType N = ids->GetNumberOfTuples();

  vtkSmartPointer<vtkCharArray> distributions = vtkSmartPointer<vtkCharArray>::New();
  distributions->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; ++i)
  {
    distributions->SetValue(i, distribution);
  }
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; ++i)
  {
    senses->SetValue(i, sense);
  }
  vtkSmartPointer<vtkDoubleArray> forces = vtkSmartPointer<vtkDoubleArray>::New();
  forces->SetNumberOfValues(N);
  for (vtkIdType i=0; i<N; ++i)
  {
    forces->SetValue(i, force);
  }

  return CreateAppliedLoad(ids, distributions, senses, forces, name);
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::CreateAppliedLoad
  (
  vtkIdType id,
  int distribution,
  int sense,
  double force,
  const char* name
  )
{
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetNumberOfValues(1);
  ids->SetValue(0, id);
  vtkSmartPointer<vtkCharArray> distributions = vtkSmartPointer<vtkCharArray>::New();
  distributions->SetNumberOfValues(1);
  distributions->SetValue(0, distribution);
  vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
  senses->SetNumberOfValues(1);
  senses->SetValue(0, sense);
  vtkSmartPointer<vtkDoubleArray> forces = vtkSmartPointer<vtkDoubleArray>::New();
  forces->SetNumberOfValues(1);
  forces->SetValue(0, force);

  return CreateAppliedLoad(ids, distributions, senses, forces, name);
}

//----------------------------------------------------------------------------
// This is some utility stuff needed by DistributeConstraintOnElementsToNodes.
namespace vtkboneConstraintUtilitiesHelper
{

  // We will want to index node constraints in a std::map object by
  // both Id and sense, so create a struct to hold these as one item.
  struct NodeKey
  {
    vtkIdType id;
    int sense;
    NodeKey(vtkIdType a, int b) : id(a), sense(b) {}
  };
  // Define an ordering for NodeKey.
  struct NodeKeyCompare
  {
    bool operator() (const NodeKey& lhs, const NodeKey& rhs) const
    {return lhs.id==rhs.id ? lhs.sense<rhs.sense : lhs.id<rhs.id;}
  };

  class vtkboneexception : public std::exception
  {
  public:
    vtkboneexception (const char* descr) throw()
      : description(descr) {}
    virtual ~vtkboneexception() throw() {}
    virtual const char* what() const throw()
    {
      return this->description.c_str();
    }
    std::string description;
  };

  typedef std::map<NodeKey,double,NodeKeyCompare> constraint_nodes_t;

  // Create a of list of constrained nodes.  We will step through the nodes
  // and add items to the list; later items will replace earlier ones.
  void GenerateDisplacementConstrainedNodeList
    (
    vtkboneConstraint* constraint,
    constraint_nodes_t& constrained_nodes
    )
  {
    vtkIdTypeArray* ids = constraint->GetIndices();
    vtkDataArray* senses = constraint->GetAttributes()->GetArray("SENSE");
    if (!senses)
      throw vtkboneexception("Missing SENSE array for constraint");
    vtkDataArray* values = constraint->GetAttributes()->GetArray("VALUE");
    if (!values)
      throw vtkboneexception("Missing VALUE array for constraint");
    vtkIdType N = constraint->GetNumberOfValues();
    if (ids->GetNumberOfTuples() != N ||
        senses->GetNumberOfTuples() != N ||
        values->GetNumberOfTuples() != N)
      throw vtkboneexception("Incorrectly sized attribute array for constraint.");

    std::vector<vtkIdType> node_list;
    for (vtkIdType i=0; i<N; ++i)
    {
      vtkIdType nodeId = ids->GetValue(i);
      int sense = senses->GetTuple1(i);
      NodeKey node_key(nodeId, sense);
      double val = values->GetTuple1(i);
      constrained_nodes[node_key] = val;
    }
  }

  // Create a of list of constrained nodes.  We will step through the nodes
  // and add items to the list; later items will replace earlier ones.
  void GenerateZeroValuedDisplacementConstrainedNodeList
    (
    vtkboneConstraint* constraint,
    constraint_nodes_t& constrained_nodes,
    double tol
    )
  {
    vtkIdTypeArray* ids = constraint->GetIndices();
    vtkDataArray* senses = constraint->GetAttributes()->GetArray("SENSE");
    if (!senses)
      throw vtkboneexception("Missing SENSE array for constraint");
    vtkDataArray* values = constraint->GetAttributes()->GetArray("VALUE");
    if (!values)
      throw vtkboneexception("Missing VALUE array for constraint");
    vtkIdType N = constraint->GetNumberOfValues();
    if (ids->GetNumberOfTuples() != N ||
        senses->GetNumberOfTuples() != N ||
        values->GetNumberOfTuples() != N)
      throw vtkboneexception("Incorrectly sized attribute array for constraint.");

    std::vector<vtkIdType> node_list;
    for (vtkIdType i=0; i<N; ++i)
    {
      vtkIdType nodeId = ids->GetValue(i);
      int sense = senses->GetTuple1(i);
      double val = values->GetTuple1(i);
      if (fabs(val) < tol)
      {
        NodeKey node_key(nodeId, sense);
        constrained_nodes[node_key] = val;
      }
    }
  }

  // Create a of list of constrained nodes.  We will step through the nodes
  // and add items to the list; later items will replace earlier ones.
  void GenerateNonzeroDisplacementConstrainedNodeList
    (
    vtkboneConstraint* constraint,
    constraint_nodes_t& constrained_nodes,
    double tol
    )
  {
    vtkIdTypeArray* ids = constraint->GetIndices();
    vtkDataArray* senses = constraint->GetAttributes()->GetArray("SENSE");
    if (!senses)
      throw vtkboneexception("Missing SENSE array for constraint");
    vtkDataArray* values = constraint->GetAttributes()->GetArray("VALUE");
    if (!values)
      throw vtkboneexception("Missing VALUE array for constraint");
    vtkIdType N = constraint->GetNumberOfValues();
    if (ids->GetNumberOfTuples() != N ||
        senses->GetNumberOfTuples() != N ||
        values->GetNumberOfTuples() != N)
      throw vtkboneexception("Incorrectly sized attribute array for constraint.");

    std::vector<vtkIdType> node_list;
    for (vtkIdType i=0; i<N; ++i)
    {
      vtkIdType nodeId = ids->GetValue(i);
      int sense = senses->GetTuple1(i);
      double val = values->GetTuple1(i);
      if (fabs(val) >= tol)
      {
        NodeKey node_key(nodeId, sense);
        constrained_nodes[node_key] = val;
      }
    }
  }

  // Create a of list of constrained nodes.  We will step through the nodes
  // and add items to the list, or add contributions to existing items
  // as required.
  void GenerateConstrainedNodeListFromNodes
    (
    vtkboneConstraint* constraint,
    constraint_nodes_t& constrained_nodes
    )
  {
    vtkIdTypeArray* ids = constraint->GetIndices();
    vtkDataArray* senses = constraint->GetAttributes()->GetArray("SENSE");
    if (!senses)
      throw vtkboneexception("Missing SENSE array for constraint");
    vtkDataArray* values = constraint->GetAttributes()->GetArray("VALUE");
    if (!values)
      throw vtkboneexception("Missing VALUE array for constraint");
    vtkIdType N = constraint->GetNumberOfValues();
    if (ids->GetNumberOfTuples() != N ||
        senses->GetNumberOfTuples() != N ||
        values->GetNumberOfTuples() != N)
      throw vtkboneexception("Incorrectly sized attribute array for constraint.");

    std::vector<vtkIdType> node_list;
    for (vtkIdType i=0; i<N; ++i)
    {
      vtkIdType nodeId = ids->GetValue(i);
      int sense = senses->GetTuple1(i);
      NodeKey node_key(nodeId, sense);
      double val = values->GetTuple1(i);
      if (constrained_nodes.count(node_key))
      {
        constrained_nodes[node_key] += val;
      }
      else
      {
        constrained_nodes[node_key] = val;
      }
    }
  }

  // Create a of list of constrained nodes.  We will step through the elements
  // and add items to the list, or add contributions to existing items
  // as required.
  void GenerateConstrainedNodeListFromElements
    (
    vtkUnstructuredGrid* geometry,
    vtkboneConstraint* constraint,
    constraint_nodes_t& constrained_nodes
    )
  {
    vtkIdTypeArray* ids = constraint->GetIndices();
    vtkDataArray* senses = constraint->GetAttributes()->GetArray("SENSE");
    if (!senses)
      throw vtkboneexception("Missing SENSE array for constraint on elements");
    vtkDataArray* distributions = constraint->GetAttributes()->GetArray("DISTRIBUTION");
    if (!distributions)
      throw vtkboneexception("Missing DISTRIBUTION array for constraint on elements");
    vtkDataArray* values = constraint->GetAttributes()->GetArray("VALUE");
    if (!values)
      throw vtkboneexception("Missing VALUE array for constraint on elements");
    vtkIdType N = constraint->GetNumberOfValues();
    if (ids->GetNumberOfTuples() != N ||
        senses->GetNumberOfTuples() != N ||
        distributions->GetNumberOfTuples() != N ||
        values->GetNumberOfTuples() != N)
      throw vtkboneexception("Incorrectly sized attribute array for constraint.");

    std::vector<vtkIdType> node_list;
    for (vtkIdType i=0; i<N; ++i)
    {
      vtkIdType cellId = ids->GetValue(i);
      if (geometry->GetCellType(cellId) != VTK_VOXEL)
        throw vtkboneexception("DistributeConstraintOnElementsToNodes only supports VTK_VOXEL type cells.");
      // Generate local node list for cell according to distribution
      node_list.clear();
      vtkSmartPointer<vtkIdList> cellPoints = vtkSmartPointer<vtkIdList>::New();
      geometry->GetCellPoints(cellId, cellPoints);
      switch((int)(distributions->GetTuple1(i)))
      {
        case vtkboneConstraint::FACE_X0_DISTRIBUTION:
          node_list.push_back(cellPoints->GetId(0));
          node_list.push_back(cellPoints->GetId(2));
          node_list.push_back(cellPoints->GetId(4));
          node_list.push_back(cellPoints->GetId(6));
          break;
        case vtkboneConstraint::FACE_X1_DISTRIBUTION:
          node_list.push_back(cellPoints->GetId(1));
          node_list.push_back(cellPoints->GetId(3));
          node_list.push_back(cellPoints->GetId(5));
          node_list.push_back(cellPoints->GetId(7));
          break;
        case vtkboneConstraint::FACE_Y0_DISTRIBUTION:
          node_list.push_back(cellPoints->GetId(0));
          node_list.push_back(cellPoints->GetId(1));
          node_list.push_back(cellPoints->GetId(4));
          node_list.push_back(cellPoints->GetId(5));
          break;
        case vtkboneConstraint::FACE_Y1_DISTRIBUTION:
          node_list.push_back(cellPoints->GetId(2));
          node_list.push_back(cellPoints->GetId(3));
          node_list.push_back(cellPoints->GetId(6));
          node_list.push_back(cellPoints->GetId(7));
          break;
        case vtkboneConstraint::FACE_Z0_DISTRIBUTION:
          node_list.push_back(cellPoints->GetId(0));
          node_list.push_back(cellPoints->GetId(1));
          node_list.push_back(cellPoints->GetId(2));
          node_list.push_back(cellPoints->GetId(3));
          break;
        case vtkboneConstraint::FACE_Z1_DISTRIBUTION:
          node_list.push_back(cellPoints->GetId(4));
          node_list.push_back(cellPoints->GetId(5));
          node_list.push_back(cellPoints->GetId(6));
          node_list.push_back(cellPoints->GetId(7));
          break;
        case vtkboneConstraint::BODY_DISTRIBUTION:
          node_list.push_back(cellPoints->GetId(0));
          node_list.push_back(cellPoints->GetId(1));
          node_list.push_back(cellPoints->GetId(2));
          node_list.push_back(cellPoints->GetId(3));
          node_list.push_back(cellPoints->GetId(4));
          node_list.push_back(cellPoints->GetId(5));
          node_list.push_back(cellPoints->GetId(6));
          node_list.push_back(cellPoints->GetId(7));
          break;
        default:
          throw vtkboneexception("Invalid DISTRIBUTION value.");
      }
      // Add each node in node_list to constrained_nodes.
      int nodes_in_cell = node_list.size();
      int sense = senses->GetTuple1(i);
      for (std::vector<vtkIdType>::const_iterator nodeId = node_list.begin();
           nodeId != node_list.end();
           ++nodeId)
      {
        NodeKey node_key(*nodeId, sense);
        double val = values->GetTuple1(i)/nodes_in_cell;
        if (constrained_nodes.count(node_key))
        {
          constrained_nodes[node_key] += val;
        }
        else
        {
          constrained_nodes[node_key] = val;
        }
      }
    }
  }

  void GenerateConstrainedNodeList
    (
    vtkUnstructuredGrid* geometry,
    vtkboneConstraint* constraint,
    constraint_nodes_t& constrained_nodes
    )
  {
    if (constraint->GetConstraintAppliedTo() == vtkboneConstraint::ELEMENTS)
    {
      GenerateConstrainedNodeListFromElements(geometry, constraint, constrained_nodes);
    }
    else
    {
      GenerateConstrainedNodeListFromNodes(constraint, constrained_nodes);
    }
  }

  vtkboneConstraint* ConvertConstrainedNodesListToConstraint
    (
    constraint_nodes_t& constrained_nodes,
    int type,
    const char* name
    )
  {
    size_t N = constrained_nodes.size();
    vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
    ids->SetNumberOfValues(N);
    vtkSmartPointer<vtkCharArray> senses = vtkSmartPointer<vtkCharArray>::New();
    senses->SetName("SENSE");
    senses->SetNumberOfValues(N);
    vtkSmartPointer<vtkDoubleArray> values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    values->SetNumberOfValues(N);
    vtkIdType i = 0;
    for (constraint_nodes_t::const_iterator node_key = constrained_nodes.begin();
         node_key != constrained_nodes.end();
         ++node_key)
    {
      ids->SetValue(i, node_key->first.id);
      senses->SetValue(i, node_key->first.sense);
      values->SetValue(i, node_key->second);
      ++i;
    }
    assert(i == N);
    vtkboneConstraint* constraint = vtkboneConstraint::New();
    constraint->SetName(name);
    constraint->SetIndices(ids);
    constraint->SetConstraintType(type);
    constraint->SetConstraintAppliedTo(vtkboneConstraint::NODES);
    constraint->GetAttributes()->AddArray(senses);
    constraint->GetAttributes()->AddArray(values);

    return constraint;
  }

}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::GatherDisplacementConstraints
  (
  vtkboneConstraint* constraint
  )
{
  using namespace vtkboneConstraintUtilitiesHelper;
  if (constraint->GetConstraintType() != vtkboneConstraint::DISPLACEMENT)
  {
    // Not a displacement constraint - Just return empty vtkboneConstraint
    return vtkboneConstraint::New();
  }

  constraint_nodes_t constrained_nodes;
  try
  {
    GenerateDisplacementConstrainedNodeList(constraint, constrained_nodes);
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  vtkboneConstraint* sortedConstraint = NULL;
  try
  {
    sortedConstraint = ConvertConstrainedNodesListToConstraint(
        constrained_nodes,
        vtkboneConstraint::DISPLACEMENT,
        "GATHERED DISPLACED NODES");
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  return sortedConstraint;
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::GatherDisplacementConstraints
  (
  vtkboneConstraintCollection* constraints
  )
{
  using namespace vtkboneConstraintUtilitiesHelper;

  constraint_nodes_t constrained_nodes;
  for (int i=0; i<constraints->GetNumberOfItems(); ++i)
  {
    vtkboneConstraint* constraint = constraints->GetItem(i);
    if (constraint->GetConstraintType() == vtkboneConstraint::DISPLACEMENT)
    {
      try
      {
        GenerateDisplacementConstrainedNodeList(constraint, constrained_nodes);
      }
      catch(std::exception& e)
      {
        return NULL;
      }
    }
  }

  vtkboneConstraint* sortedConstraint = NULL;
  try
  {
    sortedConstraint = ConvertConstrainedNodesListToConstraint(
        constrained_nodes,
        vtkboneConstraint::DISPLACEMENT,
        "GATHERED DISPLACED NODES");
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  return sortedConstraint;
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::GatherDisplacementConstraints
  (
  vtkboneFiniteElementModel* model
  )
{
  return GatherDisplacementConstraints(model->GetConstraints());
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::GatherZeroValuedDisplacementConstraints
  (
  vtkboneConstraint* constraint,
  double tol
  )
{
  using namespace vtkboneConstraintUtilitiesHelper;
  if (constraint->GetConstraintType() != vtkboneConstraint::DISPLACEMENT)
  {
    // Not a displacement constraint - Just return empty vtkboneConstraint
    return vtkboneConstraint::New();
  }

  constraint_nodes_t constrained_nodes;
  try
  {
    GenerateZeroValuedDisplacementConstrainedNodeList(constraint, constrained_nodes, tol);
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  vtkboneConstraint* sortedConstraint = NULL;
  try
  {
    sortedConstraint = ConvertConstrainedNodesListToConstraint(
        constrained_nodes,
        vtkboneConstraint::DISPLACEMENT,
        "GATHERED DISPLACED NODES");
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  return sortedConstraint;
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::GatherZeroValuedDisplacementConstraints
  (
  vtkboneConstraintCollection* constraints,
  double tol
  )
{
  using namespace vtkboneConstraintUtilitiesHelper;

  constraint_nodes_t constrained_nodes;
  for (int i=0; i<constraints->GetNumberOfItems(); ++i)
  {
    vtkboneConstraint* constraint = constraints->GetItem(i);
    if (constraint->GetConstraintType() == vtkboneConstraint::DISPLACEMENT)
    {
      try
      {
        GenerateZeroValuedDisplacementConstrainedNodeList(constraint, constrained_nodes, tol);
      }
      catch(std::exception& e)
      {
        return NULL;
      }
    }
  }

  vtkboneConstraint* sortedConstraint = NULL;
  try
  {
    sortedConstraint = ConvertConstrainedNodesListToConstraint(
        constrained_nodes,
        vtkboneConstraint::DISPLACEMENT,
        "GATHERED DISPLACED NODES");
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  return sortedConstraint;
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::GatherZeroValuedDisplacementConstraints
  (
  vtkboneFiniteElementModel* model,
  double tol
  )
{
  return GatherZeroValuedDisplacementConstraints(model->GetConstraints(), tol);
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::GatherNonzeroDisplacementConstraints
  (
  vtkboneConstraint* constraint,
  double tol
  )
{
  using namespace vtkboneConstraintUtilitiesHelper;
  if (constraint->GetConstraintType() != vtkboneConstraint::DISPLACEMENT)
  {
    // Not a displacement constraint - Just return empty vtkboneConstraint
    return vtkboneConstraint::New();
  }

  constraint_nodes_t constrained_nodes;
  try
  {
    GenerateNonzeroDisplacementConstrainedNodeList(constraint, constrained_nodes, tol);
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  vtkboneConstraint* sortedConstraint = NULL;
  try
  {
    sortedConstraint = ConvertConstrainedNodesListToConstraint(
        constrained_nodes,
        vtkboneConstraint::DISPLACEMENT,
        "GATHERED DISPLACED NODES");
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  return sortedConstraint;
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::GatherNonzeroDisplacementConstraints
  (
  vtkboneConstraintCollection* constraints,
  double tol
  )
{
  using namespace vtkboneConstraintUtilitiesHelper;

  constraint_nodes_t constrained_nodes;
  for (int i=0; i<constraints->GetNumberOfItems(); ++i)
  {
    vtkboneConstraint* constraint = constraints->GetItem(i);
    if (constraint->GetConstraintType() == vtkboneConstraint::DISPLACEMENT)
    {
      try
      {
        GenerateNonzeroDisplacementConstrainedNodeList(constraint, constrained_nodes, tol);
      }
      catch(std::exception& e)
      {
        return NULL;
      }
    }
  }

  vtkboneConstraint* sortedConstraint = NULL;
  try
  {
    sortedConstraint = ConvertConstrainedNodesListToConstraint(
        constrained_nodes,
        vtkboneConstraint::DISPLACEMENT,
        "GATHERED DISPLACED NODES");
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  return sortedConstraint;
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::GatherNonzeroDisplacementConstraints
  (
  vtkboneFiniteElementModel* model,
  double tol
  )
{
  return GatherNonzeroDisplacementConstraints(model->GetConstraints(), tol);
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::DistributeConstraintToNodes
  (
  vtkUnstructuredGrid* geometry,
  vtkboneConstraint* constraint
  )
{
  using namespace vtkboneConstraintUtilitiesHelper;

  constraint_nodes_t constrained_nodes;
  try
  {
    GenerateConstrainedNodeList(geometry, constraint, constrained_nodes);
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  vtkboneConstraint* constraintOnNodes = NULL;
  try
  {
    constraintOnNodes = ConvertConstrainedNodesListToConstraint(
        constrained_nodes,
        vtkboneConstraint::FORCE,
        "CONSTRAINT DISTRIBUTED TO NODES");
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  return constraintOnNodes;
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::DistributeForceConstraintsToNodes
  (
  vtkUnstructuredGrid* geometry,
  vtkboneConstraintCollection* constraints
  )
{
  using namespace vtkboneConstraintUtilitiesHelper;

  constraint_nodes_t constrained_nodes;
  for (int i=0; i<constraints->GetNumberOfItems(); ++i)
  {
    vtkboneConstraint* constraint = constraints->GetItem(i);
    if (constraint->GetConstraintType() == vtkboneConstraint::FORCE)
    {
      try
      {
        GenerateConstrainedNodeList(geometry, constraint, constrained_nodes);
      }
      catch(std::exception& e)
      {
        return NULL;
      }
    }
  }

  vtkboneConstraint* constraintOnNodes = NULL;
  try
  {
    constraintOnNodes = ConvertConstrainedNodesListToConstraint(
        constrained_nodes,
        vtkboneConstraint::FORCE,
        "FORCE CONSTRAINTS DISTRIBUTED TO NODES");
  }
  catch(std::exception& e)
  {
    return NULL;
  }

  return constraintOnNodes;
}

//----------------------------------------------------------------------------
vtkboneConstraint* vtkboneConstraintUtilities::DistributeForceConstraintsToNodes
  (
  vtkboneFiniteElementModel* model
  )
{
  return DistributeForceConstraintsToNodes(model, model->GetConstraints());
}
