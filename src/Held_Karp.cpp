#include "Held_Karp.hpp"
#include <limits>
#include "subset.hpp"
#include "subset_iterator.hpp"
#include <algorithm>
#include <stdexcept>

std::vector<std::size_t> held_karp(
    const AdjacenceMatrixWeightedGraph& graph)
{
    const std::size_t n = graph.size();

    if (n == 0) return {};
    if (n == 1) return {0};

    if (n > 20) {
        throw std::runtime_error("Held-Karp supports at most 20 cities");
    }

    const std::size_t m = n - 1;
    const std::size_t number_of_subsets = std::size_t{1}<< m;
    const double infinity = std::numeric_limits<double>::infinity();

    std::vector<std::vector<double>> dp(
    number_of_subsets,
    std::vector<double>(m, infinity));

    for (std::size_t j = 0; j < m; ++j) {
        const std::size_t mask = std::size_t{1} << j;
        dp[mask][j] = graph.weight_between_vertices(0, j + 1);
    }

    std::vector<std::vector<int>> parent(
    number_of_subsets,
    std::vector<int>(m, -1)
);

for (std::size_t k = 2; k<= m; k++){
    subset::SubsetIterator it(k,m);
    while(it!= subset::SubsetIterator::end()){
        subset::Subset visited = *it;
        for (std::size_t last = 0; last < m; ++last){
            if(visited.contains(last)){
                subset::Subset remaining = visited;
                remaining.minus(last);
            

                for (std::size_t previous = 0; previous < m; ++previous){
                    if(remaining.contains(previous)){
                        double candidate = dp[remaining.mask()][previous]
                        + graph.weight_between_vertices(previous+1, last+1);
                        if(candidate < dp[visited.mask()][last]){
                            dp[visited.mask()][last] = candidate;
                            parent[visited.mask()][last] = static_cast<int>(previous);
                        }
                    }
                }
            }

        }




        ++it;
    }


}

subset::Subset full_set(m, m);

double best_cost = infinity;
int best_last = -1;

for (std::size_t last = 0; last < m; ++last) {
    double cost =
        dp[full_set.mask()][last]
        + graph.weight_between_vertices(last + 1, 0);

    if (cost < best_cost) {
        best_cost = cost;
        best_last = static_cast<int>(last);
    }
}
if (best_last == -1) {
    throw std::runtime_error("Could not find a finite tour cost");
}

std::vector<std::size_t> backward_tour;
subset::Subset remaining = full_set;
int last = best_last;

while (last != -1) {
    backward_tour.push_back(last + 1);

    int previous = parent[remaining.mask()][last];

    remaining.minus(last);
    last = previous;
}

backward_tour.push_back(0);

std::reverse(backward_tour.begin(), backward_tour.end());
return backward_tour;


}
