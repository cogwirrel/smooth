//============================================================================
// Name        : Smooth.cpp
// Author      : George Rokos
// Description : 2D Vertex-Smoothing kernel - Smart Laplacian variant
//============================================================================


#include <cmath>
#include <limits.h>
#include "Smooth.hpp"

#include "CUDATools.h"

void Smooth::smooth(Mesh* mesh,
            size_t niter,
            std::vector<std::vector<size_t> >& colorings){

  CUDATools cudaTools;

  cudaTools.initialize();

  if(cudaTools.isEnabled()) {
    cudaTools.copyMeshDataToDevice(mesh, colorings, 2); // TODO do we need a quality? (NULL for now)
   // For the specified number of iterations, loop over all mesh vertices.
    for(size_t iter=0; iter<niter; ++iter){

       // Loop over colouring groups
      for(unsigned int ic = 0; ic < colorings.size(); ++ic) {

        cudaTools.copyCoordinatesToDevice(mesh);
        cudaTools.copyMetricToDevice(mesh);
        cudaTools.launchSmoothingKernel(ic);
        cudaTools.copyCoordinatesFromDevice(mesh);
        cudaTools.copyMetricFromDevice(mesh);
      }
    }
  }
}
