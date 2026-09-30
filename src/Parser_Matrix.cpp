#include "AdjacenceMatrixWeightedGraph.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Assisted by ChatGPT, 2026-09-30.
std::vector<size_t> trivial_solution(
    const AdjacenceMatrixWeightedGraph& graph
)
{
    std::vector<size_t> tour;

    for (size_t i = 0; i < graph.size(); ++i) {
        tour.push_back(i);
    }

    return tour;
}

void write_tour(
    const std::string& filename,
    const std::string& name,
    const std::vector<size_t>& tour
)
{
    std::ofstream output(filename);

    if (!output) {
        throw std::runtime_error("Cannot open output file: " + filename);
    }

    output << "NAME : " << name << ".tour\n";
    output << "TYPE : TOUR\n";
    output << "DIMENSION : " << tour.size() << '\n';
    output << "TOUR_SECTION\n";

    for (size_t city : tour) {
        output << city + 1 << '\n';
    }

    output << "-1\n";
    output << "EOF\n";

    if (!output) {
        throw std::runtime_error("Failed to write tour: " + filename);
    }
}

namespace fs = std::filesystem;

// Assisted by ChatGPT, 2026-09-30.
struct TSPData {
    size_t dimension;
    AdjacenceMatrixWeightedGraph graph;
    std::string type;
};

std::pair<std::string, TSPData> parse_tsp(
    const std::string& filename
);

// Assisted by ChatGPT, 2026-09-30.
int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <instance_path> <output_path>\n";
        return 1;
    }

    try {
        auto [name, data] = parse_tsp(argv[1]);

        auto tour = trivial_solution(data.graph);
        write_tour(argv[2], name, tour);

        std::cout << "Tour saved to " << argv[2] << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}

std::pair<std::string, TSPData> parse_tsp(
    const std::string& filename
)
{
    std::ifstream file(filename);

    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string name;
    std::string type;
    std::string weight_type;
    std::string weight_format;
    size_t dimension = 0;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream line_stream(line);
        std::string first_word;
        line_stream >> first_word;

        if (first_word == "EDGE_WEIGHT_SECTION") {
            if (name.empty() || dimension == 0 || type != "TSP") {
                throw std::runtime_error("Missing or invalid header");
            }

            if (weight_type != "EXPLICIT" ||
                weight_format != "LOWER_DIAG_ROW") {
                throw std::runtime_error(
                    "Expected EXPLICIT weights in LOWER_DIAG_ROW format"
                );
            }

            // Build the full symmetric matrix.
            std::vector<std::vector<double>> matrix(
                dimension,
                std::vector<double>(dimension, 0.0)
            );

            for (size_t i = 0; i < dimension; ++i) {
                for (size_t j = 0; j <= i; ++j) {
                    double weight;

                    if (!(file >> weight)) {
                        throw std::runtime_error(
                            "Missing or invalid weight in: " + filename
                        );
                    }

                    matrix[i][j] = weight;
                    matrix[j][i] = weight;
                }
            }

            // Save the matrix for inspection.
            std::ofstream output(filename + ".matrix.txt");

            if (!output) {
                throw std::runtime_error(
                    "Cannot open matrix output file"
                );
            }

            for (const auto& row : matrix) {
                for (double weight : row) {
                    output << std::setw(6) << weight;
                }
                output << '\n';
            }

            // Construct your graph from the completed matrix.
            return {
                name,
                TSPData{
                    dimension,
                    AdjacenceMatrixWeightedGraph(matrix),
                    type
                }
            };
        }

        const auto colon = line.find(':');

        if (colon == std::string::npos) {
            continue;
        }

        std::istringstream key_stream(line.substr(0, colon));
        std::string key;
        key_stream >> key;

        const std::string value = line.substr(colon + 1);
        std::istringstream value_stream(value);

        if (key == "NAME") {
            value_stream >> name;
        } else if (key == "TYPE") {
            value_stream >> type;
        } else if (key == "DIMENSION") {
            const int parsed_dimension = std::stoi(value);

            if (parsed_dimension <= 0) {
                throw std::runtime_error("Invalid DIMENSION");
            }

            dimension = static_cast<size_t>(parsed_dimension);
        } else if (key == "COMMENT") {
            std::cout << value << '\n';
        } else if (key == "EDGE_WEIGHT_TYPE") {
            value_stream >> weight_type;
        } else if (key == "EDGE_WEIGHT_FORMAT") {
            value_stream >> weight_format;
        }
    }

    throw std::runtime_error(
        "Missing EDGE_WEIGHT_SECTION in: " + filename
    );
}