//============================================================================
// Name        : Mesh.hpp
// Author      : George Rokos
// Description : Mesh description
//============================================================================

#ifndef MESH_HPP_
#define MESH_HPP_


#include <cstddef>
#include <vector>

#include <driver_types.h>

class Mesh{
public:
  Mesh();
  ~Mesh();  
  void pin_data();

  std::vector<size_t> _a;
  size_t* _a_pinned;

private:
  void cuda_check(cudaError_t success);
};

#endif /* MESH_HPP_ */
