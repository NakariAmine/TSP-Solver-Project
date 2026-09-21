#pragma once

#include "graph.hpp"

class CompleteGraph : public virtual Graph {
public:
    size_t nb_edges() const override final;

    bool edge_exists(size_t i, size_t j) const override final;

    std::vector<size_t> get_neighbors(size_t i) const override final;
};