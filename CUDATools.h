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
        std::cout << "Error loading CUDA module \"Smooth\"" << std::endl;
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
   //   std::vector<std::vector<size_t> >& colour_sets,
      size_t* colorIndex,
      size_t* colorArray,
      size_t num_colored_nodes,
      size_t dimensions)
  {
    ndims = dimensions;
    nloc = ndims+1;
    NColoredNodes = num_colored_nodes;
    NNodes = mesh->NNodes;
    NElements = mesh->NElements;

    colourIndex = colorIndex;

    // convert pragmatic data-structures to C-style arrays
    //NNListToArray(mesh->NNList);
    //colourSetsToArray(colour_sets); //TODO ???
    //NEListToArray(mesh->NEList);

    // and copy everything to the device
    copyPinnedDataToDevice(mesh->pinned_data, mesh->total_size,
                                              mesh->ENList_pinned,
                                              mesh->coords_pinned,
                                              mesh->metric_pinned,
                                              mesh->normals_pinned,
                                              mesh->NNListIndex_pinned,
                                              mesh->NNListArray_pinned,
                                              mesh->NEListIndex_pinned,
                                              mesh->NEListArray_pinned);
     //copyArrayToDevice<float>(mesh->coords_pinned, CUDA_coords, NNodes * ndims);
     //copyArrayToDevice<float>(mesh->metric_pinned, CUDA_metric, NNodes * nloc); //TODO is thhis right?
     //copyArrayToDevice<float>(mesh->normals_pinned, CUDA_normals, NNodes * ndims);
     //copyArrayToDevice<size_t>(mesh->ENList_pinned, CUDA_ENList, NElements * nloc);


    //copyArrayToDevice<size_t>(mesh->NNListArray_pinned, CUDA_NNListArray,
    //                          mesh->NNListArray_size);
    //copyArrayToDevice<size_t>(mesh->NNListIndex_pinned, CUDA_NNListIndex, NNodes+1);
    copyArrayToDevice<size_t>(colorArray, CUDA_colourArray, num_colored_nodes);
    //copyArrayToDevice<size_t>(mesh->NEListArray_pinned, CUDA_NEListArray, 
    //                          mesh->NEListArray_size);
    //copyArrayToDevice<size_t>(mesh->NEListIndex_pinned, CUDA_NEListIndex, NNodes+1);
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
    copyArrayToDeviceNoAlloc<float>(&mesh->coords[0], CUDA_coords, NNodes * ndims);
  }

  void copyMetricToDevice(Mesh* mesh)
  {
    copyArrayToDeviceNoAlloc<float>(&mesh->metric[0], CUDA_metric, NNodes * nloc);
  }

  void copyCoordinatesFromDevice(Mesh* mesh)
  {
    copyArrayFromDevice<float>(&mesh->coords[0], CUDA_coords, NNodes * ndims);
  }

  void copyMetricFromDevice(Mesh* mesh)
  {
    copyArrayFromDevice<float>(&mesh->metric[0], CUDA_metric, NNodes * nloc);
  }

  void freeResources()
  {
    // cuMemFree(CUDA_coords);
    // cuMemFree(CUDA_metric);
    // cuMemFree(CUDA_normals);
    // cuMemFree(CUDA_ENList);
    cuMemFree(CUDA_pinned_data);
    // cuMemFree(CUDA_NNListArray);
    // cuMemFree(CUDA_NNListIndex);
    cuMemFree(CUDA_colourArray);
    // cuMemFree(CUDA_NEListArray);
    // cuMemFree(CUDA_NEListIndex);

    cuCtxDestroy(cuContext);
  }

  void launchSmoothingKernel(size_t colour)
  {
    CUdeviceptr CUDA_ColourSetAddr = CUDA_colourArray + colourIndex[colour] * sizeof(size_t);
    size_t NNodesInSet = colourIndex[colour+1] - colourIndex[colour];
    threadsPerBlock = 128;
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
  inline void copyPinnedDataToDevice(
    void* pinned_data,
    size_t total_size,
    size_t* ENList_pinned,
    float* coords_pinned,
    float* metric_pinned,
    float* normals_pinned,
    size_t* NNListIndex_pinned,
    size_t* NNListArray_pinned,
    size_t* NEListIndex_pinned,
    size_t* NEListArray_pinned)
  {
    if(cuMemAlloc(&CUDA_pinned_data, total_size) != CUDA_SUCCESS)
    {
      std::cout << "Error batch allocating CUDA memory" << std::endl;
      exit(1);
    }
    
    if(cuMemcpyHtoD(CUDA_pinned_data, pinned_data, total_size) != CUDA_SUCCESS)
    {
      std::cout << "Unable to copy data to device" << std::endl;
      exit(1);
    }
    
    // Point the device pointers to the right place
    CUDA_ENList = CUDA_pinned_data;
    CUDA_coords = CUDA_pinned_data + ((size_t)coords_pinned - (size_t)pinned_data);
    CUDA_metric = CUDA_pinned_data + ((size_t)metric_pinned - (size_t)pinned_data);
    CUDA_normals = CUDA_pinned_data + ((size_t)normals_pinned - (size_t)pinned_data);
    CUDA_NNListIndex = CUDA_pinned_data + ((size_t)NNListIndex_pinned - (size_t)pinned_data);
    CUDA_NNListArray = CUDA_pinned_data + ((size_t)NNListArray_pinned - (size_t)pinned_data);
    CUDA_NEListIndex = CUDA_pinned_data + ((size_t)NEListIndex_pinned - (size_t)pinned_data);
    CUDA_NEListArray = CUDA_pinned_data + ((size_t)NEListArray_pinned - (size_t)pinned_data);
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

  size_t NNodes, NColoredNodes, NElements, NSElements, ndims, nloc;

  CUdeviceptr CUDA_pinned_data;
  CUdeviceptr CUDA_coords;
  CUdeviceptr CUDA_metric;
  CUdeviceptr CUDA_normals;
  CUdeviceptr CUDA_ENList;

  CUdeviceptr CUDA_NNListArray;
  CUdeviceptr CUDA_NNListIndex;

  CUdeviceptr CUDA_NEListArray;
  CUdeviceptr CUDA_NEListIndex;

  size_t* colourIndex;
  CUdeviceptr CUDA_colourArray;
  size_t NColours;

  CUdeviceptr CUDA_orientation;
};

#endif
