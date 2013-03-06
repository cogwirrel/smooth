//============================================================================
// Author      : George Rokos
// Description : 2nd Assessed Coursework for ACA 2013
//============================================================================

#include <cstdlib>
#include <iostream>
#include <sys/time.h>

#include "Mesh.hpp"
#include "Smooth.hpp"
#include "Color.hpp"

double get_wtime(){
    struct timeval tic;

    long seconds, useconds;

    gettimeofday(&tic, NULL);

    seconds  = tic.tv_sec;
    useconds = tic.tv_usec;

    return seconds + useconds*1e-06;
}

void colourSetsToArray(const std::vector<std::vector<size_t> > & colour_sets,
                       size_t** colourIndex,
                       size_t** colourArray,
                       size_t* num_coloured_nodes
                      )
  {
    std::vector<std::vector<size_t> >::const_iterator vec_it;
    std::vector<size_t>::const_iterator vector_it;

    cudaHostAlloc((void**)colourIndex, colour_sets.size()+1, cudaHostAllocPortable);
    
    *num_coloured_nodes = 0;
    for(vec_it = colour_sets.begin(); vec_it != colour_sets.end(); ++vec_it) {
      *num_coloured_nodes += vec_it->size(); 
    }
    
    cudaHostAlloc((void **) colourArray, *num_coloured_nodes, cudaHostAllocPortable);

    size_t offset = 0;

    size_t colorSetIndex = 0;
    for(vec_it = colour_sets.begin(); vec_it != colour_sets.end(); vec_it++, colorSetIndex++)
    {
      (*colourIndex)[colorSetIndex] = offset;
      for(vector_it = vec_it->begin();
          vector_it != vec_it->end();
          vector_it++, offset++) {
        (*colourArray)[offset] = *vector_it;
      }
    }
  
    (*colourIndex)[colour_sets.size()] = offset;
  }

int main(int argc, char **argv){
  if(argc!=2){
    std::cerr << "Usage: " << argv[0] << " mesh_file" << std::endl;
  }

  Mesh *mesh = new Mesh(argv[1]);
  std::vector<std::vector<size_t> > colorings = Color::color(mesh, false);
  
  /* Pin mesh nd color data */
  mesh->pin_data();

  size_t* colourIndex;
  size_t* colourArray;
  size_t num_coloured_nodes;
  colourSetsToArray(colorings, &colourIndex, &colourArray, &num_coloured_nodes);
  
  Quality q = mesh->get_mesh_quality();

  std::cout << "Initial quality:\n"
            << "Quality mean:  " << q.mean << std::endl
            << "Quality min:   " << q.min << std::endl;

  double time = get_wtime();
  Smooth::smooth(mesh, 200, colorings.size(), num_coloured_nodes, colourIndex, colourArray);
  double time_smooth = get_wtime() - time;

  q = mesh->get_mesh_quality();

  std::cout<<"After smoothing:\n"
           << "Quality mean:  " << q.mean << std::endl
           << "Quality min:   " << q.min << std::endl;

  if((q.mean>0.90)&&(q.min>0.55))
    std::cout << "Test passed"<< std::endl;
  else
    std::cout << "Test failed"<< std::endl;

  std::cout<<"BENCHMARK: " << time_smooth << "s" << std::endl;

  delete mesh;
  cudaFreeHost(colourIndex);
  cudaFreeHost(colourArray);
  return EXIT_SUCCESS;
}
