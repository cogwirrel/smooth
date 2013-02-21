//============================================================================
// Name        : Smooth.cpp
// Author      : George Rokos
// Description : 2D Vertex-Smoothing kernel - Smart Laplacian variant
//============================================================================

#include <algorithm>
#include <cmath>

#include "SVD2x2.hpp"
#include "Smooth.hpp"

#include <limits.h>

std::vector<std::vector<size_t>*> color(Mesh* mesh) {
  // Creates vector intialised to -1
  std::vector<size_t> vertex_to_col(mesh->NNodes, -1);
  // Colors vector holding set of vectors assuming potentially 8 colors
  std::vector<std::vector<size_t>*> col_to_vertex;

  // For each vector loop through and work out what color
  // it can fall in
  for(size_t vid=0; vid < mesh->NNodes; ++vid){
    // Loop over connecting verticies.
    unsigned int colors = UINT_MAX;
    for(std::vector<size_t>::const_iterator it=mesh->NNList[vid].begin();
              it!=mesh->NNList[vid].end(); ++it){
      // If it's been colored
      if (!(vertex_to_col[*it] == -1)) {
        colors &= ~(1 << vertex_to_col[*it]);
      }
    }
    if (colors == 0) {
      //TODO: RUN OUT OF COLOS
    } else {
      int min_bit = __builtin_ffs(colors) - 1;
      vertex_to_col[vid] = min_bit;
      if (min_bit >= col_to_vertex.size()) {
        col_to_vertex.resize(min_bit + 1);
      }
      if (col_to_vertex[min_bit] == NULL) {
        col_to_vertex[min_bit] = new std::vector<size_t>();
      }
      col_to_vertex[min_bit]->push_back(vid);
    }
  }

  return col_to_vertex;
}

void delete_vector(std::vector<std::vector<size_t>* >& vec) {
  for(int i = 0; i < vec.size(); i++) {
    delete vec[i];
  }
}

__global__ void smooth_vector(Mesh* mesh, size_t* vids) {
  size_t vid = vids[blockIdx.x];
  if(mesh->isCornerNode(vid))
    return;

  // Find the quality of the worst element adjacent to vid
  double worst_q=1.0;
  for(std::set<size_t>::const_iterator it=mesh->NEList[vid].begin();
      it!=mesh->NEList[vid].end(); ++it){
    worst_q = std::min(worst_q, mesh->element_quality(*it));
  }

  /* Find the barycentre (centre of mass) of the cavity. A cavity is
   * defined as the set containing vid and all its adjacent vertices and
   * triangles. Since we work on metric space, all lengths have to measured
   * using the metric. The metric tensor is a 2x2 symmetric matrix which
   * encodes the ideal length and orientation of an edge containing vid. As
   * an edge is defined by two vertices, we calculate the edge length using
   * the value of the metric in the middle of the edge, i.e. the average of
   * the two metric tensors of the vertices defining the edge.
   */

  const double * m0 = &mesh->metric[3*vid];

  double x0 = mesh->coords[2*vid];
  double y0 = mesh->coords[2*vid+1];

  double A[4] = {0.0, 0.0, 0.0, 0.0};
  double q[2] = {0.0, 0.0};

  // Iterate over all edges and assemble matrices A and q.
  for(std::vector<size_t>::const_iterator it=mesh->NNList[vid].begin();
      it!=mesh->NNList[vid].end(); ++it){
    size_t il = *it;

    const double *m1 = &mesh->metric[3*il];

    // Find the metric in the middle of the edge.
    double ml00 = 0.5*(m0[0] + m1[0]);
    double ml01 = 0.5*(m0[1] + m1[1]);
    double ml11 = 0.5*(m0[2] + m1[2]);

    double x = mesh->coords[2*il] - x0;
    double y = mesh->coords[2*il+1] - y0;

    // Calculate and accumulate the contribution of
    // this vertex to the barycentre of the cavity.
    q[0] += (ml00*x + ml01*y);
    q[1] += (ml01*x + ml11*y);

    A[0] += ml00;
    A[1] += ml01;
    A[3] += ml11;
  }

  // The metric tensor is symmetric, i.e. ml01=ml10, so A[2]=A[1].
  A[2]=A[1];

  // Displacement vector for vid
  double p[2];

  /* The displacement p for vid is found by solving the linear system:
   * ┌─       ─┐   ┌    ┐   ┌    ┐
   * │A[0] A[1]│   │p[0]│   │q[0]│
   * │         │ x │    │ = │    │
   * │A[2] A[3]│   │p[1]│   │q[0]│
   * └─       ─┘   └    ┘   └    ┘
   */
  svd_solve_2x2(A, p, q);

  /* If this is a surface vertex, restrict the displacement
   * to the surface. The new displacement is the projection
   * of the old displacement on the surface.
   */
  if(mesh->isSurfaceNode(vid)){
    p[0] -= p[0]*fabs(mesh->normals[2*vid]);
    p[1] -= p[1]*fabs(mesh->normals[2*vid+1]);
  }

  // Update the coordinates
  mesh->coords[2*vid] += p[0];
  mesh->coords[2*vid+1] += p[1];

  /************************************************************************
   * At this point we must also interpolate the metric tensors from all   *
   * neighbouring vertices in order to calculate the new value of vid's   *
   * metric tensor at the new location. This is a quite complex procedure *
   * and has been omitted for simplicity of the exercise. A vertex will   *
   * always use its original metric tensor, no matter whether it has been *
   * relocated or not.                                                    *
   ************************************************************************/

  /* Find the quality of the worst element after smoothing. If an element
   * of the cavity was inverted, i.e. if vid was relocated outside the
   * interior convex hull of the cavity, then the calculated area of that
   * element will be negative and mesh->element_quality() will return a
   * negative number. In such a case, the smoothing operation has to be
   * rejected.
   */
  double new_worst_q=1.0;
  for(std::set<size_t>::const_iterator it=mesh->NEList[vid].begin();
      it!=mesh->NEList[vid].end(); ++it){
    new_worst_q = std::min(new_worst_q, mesh->element_quality(*it));
  }

  /* If quality is worse than before, either because of element inversion
   * or just because relocating vid to the barycentre of the cavity does
   * not improve quality, revert the changes.
   */
  if(new_worst_q < worst_q){
    mesh->coords[2*vid] -= p[0];
    mesh->coords[2*vid+1] -= p[1];
  }
}

