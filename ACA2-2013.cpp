//============================================================================
// Name        : ACA2-2013.cpp
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

size_t get_colored_node_count(std::vector<std::vector<size_t> >& colorings) {
  size_t count = 0;
  for(size_t i = 0; i < colorings.size(); i++) {
    count += colorings[i].size();
  }
  return count;
}

int main(int argc, char **argv){
  if(argc!=2){
    std::cerr << "Usage: " << argv[0] << " mesh_file" << std::endl;
  }

  Mesh *mesh = new Mesh(argv[1]);

  std::vector<std::vector<size_t> > colorings = Color::color(mesh);
  size_t num_colored_nodes = get_colored_node_count(colorings);

  Quality q = mesh->get_mesh_quality();

  std::cout << "Initial quality:\n"
            << "Quality mean:  " << q.mean << std::endl
            << "Quality min:   " << q.min << std::endl;

  double time = get_wtime();
  Smooth::smooth(mesh, 200, num_colored_nodes, colorings);
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

  return EXIT_SUCCESS;
}
