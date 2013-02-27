/*  Copyright (C) 2010 Imperial College London and others.
 *
 *  Please see the AUTHORS file in the main source directory for a
 *  full list of copyright holders.
 *
 *  Georgios Rokos
 *  Software Performance Optimisation Group
 *  Department of Computing
 *  Imperial College London
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following
 *  disclaimer in the documentation and/or other materials provided
 *  with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 */

#ifndef CUDATOOLS_H
#define CUDATOOLS_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <string>
#include <map>
#include <set>
#include <vector>

#include <iostream>
#include <stdint.h>

#include <cuda.h>

#include "Mesh.hpp"

class CUDATools
{
public:

  CUDATools()
  {
    enabled = false;
  }

  bool isEnabled()
  {
    return enabled;
  }

  void initialize()
  {
    enabled = false;

      if(cuInit(0) != CUDA_SUCCESS)
      {
        std::cout << "Error initializing CUDA driver" << std::endl;;
        return;
      }

      int deviceCount = 0;
      cuDeviceGetCount(&deviceCount);
      if(deviceCount == 0)
      {
        std::cout << "No CUDA-enabled devices found" << std::endl;
        return;
      }

      if(cuDeviceGet(&cuDevice, 0) != CUDA_SUCCESS)
      {
        std::cout << "Cannot get CUDA device" << std::endl;
        return;
      }

      if(cuCtxCreate(&cuContext, 0, cuDevice) != CUDA_SUCCESS)
      {
        std::cout << "Error creating CUDA context" << std::endl;
        return;
      }

      //TODO: modify makefile to load this
      if(cuModuleLoad(&smoothModule, "SmoothVector.cubin") != CUDA_SUCCESS)
      {
        std::cout << "Error loading CUDA cubin module \"Smooth\"" << std::endl;
        return;
      }

      if(cuModuleGetFunction(&smoothKernel, smoothModule, "smooth") != CUDA_SUCCESS)
      {
        std::cout << "Error loading CUDA kernel smooth method" << std::endl;
        return;
      }

      enabled = true;
  }

