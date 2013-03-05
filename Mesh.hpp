//============================================================================
// Name        : Mesh.hpp
// Author      : George Rokos
// Description : Mesh description
//============================================================================

#ifndef MESH_HPP_
#define MESH_HPP_


#include <cstddef>
#include <set>
#include <vector>

#include <driver_types.h>

struct Quality{
  float mean;
  float min;
  float rms;
};

class Mesh{
public:
  // Constructor
  Mesh(const char *filename);
  ~Mesh();  
  void pin_data();

  size_t NNodes;    // Number of mesh vertices.
  size_t NElements; // Number of mesh elements.

  void* pinned_data; // ENList, coords, metric and normals
  size_t total_size; // Size of pinned data

  // Element eid is comprised of the vertices
  // ENList[3*eid], ENList[3*eid+1] and ENList[3*eid+2].
  std::vector<size_t> ENList;
  size_t* ENList_pinned;
  size_t ENList_bytes;

  // Vertex vid has coordinates x=coords[2*vid] and y=coords[2*vid+1].
  std::vector<float> coords;
  float* coords_pinned;
  size_t coords_bytes;
  
  // The metric tensor at vertex vid is M_00 = metric[3*vid],
  //                                    M_01 = M_10 = metric[3*vid+1] and
  //                                    M_11 = metric[3*vid+2].
  std::vector<float> metric;
  float* metric_pinned;
  size_t metric_bytes;

  /* If vid is on the surface, the normal vector
   * (normals[2*vid],normals[2*vid+1] =
   *                            = (0.0,1.0) if vid is on the top surface
   *                            = (0.0,-1.0) if vid is on the bottom surface
   *                            = (1.0,0.0) if vid is on the right surface
   *                            = (-1.0,0.0) if vid is on the left surface
   * For all other vertices, the normal vector is (0.0,0.0).
   */
  std::vector<float> normals;
  float* normals_pinned;
  size_t normals_bytes;

  // For every vertex i, NNList[i] contains the IDs of all adjacent vertices.
  std::vector< std::vector<size_t> > NNList;
  size_t* NNListArray_pinned;
  size_t* NNListIndex_pinned;
  size_t NNListArray_size;
  size_t NNListArray_bytes;
  size_t NNListIndex_bytes;

  // For every vertex i, NEList[i] contains the IDs of all adjacent elements.
  std::vector< std::set<size_t> > NEList;
  size_t* NEListArray_pinned;
  size_t* NEListIndex_pinned;
  size_t NEListArray_size;
  size_t NEListArray_bytes;
  size_t NEListIndex_bytes;

  bool isCornerNode(size_t vid) const;

  float element_area(size_t eid) const;
  float element_quality(size_t eid) const;
  Quality get_mesh_quality() const;

  int orientation;

private:
  void* NNListToArray(void* ptr);
  void* NEListToArray(void* ptr);
  void setNNListSize();
  void setNEListSize();
  void cuda_check(cudaError_t success);
  void create_adjacency();
  void find_surface();
  void set_orientation();
};

#endif /* MESH_HPP_ */
