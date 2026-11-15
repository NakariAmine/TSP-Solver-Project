#include "Parser.hpp"
#include "Held_Karp.hpp"
#include "ACO_Solver.hpp"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>

AdjacenceMatrixWeightedGraph load_graph(const std::string& path) {
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream row(line);
        std::string section;
        row >> section;
        if (section == "NODE_COORD_SECTION") {
            return AdjacenceMatrixWeightedGraph(parse_coordinates(path).distances);
        }
        if (section == "EDGE_WEIGHT_SECTION") return parse_tsp(path).second.graph;
    }
    throw std::runtime_error("Cannot read a supported instance: " + path);
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) return 1;
        AdjacenceMatrixWeightedGraph graph = load_graph(argv[1]);
        std::string algorithm = argv[2];
        ACOConfig config;
        if (algorithm == "aco-only") {
            config.mode = ACOMode::ACOOnly;
        } else {
            config.mode = ACOMode::AdaptiveBandit;
        }
        TourResult result;
        std::vector<std::size_t> tour;

        auto start = std::chrono::steady_clock::now();
        if (algorithm == "trivial") {
            tour.resize(graph.size());
            std::iota(tour.begin(), tour.end(), 0);
        } else if (algorithm == "held-karp") {
            tour = held_karp(graph);
        } else {
            result = aco_solve(graph, config);
        }
        auto end = std::chrono::steady_clock::now();
        if (algorithm == "aco-only" || algorithm == "aco-optimized") {
            tour = std::move(result.tour);
        }

        if (tour.size() != graph.size()) throw std::runtime_error("Invalid tour size");
        std::vector<bool> seen(graph.size(), false);
        for (std::size_t city : tour) {
            if (city >= graph.size() || seen[city]) throw std::runtime_error("Invalid tour");
            seen[city] = true;
        }
        double length = 0;
        if (tour.size() > 1) {
            length = graph.total_weight(tour)
                + graph.weight_between_vertices(tour.back(), tour.front());
        }
        if (!std::isfinite(length)) throw std::runtime_error("Non-finite tour length");
        double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
        std::cout << std::setprecision(17) << "RESULT {\"time_ms\":" << elapsed
                  << ",\"length\":" << length << ",\"tour\":[";
        for (std::size_t i = 0; i < tour.size(); ++i) {
            if (i) std::cout << ',';
            std::cout << tour[i];
        }
        std::cout << "]}\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
