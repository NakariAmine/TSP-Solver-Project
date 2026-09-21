#include "CompleteGraph.hpp"

#include <stdexcept>

size_t CompleteGraph::nb_edges() const
{
    if (size() < 2) {
        return 0;
    }

    return size() * (size() - 1) / 2;
}

bool CompleteGraph::edge_exists(size_t i, size_t j) const
{
    if (i >= size() || j >= size()) {
        throw std::out_of_range("Invalid vertex index");
    }

    return i != j;
}

std::vector<size_t> CompleteGraph::get_neighbors(size_t i) const
{
    if (i >= size()) {
        throw std::out_of_range("Invalid vertex index");
    }

    std::vector<size_t> neighbors;

    for (size_t j = 0; j < size(); j++) {
        if (j != i) {
            neighbors.push_back(j);
        }
    }

    return neighbors;
}