#include "Parser.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

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
    size_t dimension = 0;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream line_stream(line);
        std::string first_word;
        line_stream >> first_word;

        if (first_word == "EDGE_WEIGHT_SECTION") {
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

            return {
                name,
                TSPData{
                    dimension,
                    AdjacenceMatrixWeightedGraph(matrix),
                    type
                }
            };
        }

        const std::string::size_type colon = line.find(':');

        // ChatGPT assistance, 2026-09-30: check whether ':' was not found.
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
            value_stream >> dimension;
        } else if (key == "COMMENT") {
            std::cout << value << '\n';
        }
    }

    throw std::runtime_error(
        "Missing EDGE_WEIGHT_SECTION in: " + filename
    );
}
