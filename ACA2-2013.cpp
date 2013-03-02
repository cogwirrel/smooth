//============================================================================
// Name        : ACA2-2013.cpp
// Author      : George Rokos
// Description : 2nd Assessed Coursework for ACA 2013
//============================================================================

#include <cstdlib>
#include <iostream>

#include "Mesh.hpp"


int main(int argc, char **argv){

  Mesh *mesh = new Mesh();
  mesh->pin_data();
  
  delete mesh;

  return EXIT_SUCCESS;
}
