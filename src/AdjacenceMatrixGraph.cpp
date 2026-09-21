
#include<vector>
 #include<string>
 #include <stddef.h>
 #include "AdjacenceMatrixGraph.hpp"
 #include <stdexcept>


AdjacenceMatrixGraph::AdjacenceMatrixGraph
(const std::vector<std::vector<size_t>>& input): matrix(input) {

};


size_t AdjacenceMatrixGraph::size() const {

    return matrix.size();
};

size_t AdjacenceMatrixGraph::nb_edges() const{

    size_t count = 0;

    for (size_t i = 0; i < matrix.size(); i++){
        for (size_t j = 0; j < matrix[i].size(); j++){
            if(matrix[i][j] == 1 ){
                count ++;
            }
        }
    }

    return count; 
};

bool AdjacenceMatrixGraph::edge_exists(size_t i,size_t j) const {
if (i >= matrix.size() || j >= matrix.size()) {
    throw std::out_of_range("Invalid vertex index");
}

return matrix[i][j] == 1;

}

std::vector<size_t> AdjacenceMatrixGraph::get_neighbors(size_t i) const{
    std::vector<size_t> neighbors;

    for (size_t j = 0; j < matrix[i].size(); j++){
        if(matrix[i][j] == 1){
            neighbors.push_back(j);
        }
    }

    return neighbors;

}
