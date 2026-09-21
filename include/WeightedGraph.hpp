#pragma once

#include "graph.hpp"

class WeightedGraph : public virtual Graph {
public:
    virtual double weight_between_vertices(
        size_t i,
        size_t j
    ) const = 0;

double total_weight(
    const std::vector<size_t>& path
) const;};