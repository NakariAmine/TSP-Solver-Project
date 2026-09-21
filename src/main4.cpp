 #include "AdjacenceMatrixGraph.hpp"
#include "AdjacenceMatrixWeightedGraph.hpp"
#include "CoordinateGraph.hpp"

#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

void print_path(const std::vector<size_t>& path)
{
    if (path.empty()) {
        std::cout << "No path found\n";
        return;
    }

    for (size_t i = 0; i < path.size(); i++) {
        std::cout << path[i];

        if (i + 1 < path.size()) {
            std::cout << " -> ";
        }
    }

    std::cout << '\n';
}

int main()
{
    // ---------------- Social network ----------------

    std::vector<std::vector<size_t>> social_network = {
        {1, 2},       // Person 0 knows 1 and 2
        {0, 3},       // Person 1 knows 0 and 3
        {0, 3},       // Person 2 knows 0 and 3
        {1, 2, 4},    // Person 3 knows 1, 2 and 4
        {3}           // Person 4 knows 3
    };

    AdjacenceListGraph social_graph(social_network);

    std::vector<size_t> social_path =
        social_graph.find_path(0, 4);

    std::cout << "Social network path: ";
    print_path(social_path);


    // ---------------- Tram network ----------------

    std::vector<std::vector<double>> tram_network = {
        {0.0, 4.5, 0.0, 0.0},
        {4.5, 0.0, 3.0, 0.0},
        {0.0, 3.0, 0.0, 5.5},
        {0.0, 0.0, 5.5, 0.0}
    };

    AdjacenceMatrixWeightedGraph tram_graph(tram_network);

    std::vector<size_t> tram_path =
        tram_graph.find_path(0, 3);

    std::cout << "Tram network path: ";
    print_path(tram_path);

    if (!tram_path.empty()) {
        std::cout << "Total travel time: "
                  << tram_graph.total_weight(tram_path)
                  << " minutes\n";
    }


    // ---------------- Geographic map ----------------

    std::vector<std::pair<double, double>> map = {
        {0, 2},
        {10, 5},
        {30, 21.5},
        {11.2, 13.3},
        {52, 34}
    };

    CoordinatesGraph geographic_graph(
        map,
        euclidean_distance
    );

    std::vector<size_t> geographic_path =
        geographic_graph.find_path(0, 4);

    std::cout << "Geographic map path: ";
    print_path(geographic_path);

    if (!geographic_path.empty()) {
        std::cout << "Total geographic distance: "
                  << geographic_graph.total_weight(geographic_path)
                  << '\n';
    }

    return EXIT_SUCCESS;
}