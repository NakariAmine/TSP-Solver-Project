 #pragma once

 #include<vector>
 #include<string>
 #include <stddef.h>
 #include "graph.hpp"
 #include "WeightedGraph.hpp"


class AdjacenceMatrixWeightedGraph: public WeightedGraph{


public:
AdjacenceMatrixWeightedGraph(const std::vector<std::vector<double>>& weighted_matrix);

size_t size() const override;
size_t nb_edges() const override;
bool edge_exists(size_t i,size_t j) const override;
double weight_between_vertices(size_t i,size_t j) const override;
std::vector<size_t> get_neighbors(size_t i) const override;

private:
std::vector<std::vector<double>> weight_matrix;

protected:


};