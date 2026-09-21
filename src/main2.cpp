#include <cstdlib>
#include <iostream>
#include <vector>
#include "AdjacenceMatrixWeightedGraph.hpp"

int main()
{
    // Station IDs:
    // 0 = Grenoble Gare
    // 1 = Victor Hugo
    // 2 = Chavant
    // 3 = Universites

    // Each nonzero value represents the average travel time in minutes.
    //
    // weight_matrix[i][j] = travel time from station i to station j
    //
    // 0 means that no direct connection exists.

    std::vector<std::vector<double>> tram_network = {
        // To:       0    1    2    3

        /* Gare */   {0.0, 4.5, 0.0, 0.0},

        /* Victor */ {4.5, 0.0, 3.0, 0.0},

        /* Chavant */{0.0, 3.0, 0.0, 5.5},

        /* Univ. */  {0.0, 0.0, 5.5, 0.0}
    };

    AdjacenceMatrixWeightedGraph graph(tram_network);

    // Test size()
    std::cout << "Number of stations: "
              << graph.size() << '\n';

    // Test nb_edges()
    std::cout << "Number of directed connections: "
              << graph.nb_edges() << '\n';

    // Test edge_exists()
    if (graph.edge_exists(0, 1)) {
        std::cout << "A direct connection exists from "
                  << "Grenoble Gare to Victor Hugo.\n";
    }

    if (!graph.edge_exists(0, 2)) {
        std::cout << "No direct connection exists from "
                  << "Grenoble Gare to Chavant.\n";
    }

    // Test weight_between_vertices()
    std::cout << "Travel time from Grenoble Gare to Victor Hugo: "
              << graph.weight_between_vertices(0, 1)
              << " minutes\n";

    // Test get_neighbors()
    std::vector<size_t> neighbors = graph.get_neighbors(2);

    std::cout << "Neighbors of Chavant: ";

    for (size_t neighbor : neighbors) {
        std::cout << neighbor << ' ';
    }

    std::cout << '\n';

    double total_time = 0.0;
    size_t connection_count = 0;

    std::cout << "\nAverage travel times between directly connected stations:\n";

    for (size_t i = 0; i < graph.size(); i++) {
        for (size_t j = 0; j < graph.size(); j++) {
            if (graph.edge_exists(i, j)) {
                double travel_time = graph.weight_between_vertices(i, j);

                std::cout << "Station " << i
                        << " -> Station " << j
                        << ": " << travel_time
                        << " minutes\n";

                total_time += travel_time;
                connection_count++;
            }
        }
    }

    if (connection_count > 0) {
        double average = total_time / connection_count;

        std::cout << "\nOverall average travel time: "
                << average << " minutes\n";
    }

    return EXIT_SUCCESS;
}