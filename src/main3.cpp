#include "CoordinateGraph.hpp"
#include <vector>
#include <cstdlib>
#include <iostream>

int main()
{
    std::vector<std::pair<double, double>> map = {
        {0, 2},
        {10, 5},
        {30, 21.5},
        {11.2, 13.3},
        {52, 34}
    };

    CoordinatesGraph euclidean_graph(map, euclidean_distance);
    CoordinatesGraph manhattan_graph(map, manhattan_distance);

    std::cout << "The number of cities in the map is "
              << euclidean_graph.size() << std::endl;

    for (size_t i = 0; i < map.size(); i++) {
        for (size_t j = i + 1; j < map.size(); j++) {
            std::cout << "The distance between city "
                      << i << " and " << j << " is:\n";

            std::cout << "  Euclidean: "
                      << euclidean_graph.weight_between_vertices(i, j)
                      << '\n';

            std::cout << "  Manhattan: "
                      << manhattan_graph.weight_between_vertices(i, j)
                      << '\n';
        }
    }

    return EXIT_SUCCESS;
}