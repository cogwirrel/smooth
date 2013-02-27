//============================================================================
// Name        : Smooth.hpp
// Author      : George Rokos
// Description : 2D Vertex-Smoothing kernel prototype
//============================================================================

#ifndef SMOOTH_HPP_
#define SMOOTH_HPP_

#include <vector>

#include "Mesh.hpp"

class Smooth{
  public:
    static void smooth(Mesh *mesh,
                size_t niter,
                std::vector<std::vector<size_t>*>& colorings);
};


#endif /* SMOOTH_HPP_ */
