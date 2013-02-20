//============================================================================
// Name        : Smooth.hpp
// Author      : George Rokos
// Description : 2D Vertex-Smoothing kernel prototype
//============================================================================

#ifndef SMOOTH_HPP_
#define SMOOTH_HPP_

#include <vector>

#include "Mesh.hpp"
void delete_vector(std::vector<std::vector<size_t>* >& vec);
std::vector<std::vector<size_t>*> color(Mesh *mesh);
void smooth(Mesh *mesh, size_t niter);
void smooth_vector(Mesh* mesh, size_t vid);

#endif /* SMOOTH_HPP_ */
