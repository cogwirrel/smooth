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
    _b.push_back(i);
    _c.push_back(i);
    _d.push_back(i);
  }
}

Mesh::~Mesh() {
  cudaFreeHost(_a_pinned);
  cudaFreeHost(_b_pinned);
  cudaFreeHost(_c_pinned);
  cudaFreeHost(_d_pinned);
}

void Mesh::pin_data() {
  size_t _a_bytes = sizeof(size_t) * _a.size();
  size_t _b_bytes = sizeof(float) * _b.size();
  size_t _c_bytes = sizeof(float) * _c.size();
  size_t _d_bytes = sizeof(float) * _d.size();
  
  cuda_check(cudaMallocHost((void **)_a_pinned, _a_bytes));
  cuda_check(cudaMallocHost((void **)_b_pinned, _b_bytes));
  cuda_check(cudaMallocHost((void **)_c_pinned, _c_bytes));
  cuda_check(cudaMallocHost((void **)_d_pinned, _d_bytes));
  
  memcpy(_a_pinned, &_a[0], _a_bytes);
  memcpy(_b_pinned, &_b[0], _b_bytes);
  memcpy(_c_pinned, &_c[0], _c_bytes);
  memcpy(_d_pinned, &_d[0], _d_bytes);
}

void Mesh::cuda_check(cudaError_t status) {
  if (status != cudaSuccess) {
    std::cout << "Error could not allocate memory result " << status << std::endl;
    exit(1);
  }
}

    
