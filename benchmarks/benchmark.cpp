#include "Held_Karp.hpp"
#include "Parser.hpp"

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Instance {
    std::string path;
    AdjacenceMatrixWeightedGraph graph;
    double optimal_length;
};

AdjacenceMatrixWeightedGraph load_graph(const std::string& path)
{
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open instance: " + path);

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream row(line);
        std::string section;
        row >> section;
        if (section == "NODE_COORD_SECTION") {
            CoordinateData data = parse_coordinates(path);
            return AdjacenceMatrixWeightedGraph(data.distances);
        }
        if (section == "EDGE_WEIGHT_SECTION") {
            return parse_tsp(path).second.graph;
        }
    }
    throw std::runtime_error("Missing supported data section: " + path);
}

// Validate the returned permutation, then include the closing edge in its cost.
double tour_length(const AdjacenceMatrixWeightedGraph& graph,
                   const std::vector<std::size_t>& tour)
{
    if (tour.size() != graph.size()) {
        throw std::runtime_error("Solver returned a tour with the wrong number of cities");
    }
    std::vector<bool> seen(graph.size(), false);
    for (std::size_t city : tour) {
        if (city >= graph.size() || seen[city]) {
            throw std::runtime_error("Solver returned an invalid city or repeated a city");
        }
        seen[city] = true;
    }
    if (tour.size() < 2) return 0.0;

    double length = graph.total_weight(tour)
        + graph.weight_between_vertices(tour.back(), tour.front());
    if (!std::isfinite(length)) {
        throw std::runtime_error("Solver returned a tour with non-finite length");
    }
    return length;
}

std::string csv_field(const std::string& value)
{
    std::string result = "\"";
    for (char character : value) {
        if (character == '"') result += '"';
        result += character;
    }
    return result + '"';
}

int main(int argc, char* argv[])
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <output.csv> <runs> <instance_path> [instance_path ...]\n";
        return 1;
    }

    try {
        const std::filesystem::path output_path(argv[1]);
        if (std::filesystem::exists(output_path)) {
            throw std::runtime_error("Output already exists; choose a new CSV path: "
                                     + output_path.string());
        }
        std::size_t consumed = 0;
        const int runs = std::stoi(argv[2], &consumed);
        if (runs < 1 || consumed != std::string(argv[2]).size()) {
            throw std::runtime_error("Runs must be a positive integer");
        }

        // Parse and warm up every instance before creating the output file.
        std::vector<Instance> instances;
        for (int argument = 3; argument < argc; ++argument) {
            const std::string path = argv[argument];
            AdjacenceMatrixWeightedGraph graph = load_graph(path);
            if (graph.size() > 20) {
                throw std::runtime_error("Held-Karp benchmark supports at most 20 cities: " + path);
            }

            // One untimed solver run also supplies the exact optimum baseline.
            std::vector<std::size_t> warmup_tour = held_karp(graph);
            double optimum = tour_length(graph, warmup_tour);
            instances.push_back({path, std::move(graph), optimum});
        }

        std::ofstream output(output_path);
        if (!output) throw std::runtime_error("Cannot open output CSV: " + output_path.string());
        output << "instance,cities,algorithm,run,seed,runtime_ms,tour_length,optimal_length,gap_percent\n";
        output << std::setprecision(12);

        for (const Instance& instance : instances) {
            for (int run = 1; run <= runs; ++run) {
                const auto start = std::chrono::steady_clock::now();
                std::vector<std::size_t> tour = held_karp(instance.graph);
                const auto end = std::chrono::steady_clock::now();

                double runtime_ms = std::chrono::duration<double, std::milli>(end - start).count();
                double length = tour_length(instance.graph, tour);
                if (length != instance.optimal_length) {
                    throw std::runtime_error("Held-Karp cost changed between runs: " + instance.path);
                }

                // Held-Karp is deterministic: seed is empty and its optimality gap is 0.
                output << csv_field(instance.path) << ',' << instance.graph.size()
                       << ",held-karp," << run << ",," << runtime_ms << ',' << length
                       << ',' << instance.optimal_length << ",0\n";
            }
            std::cout << instance.path << ": " << runs << " runs, optimal length "
                      << instance.optimal_length << '\n';
        }

        output.close();
        if (!output) throw std::runtime_error("Failed to write output CSV");
        std::cout << "Results saved to " << output_path.string() << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
