
#include "WeightedGraph.hpp"

double WeightedGraph::total_weight(
    const std::vector<size_t>& path
) const
{
    double total = 0.0;

    for (size_t i = 0; i + 1 < path.size(); i++) {
        total += weight_between_vertices(
            path[i],
            path[i + 1]
        );
    }

    return total;
}
