//============================================================================
// Name        : Mesh.hpp
// Author      : George Rokos
// Description : Mesh description
//============================================================================

#ifndef MESH_HPP_
#define MESH_HPP_

#ifdef __CUDAC__
#define HOST __host__
#define DEVICE __device__
#else
#define HOST
#define DEVICE
#endif


#include <cstddef>
#include <set>
#include <vector>

struct Quality{
  double mean;
  double min;
  double rms;
};

class Mesh{
public:
  // Constructor
  Mesh(const char *filename);

  size_t NNodes;    // Number of mesh vertices.
  size_t NElements; // Number of mesh elements.

  // Element eid is comprised of the vertices
  // ENList[3*eid], ENList[3*eid+1] and ENList[3*eid+2].
  std::vector<size_t> ENList;

  // Vertex vid has coordinates x=coords[2*vid] and y=coords[2*vid+1].
  std::vector<double> coords;

  // The metric tensor at vertex vid is M_00 = metric[3*vid],
  //                                    M_01 = M_10 = metric[3*vid+1] and
  //                                    M_11 = metric[3*vid+2].
  std::vector<double> metric;

  /* If vid is on the surface, the normal vector
   * (normals[2*vid],normals[2*vid+1] =
   *                            = (0.0,1.0) if vid is on the top surface
   *                            = (0.0,-1.0) if vid is on the bottom surface
   *                            = (1.0,0.0) if vid is on the right surface
   *                            = (-1.0,0.0) if vid is on the left surface
   * For all other vertices, the normal vector is (0.0,0.0).
   */
  std::vector<double> normals;

  // For every vertex i, NNList[i] contains the IDs of all adjacent vertices.
  std::vector< std::vector<size_t> > NNList;

  // For every vertex i, NEList[i] contains the IDs of all adjacent elements.
  std::vector< std::set<size_t> > NEList;

  inline __device__ bool isSurfaceNode(size_t vid) const{
    return NEList[vid].size() < NNList[vid].size();
  }

  inline __device__ bool isCornerNode(size_t vid) const{
    return fabs(normals[2*vid])==1.0 && fabs(normals[2*vid+1]==1.0);
  }

  double element_area(size_t eid) const;
  
  inline __device__ double element_quality(size_t eid) const{
	  const size_t *n = &ENList[3*eid];

	  // Pointers to the coordinates of each vertex
	  const double *c0 = &coords[2*n[0]];
	  const double *c1 = &coords[2*n[1]];
	  const double *c2 = &coords[2*n[2]];

	  // Pointers to the metric tensor at each vertex
	  const double *m0 = &metric[3*n[0]];
	  const double *m1 = &metric[3*n[1]];
	  const double *m2 = &metric[3*n[2]];

	  // Metric tensor averaged over the element
	  double m00 = (m0[0] + m1[0] + m2[0])/3;
	  double m01 = (m0[1] + m1[1] + m2[1])/3;
	  double m11 = (m0[2] + m1[2] + m2[2])/3;

	  // l is the length of the perimeter, measured in metric space
	  double l =
	    sqrt((c0[1] - c1[1])*((c0[1] - c1[1])*m11 + (c0[0] - c1[0])*m01) +
		 (c0[0] - c1[0])*((c0[1] - c1[1])*m01 + (c0[0] - c1[0])*m00))+
	    sqrt((c0[1] - c2[1])*((c0[1] - c2[1])*m11 + (c0[0] - c2[0])*m01) +
		 (c0[0] - c2[0])*((c0[1] - c2[1])*m01 + (c0[0] - c2[0])*m00))+
	    sqrt((c2[1] - c1[1])*((c2[1] - c1[1])*m11 + (c2[0] - c1[0])*m01) +
		 (c2[0] - c1[0])*((c2[1] - c1[1])*m01 + (c2[0] - c1[0])*m00));

	  // Area in physical space
	  double a = element_area(eid);

	  // Area in metric space
	  double a_m = a*sqrt(m00*m11 - m01*m01);

	  // Function
	  double f = std::min(l/3.0, 3.0/l);
	  double F = pow(f * (2.0 - f), 3.0);

	  // This is the 2D Lipnikov functional.
	  double quality = 12.0 * sqrt(3.0) * a_m * F / (l*l);

	  return quality;
	}

  Quality get_mesh_quality() const;

private:
  void create_adjacency();
  void find_surface();
  void set_orientation();

  int orientation;
};

#endif /* MESH_HPP_ */
