#pragma once
#include<vector>
 #include<string>
 #include <stddef.h>
 #include "graph.hpp"

 class AdjacenceMatrixGraph: public Graph{

    public:
    AdjacenceMatrixGraph(const std::vector<std::vector<size_t>>& matrix); //construct


    size_t size() const override; //returns number of vertcies
    size_t nb_edges()const override; //returns number of edges2
    bool edge_exists(size_t i,size_t j) const override; //returns true if there is an edge between vertices i and j
    std::vector<size_t> get_neighbors(size_t i) const ; // returns a vector of the neighbors of vertex i


    private:
    std::vector<std::vector<int>> matrix;


    protected:



 };