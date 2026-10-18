// Partial ChatGPT/Codex assistance, 2026-10-16 (specifically for how tests should be formatted and implemented).
#include "Parser.hpp"
#include <iostream>

int main()
{
    std::string root = TSP_SOURCE_DIR;

    // Test 1: read the matrix and check a distance in both directions.
    std::pair<std::string, TSPData> matrix =
        parse_tsp(root + "/TSP-instances/lower_diag/gr17.tsp");
    if (matrix.first != "gr17" || matrix.second.graph.size() != 17 ||
        matrix.second.graph.weight_between_vertices(0, 1) != 633 ||
        matrix.second.graph.weight_between_vertices(1, 0) != 633) {
        std::cout << "Matrix test failed.\n";
        return 1;
    }

    // Test 2: read the coordinates and check the city IDs and first position.
    CoordinateData coords = parse_coordinates(root + "/TSP-instances/coord/berlin52.tsp");
    if (coords.indexs.size() != 52 || coords.xs.size() != 52 || coords.ys.size() != 52 ||
        coords.indexs.front() != 1 || coords.indexs.back() != 52 ||
        coords.xs.front() != 565 || coords.ys.front() != 575) {
        std::cout << "Coordinate test failed.\n";
        return 1;
    }

    std::cout << "All 2 tests passed.\n";
    return 0;
}
