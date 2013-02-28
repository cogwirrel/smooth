//============================================================================
// Name        : Mesh.cpp
// Author      : George Rokos
// Description : Mesh implementation
//============================================================================

#include <algorithm>
#include <cassert>
#include <iostream>
#include <cmath>
#include <set>
#include <vector>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>

#include "Mesh.hpp"

Mesh::Mesh(){
  NNodes = 2; 
  coords.push_back(1);
  coords.push_back(2);
  coords.push_back(3);
  coords.push_back(4);
}