void smooth(Mesh* mesh, size_t niter){
  std::vector<std::vector<size_t>*> colourings = color(mesh);

  // For the specified number of iterations, loop over all mesh vertices.
  for(size_t iter=0; iter<niter; ++iter){

    // Loop over colouring groups
    for(std::vector<std::vector<size_t>*>::const_iterator nodes=colourings.begin();
        nodes != colourings.end(); ++nodes) {

      /***********************************************/
      // This could maybe be replaced with:
      // size_t* vids = &nodes[0];

      // See http://stackoverflow.com/questions/2923272/how-to-convert-vector-to-array-c

      // Size of vids array
      int size = (*nodes)->size() * sizeof(size_t);

      // Declare an array of size number of vids in colouring in host
      size_t* vids = malloc(size);

      int i = 0;
      for(std::vector<size_t>::const_iterator v = (*nodes)->begin();
          v != (*nodes)->end(); ++v) {
        // Fill up array with vids
        vids[i] = *v;
        ++i;
      }
      /***********************************************/

      // Device copy of vids
      size_t* d_vids;

      // Allocate space for vids on device
      cudaMalloc((void **)&d_vids, size);

      // Copy host vids to device d_vids
      cudaMemcpy(d_vids, vids, size, cudaMemcpyHostToDevice);
      
      // Device copy of mesh
      Mesh* d_mesh;
      int meshsize = sizeof(mesh);
      cudaMalloc((void**)&d_mesh, meshsize);
      cudaMemcpy(d_mesh, mesh, meshsize, cudaMemcpyHostToDevice);

      // Kick off parallel execution - one block per vid in vids
      smooth_vector<<<d_vids.size(), 1>>>(d_mesh, d_vids);

      // Copy result back to host
      cudaMemcpy(mesh, d_mesh, meshsize, cudaMemcpyDeviceToHost);

      // Clean up everything bar result
      free(vids);
      cudaFree(d_vids);
      cudaFree(d_mesh);

      /*
      for(std::vector<size_t>::const_iterator v = (*nodes)->begin();
          v != (*nodes)->end(); ++v) {
        size_t vid = *v;

        //TODO: RUN ON GPU
        smooth_vector(mesh, vid);
      }*/
    }
  }
}
