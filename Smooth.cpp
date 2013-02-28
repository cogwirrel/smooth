//============================================================================
// Name        : Smooth.cpp
// Author      : George Rokos
// Description : 2D Vertex-Smoothing kernel - Smart Laplacian variant
//============================================================================


#include <cmath>
#include <limits.h>
#include "Smooth.hpp"

#include "CUDATools.h"

void Smooth::smooth(Mesh* mesh) {
  CUDATools cudaTools;

  cudaTools.initialize();

  cudaTools.launchSmoothingKernel(mesh);
  cudaTools.copyCoordinatesFromDevice(mesh);
}
