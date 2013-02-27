#ifndef COLOR_HPP_
#define COLOR_HPP_

#include <algorithm>
#include <limits.h>

class Color {
public:
  static std::vector<std::vector<size_t>*> color(Mesh* mesh) {
    // Creates vector intialised to UINT_MAX being not colored yet
    std::vector<size_t> vertex_to_col(mesh->NNodes, UINT_MAX);
    // Colors vector holding set of vectors assuming potentially 8 colors
    std::vector<std::vector<size_t>*> col_to_vertex;

    // For each vector loop through and work out what color
    // it can fall in
    for(size_t vid=0; vid < mesh->NNodes; ++vid){
      // Loop over connecting verticies.
      unsigned int colors = UINT_MAX;
      for(std::vector<size_t>::const_iterator it=mesh->NNList[vid].begin();
                it!=mesh->NNList[vid].end(); ++it){
        // If it's been colored
        if (!(vertex_to_col[*it] == UINT_MAX)) {
          colors &= ~(1 << vertex_to_col[*it]);
        }
      }
      if (colors == 0) {
        //TODO: RUN OUT OF COLOS
      } else {
        unsigned int min_bit = __builtin_ffs(colors) - 1;
        vertex_to_col[vid] = min_bit;
        if (min_bit >= col_to_vertex.size()) {
          col_to_vertex.resize(min_bit + 1);
        }
        if (col_to_vertex[min_bit] == NULL) {
          col_to_vertex[min_bit] = new std::vector<size_t>();
        }
        col_to_vertex[min_bit]->push_back(vid);
      }
    }

    return col_to_vertex;
  }
};

#endif