  void copyMeshDataToDevice(Mesh * mesh,
      std::vector<std::vector<size_t>*>& colour_sets,
      size_t dimensions)
  {
    ndims = dimensions;
    nloc = ndims+1;
    NNodes = mesh->NNodes;
    NElements = mesh->NElements;

    // convert pragmatic data-structures to C-style arrays
    NNListToArray(mesh->NNList);
    colourSetsToArray(colour_sets); //TODO ???
    NEListToArray(mesh->NEList);

    // and copy everything to the device
    copyArrayToDevice<double>(&mesh->coords[0], CUDA_coords, NNodes * ndims);
    copyArrayToDevice<double>(&mesh->metric[0], CUDA_metric, NNodes * ndims * ndims);
    copyArrayToDevice<double>(&mesh->normals[0], CUDA_normals, NNodes * ndims);
    copyArrayToDevice<size_t>(&mesh->ENList[0], CUDA_ENList, NElements * nloc);
    copyArrayToDevice<size_t>(NNListArray, CUDA_NNListArray, NNListArray_size);
    copyArrayToDevice<size_t>(NNListIndex, CUDA_NNListIndex, NNodes+1);
    copyArrayToDevice<size_t>(colourArray, CUDA_colourArray, NNodes);
    copyArrayToDevice<size_t>(NEListArray, CUDA_NEListArray, NEListArray_size);
    copyArrayToDevice<size_t>(NEListIndex, CUDA_NEListIndex, NNodes+1);

    //set the constant symbols of the smoothing-kernel, i.e. the addresses of all arrays copied above
    CUdeviceptr address; // TODO ????
    size_t symbol_size;

    #define SET_CONSTANT(SYMBOL_NAME) \
      cuModuleGetGlobal(&address, &symbol_size, smoothModule, #SYMBOL_NAME); \
      cuMemcpyHtoD(address, &CUDA_ ## SYMBOL_NAME, symbol_size);

    SET_CONSTANT(coords)
    SET_CONSTANT(metric)
    SET_CONSTANT(normals)
    SET_CONSTANT(ENList)
    SET_CONSTANT(NNListArray)
    SET_CONSTANT(NNListIndex)
    SET_CONSTANT(NEListArray)
    SET_CONSTANT(NEListIndex)

    // set element orientation in CUDA smoothing kernel
    cuModuleGetGlobal(&CUDA_orientation, &symbol_size, smoothModule, "orientation");
    cuMemcpyHtoD(CUDA_orientation, &mesh->orientation, symbol_size);
  }

  void copyCoordinatesToDevice(Mesh* mesh)
  {
    copyArrayToDeviceNoAlloc(&mesh->coords[0], CUDA_coords, NNodes * ndims);
  }

  void copyMetricToDevice(Mesh* mesh)
  {
    copyArrayToDeviceNoAlloc(&mesh->metric[0], CUDA_metric, NNodes * ndims * ndims);
  }

  void copyCoordinatesFromDevice(Mesh* mesh)
  {
    copyArrayFromDevice(&mesh->coords[0], CUDA_coords, NNodes * ndims);
  }

  void copyMetricFromDevice(Mesh* mesh)
  {
    copyArrayFromDevice<double>((double *) &mesh->metric[0], CUDA_metric, NNodes * ndims * ndims);
  }

  void freeResources()
  {
    cuMemFree(CUDA_coords);
    cuMemFree(CUDA_metric);
    cuMemFree(CUDA_normals);
    //cuMemFree(CUDA_quality);
    cuMemFree(CUDA_ENList);
    // cuMemFree(CUDA_coplanar_ids);
    cuMemFree(CUDA_NNListArray);
    cuMemFree(CUDA_NNListIndex);
    cuMemFree(CUDA_colourArray);
    cuMemFree(CUDA_NEListArray);
    cuMemFree(CUDA_NEListIndex);
    // cuMemFree(CUDA_smoothStatus);

    delete[] NNListArray;
    delete[] NNListIndex;
    delete[] colourArray;
    delete[] colourIndex;
    delete[] NEListArray;
    delete[] NEListIndex;

    cuCtxDestroy(cuContext);
  }

  void launchSmoothingKernel(int colour)
  {
    CUdeviceptr CUDA_ColourSetAddr = CUDA_colourArray + colourIndex[colour] * sizeof(size_t);
    size_t NNodesInSet = colourIndex[colour+1] - colourIndex[colour];
    threadsPerBlock = 32;
    blocksPerGrid = (NNodesInSet + threadsPerBlock - 1) / threadsPerBlock;

    void * args[] = {&CUDA_ColourSetAddr, &NNodesInSet};

    CUresult result = cuLaunchKernel(smoothKernel, blocksPerGrid, 1, 1, threadsPerBlock, 1, 1, 0, 0, args, NULL);
    if(result != CUDA_SUCCESS)
    {
      std::cout << "Error launching CUDA kernel for colour " << colour << " result "<< result << std::endl;
      return;
    }

    result = cuCtxSynchronize();
    if(result != CUDA_SUCCESS)
      std::cout << "Sync result " << result << std::endl;
  }

private:
  void NNListToArray(const std::vector< std::vector<size_t> > & NNList)
  {
    std::vector< std::vector<size_t> >::const_iterator vec_it;
    std::vector<size_t>::const_iterator vector_it;
    size_t offset = 0;
    size_t index = 0;

    for(vec_it = NNList.begin(); vec_it != NNList.end(); vec_it++)
      offset += vec_it->size();

    NNListArray_size = offset;

    NNListIndex = new size_t[NNodes+1];
    NNListArray = new size_t[NNListArray_size];

    offset = 0;

    for(vec_it = NNList.begin(); vec_it != NNList.end(); vec_it++)
    {
      NNListIndex[index++] = offset;

      for(vector_it = vec_it->begin(); vector_it != vec_it->end(); vector_it++)
        NNListArray[offset++] = *vector_it;
    }

    assert(index == NNList.size());
    NNListIndex[index] = offset;
  }

  void colourSetsToArray(const std::vector<std::vector<size_t>*> & colour_sets)
  {
    std::vector<std::vector<size_t>*>::const_iterator vec_it;
    std::vector<size_t>::const_iterator vector_it;

    NColours = colour_sets.size();

    colourIndex = new size_t[NColours+1];
    colourArray = new size_t[NNodes];

    size_t offset = 0;

    size_t colorSetIndex = 0;
    for(vec_it = colour_sets.begin(); vec_it != colour_sets.end(); vec_it++, colorSetIndex++)
    {
      colourIndex[colorSetIndex] = offset;

      for(vector_it = (*vec_it)->begin();
          vector_it != (*vec_it)->end();
          vector_it++, offset++) {
        colourArray[offset] = *vector_it;
      }
    }

    colourIndex[colour_sets.size()] = offset;
  }

  void NEListToArray(const std::vector< std::set<size_t> > & NEList)
  {
    std::vector< std::set<size_t> >::const_iterator vec_it;
    std::set<size_t>::const_iterator set_it;
    size_t offset = 0;
    size_t index = 0;

    for(vec_it = NEList.begin(); vec_it != NEList.end(); vec_it++)
      offset += vec_it->size();

    NEListArray_size = offset;

    NEListIndex = new size_t[NNodes+1];
    NEListArray = new size_t[NEListArray_size];

    offset = 0;

    for(vec_it = NEList.begin(); vec_it != NEList.end(); vec_it++)
    {
      NEListIndex[index++] = offset;

      for(set_it = vec_it->begin(); set_it != vec_it->end(); set_it++)
        NEListArray[offset++] = *set_it;
    }

    assert(index == NEList.size());
    NEListIndex[index] = offset;
  }

  template<typename type>
  inline void copyArrayToDevice(type * array, CUdeviceptr & CUDA_array, size_t array_size)
  {
    if(cuMemAlloc(&CUDA_array, array_size * sizeof(type)) != CUDA_SUCCESS)
    {
      std::cout << "Error allocating CUDA memory" << std::endl;
      exit(1);
    }

    cuMemcpyHtoD(CUDA_array, array, array_size * sizeof(type));
  }

  template<typename type>
  inline void copyArrayToDeviceNoAlloc(const type * array, CUdeviceptr & CUDA_array, size_t array_size)
  {
    cuMemcpyHtoD(CUDA_array, array, array_size * sizeof(type));
  }

  template<typename type>
  inline void copyArrayFromDevice(type * array, CUdeviceptr & CUDA_array, size_t array_size)
  {
    cuMemcpyDtoH(array, CUDA_array, array_size * sizeof(type));
  }

  bool enabled;

  CUdevice cuDevice;
  CUcontext cuContext;

  CUmodule smoothModule;
  CUmodule coarsenModule;
  CUmodule refineModule;

  CUfunction smoothKernel;
  CUfunction coarsenKernel;
  CUfunction refineKernel;

  unsigned int threadsPerBlock, blocksPerGrid;

  size_t NNodes, NElements, NSElements, ndims, nloc;

  CUdeviceptr CUDA_coords;
  CUdeviceptr CUDA_metric;
  CUdeviceptr CUDA_normals;
  CUdeviceptr CUDA_ENList;

  size_t * NNListArray;
  size_t * NNListIndex;
  CUdeviceptr CUDA_NNListArray;
  CUdeviceptr CUDA_NNListIndex;
  size_t NNListArray_size;

  size_t * NEListArray;
  size_t * NEListIndex;
  CUdeviceptr CUDA_NEListArray;
  CUdeviceptr CUDA_NEListIndex;
  size_t NEListArray_size;

  size_t * colourArray;
  size_t* colourIndex;
  CUdeviceptr CUDA_colourArray;
  size_t NColours;

  CUdeviceptr CUDA_orientation;
};

#endif
