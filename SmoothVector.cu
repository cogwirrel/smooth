#ifndef CUDASMOOTH_H
#define CUDASMOOTH_H

#include "SVD2x2.cuh"

#include <cuda.h>
//TODO How do i use cuda standard library?
extern "C" {

__constant__ double* coords;
__constant__ double* metric;
__constant__ double* normals;
// __constant__ real_t * quality;
__constant__ size_t* ENList;
__constant__ size_t* NNListArray;
__constant__ size_t* NNListIndex;
__constant__ size_t* NEListArray;
__constant__ size_t* NEListIndex;
__constant__ int* orientation;


__device__ bool isCornerNode(size_t vid) {
  return fabs(normals[2*vid])==1.0 && fabs(normals[2*vid+1])==1.0;
}

__device__ bool isSurfaceNode(size_t vid) {
  int NE_size = NEListIndex[vid + 1] - NEListIndex[vid];
  int NN_size = NNListIndex[vid + 1] - NNListIndex[vid];
  return NE_size < NN_size;
}

__device__ double element_area(size_t eid) {
  const size_t *n = &ENList[3*eid];

  //Pointers to the coorindates of each vertex
  const double *c0 = &coords[2*n[0]];
  const double *c1 = &coords[2*n[1]];
  const double *c2 = &coords[2*n[2]];

  return *orientation * 0.5 *
          ((c0[1] - c2[1]) * (c0[0] - c1[0]) -
           (c0[1] - c1[1]) * (c0[0] - c2[0]));
}

__device__ double element_quality(size_t eid) {
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
  double f = min(l/3.0, 3.0/l);
  double F = pow(f * (2.0 - f), 3.0);

  // This is the 2D Lipnikov functional.
  double quality = 12.0 * sqrt(3.0) * a_m * F / (l*l);

  return quality;
}

//TODO: WHat are the params?
__global__ void smooth(const size_t* colourSet, const size_t NNodesInSet) {

  const size_t threadID = blockIdx.x * blockDim.x + threadIdx.x;
  if(threadID >= NNodesInSet)
    return;

  size_t vid = colourSet[threadID];

  if(isCornerNode(vid)) {
    return;
  }

  // Find the quality of the worst element adjacent to vid
  double worst_q=1.0;
  // for(std::set<size_t>::const_iterator it=mesh->NEList[vid].begin();
      // it!=mesh->NEList[vid].end(); ++it){
    // worst_q = std::min(worst_q, mesh->element_quality(*it));
  // }

  // Find begining of each vid
  for (int ne_index = NEListIndex[vid];
       ne_index < NEListIndex[vid + 1];
       ++ne_index) {
    double quality = element_quality(NEListArray[ne_index]);
    if (quality < worst_q) {
      worst_q = quality;
    }
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

  const double * m0 = &metric[3*vid];

  double x0 = coords[2*vid];
  double y0 = coords[2*vid+1];

  double A[4] = {0.0, 0.0, 0.0, 0.0};
  double q[2] = {0.0, 0.0};

  // Iterate over all edges and assemble matrices A and q.
  // for(std::vector<size_t>::const_iterator it=mesh->NNList[vid].begin();
  //     it!=mesh->NNList[vid].end(); ++it){
  for (int nn_index = NNListIndex[vid];
       nn_index < NNListIndex[vid + 1];
       ++nn_index) {

      size_t il = NNListArray[nn_index];

      const double *m1 = &metric[3*il];

      // Find the metric in the middle of the edge.
      double ml00 = 0.5*(m0[0] + m1[0]);
      double ml01 = 0.5*(m0[1] + m1[1]);
      double ml11 = 0.5*(m0[2] + m1[2]);

      double x = coords[2*il] - x0;
      double y = coords[2*il+1] - y0;

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
  if(isSurfaceNode(vid)){
    p[0] -= p[0]*fabs(normals[2*vid]);
    p[1] -= p[1]*fabs(normals[2*vid+1]);
  }

  // Update the coordinates
  coords[2*vid] += p[0];
  coords[2*vid+1] += p[1];

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
  // for(std::set<size_t>::const_iterator it=mesh->NEList[vid].begin();
      // it!=mesh->NEList[vid].end(); ++it){
    // new_worst_q = std::min(new_worst_q, mesh->element_quality(*it));
  // }
  for (int ne_index = NEListIndex[vid];
       ne_index < NEListIndex[vid + 1];
       ++ne_index) {
    double quality = element_quality(NEListArray[ne_index]);
    if (quality < new_worst_q) {
      new_worst_q = quality;
    }
  }


  /* If quality is worse than before, either because of element inversion
   * or just because relocating vid to the barycentre of the cavity does
   * not improve quality, revert the changes.
   */
  if(new_worst_q < worst_q){
    coords[2*vid] -= p[0];
    coords[2*vid+1] -= p[1];
  }
}
}

#endif
// void runCudaImplementation(Mesh* mesh, std::vector<size_t>* vids) {
//         /***********************************************/
//   // Device copy of vids
//   size_t* d_vids;

//   // Size of vids
//   size_t vid_size = vids->size() * sizeof(size_t);

//   // Allocate space for vids on device
//   cudaMalloc((void **)&d_vids, vid_size);

//   // Copy host vids to device d_vids
//   cudaMemcpy(d_vids, &vids[0], vid_size, cudaMemcpyHostToDevice);

//   // Device copy of mesh
//   Mesh* d_mesh;
//   size_t mesh_size = sizeof(mesh);
//   cudaMalloc((void**)&d_mesh, mesh_size);
//   cudaMemcpy(d_mesh, mesh, mesh_size, cudaMemcpyHostToDevice);

//   // Kick off parallel execution - one block per vid in vids
//   smooth_vector<<<vids->size(), 1>>>(d_mesh, d_vids);

//   // Copy result back to host
//   cudaMemcpy(mesh, d_mesh, mesh_size, cudaMemcpyDeviceToHost);

//   // Clean up everything bar result
//   free(vids);
//   cudaFree(d_vids);
//   cudaFree(d_mesh);
// }
