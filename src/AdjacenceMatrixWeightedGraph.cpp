#include "AdjacenceMatrixWeightedGraph.hpp"

#include <stdexcept>
#include <vector>
#include <cstddef>

AdjacenceMatrixWeightedGraph::AdjacenceMatrixWeightedGraph(
    const std::vector<std::vector<double>>& input
)
    : weight_matrix(input)
{
}

size_t AdjacenceMatrixWeightedGraph::size() const
{
    return weight_matrix.size();
}

size_t AdjacenceMatrixWeightedGraph::nb_edges() const
{
    size_t count = 0;

    for (size_t i = 0; i < weight_matrix.size(); i++) {
        for (size_t j = 0; j < weight_matrix[i].size(); j++) {
            if (weight_matrix[i][j] != 0) {
                count++;
            }
        }
    }

    return count;
}

bool AdjacenceMatrixWeightedGraph::edge_exists(
    size_t i,
    size_t j
) const
{
    if (i >= weight_matrix.size() ||
        j >= weight_matrix[i].size()) {
        throw std::out_of_range("Invalid vertex index");
    }

    return weight_matrix[i][j] != 0;
}

double AdjacenceMatrixWeightedGraph::weight_between_vertices(
    size_t i,
    size_t j
) const
{
    if (!edge_exists(i, j)) {
        throw std::runtime_error(
            "An edge does not exist between the two vertices"
        );
    }

    return weight_matrix[i][j];
}

std::vector<size_t>
AdjacenceMatrixWeightedGraph::get_neighbors(size_t i) const
{
    if (i >= weight_matrix.size()) {
        throw std::out_of_range("Invalid vertex index");
    }

    std::vector<size_t> neighbors;

    for (size_t j = 0; j < weight_matrix[i].size(); j++) {
        if (weight_matrix[i][j] != 0) {
            neighbors.push_back(j);
        }
    }

    return neighbors;
}