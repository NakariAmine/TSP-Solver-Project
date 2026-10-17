#pragma once

#include "AdjacenceMatrixWeightedGraph.hpp"
#include <map>
#include <vector>
#include <string>
#include <utility>

struct TSPData {
    size_t dimension;
    AdjacenceMatrixWeightedGraph graph;
    std::string type;
};

std::pair<std::string, TSPData> parse_tsp(const std::string& filename);
struct CoordinateData {
    std::map<std::string, std::string> metadata;
    std::vector<double> ys;
    std::vector<double> xs;
    std::vector<int> indexs;
};

CoordinateData parse_coordinates(const std::string& s);
