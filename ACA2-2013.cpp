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

int main(int argc, char **argv){
  Mesh *mesh = new Mesh();

  Smooth::smooth(mesh);
  
  delete mesh;

  return EXIT_SUCCESS;
}
