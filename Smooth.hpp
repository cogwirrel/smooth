//============================================================================
// Name        : Smooth.hpp
// Author      : George Rokos
// Description : 2D Vertex-Smoothing kernel prototype
//============================================================================

#ifndef SMOOTH_HPP_
#define SMOOTH_HPP_

#include <vector>

#include "Mesh.hpp"
#include "CUDATools.h"
#include <iostream>
class Smooth{
  public:
    static void smooth(Mesh* mesh,
      size_t niter,
      size_t num_colors,
      size_t num_colored_nodes,
      size_t* colourIndex,
      size_t* colourArray) {
      
      CUDATools cudaTools;

      cudaTools.initialize();

      if(cudaTools.isEnabled()) {
        cudaTools.copyMeshDataToDevice(mesh, colourIndex, colourArray, num_colored_nodes, 2);
       // For the specified number of iterations, loop over all mesh vertices.
        for(size_t iter=0; iter<niter; ++iter){

           // Loop over colouring groups
          for(unsigned int ic = 0; ic < num_colors; ++ic) {
            cudaTools.launchSmoothingKernel(ic);
          }
        }
        cudaTools.copyCoordinatesFromDevice(mesh);
        cudaTools.freeResources();
      }
    }
};


#endif /* SMOOTH_HPP_ */
