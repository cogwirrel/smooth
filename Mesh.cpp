//============================================================================
// Name        : Mesh.cpp
// Author      : George Rokos
// Description : Mesh implementation
//============================================================================

#include <iostream>
#include <cmath>
#include <vector>
#include <string.h>

#include <cuda.h>
#include <cuda_runtime.h>

#include "Mesh.hpp"

Mesh::Mesh(){
  for(size_t i = 0; i < 10; i++){
    _a.push_back(i);
  }
}

Mesh::~Mesh() {  
  cudaFreeHost(_a_pinned);
}

void Mesh::pin_data() {
  size_t _a_bytes = sizeof(size_t) * _a.size();
  
  cuda_check(cudaMallocHost((void **)&_a_pinned, _a_bytes));
  memcpy(_a_pinned, &_a[0], _a_bytes);
}

void Mesh::cuda_check(cudaError_t status) {
  if (status != cudaSuccess) {
    std::cout << "Error could not allocate memory result " << status << std::endl;
    exit(1);
  }
}

    
