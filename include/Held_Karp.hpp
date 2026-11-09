#pragma once

#include "AdjacenceMatrixWeightedGraph.hpp"
#include <cstddef>
#include <vector>

std::vector<std::size_t> held_karp(
    const AdjacenceMatrixWeightedGraph& graph
);