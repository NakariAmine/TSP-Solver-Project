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

namespace fs = std::filesystem;

struct TSPData {
    size_t dimension = 0;
    std::vector<std::vector<double>> matrix;
    std::string type;
};

std::pair<std::string, TSPData> parse_tsp(const std::string& filename);

int main()
{
    const fs::path folder = "../TSP-instances/lower_diag";
    std::map<std::string, TSPData> instances;

    for (const auto& entry : fs::directory_iterator(folder)) {
        if (entry.is_regular_file() &&
            entry.path().extension() == ".tsp") {

            std::cout << "Reading: " << entry.path() << '\n';

            auto [name, data] = parse_tsp(entry.path().string());
            instances.insert_or_assign(name, std::move(data));
        }
    }

    for (const auto& [name, data] : instances) {
        std::cout << name
                  << " | Dimension: " << data.dimension
                  << " | Type: " << data.type << '\n';
    }
}

std::pair<std::string, TSPData> parse_tsp(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string name;
    TSPData data;
    std::string line;

    while (std::getline(file, line)) {
        // Read the first word to identify a section marker.
        std::istringstream line_stream(line);
        std::string first_word;
        line_stream >> first_word;

        if (first_word == "EDGE_WEIGHT_SECTION") {
            if (name.empty() || data.dimension == 0 ||
                data.type != "TSP") {
                throw std::runtime_error("Missing or invalid header");
            }

            data.matrix = std::vector<std::vector<double>>(
                data.dimension,
                std::vector<double>(data.dimension, 0.0)
            );

            for (size_t i = 0; i < data.dimension; ++i) {
                for (size_t j = 0; j <= i; ++j) {
                    double weight;

                    if (!(file >> weight)) {
                        throw std::runtime_error(
                            "Missing or invalid weight"
                        );
                    }

                    data.matrix[i][j] = weight;
                    data.matrix[j][i] = weight;
                }
            }

            std::ofstream output(filename + ".matrix.txt");

            if (!output) {
                throw std::runtime_error(
                    "Cannot open matrix output file"
                );
            }

            for (const auto& row : data.matrix) {
                for (double weight : row) {
                    output << std::setw(6) << weight;
                }
                output << '\n';
            }

            return {name, std::move(data)};
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
            value_stream >> data.type;
        } else if (key == "DIMENSION") {
            const int dimension = std::stoi(value);

            if (dimension <= 0) {
                throw std::runtime_error("Invalid DIMENSION");
            }

            data.dimension = static_cast<size_t>(dimension);
        } else if (key == "COMMENT") {
            std::cout << value << '\n';
        } else if (key == "EDGE_WEIGHT_TYPE") {
            std::string weight_type;
            value_stream >> weight_type;

            if (weight_type != "EXPLICIT") {
                throw std::runtime_error(
                    "This parser expects explicit weights"
                );
            }
        } else if (key == "EDGE_WEIGHT_FORMAT") {
            std::string format;
            value_stream >> format;

            if (format != "LOWER_DIAG_ROW") {
                throw std::runtime_error(
                    "This parser expects LOWER_DIAG_ROW"
                );
            }
        }
    }

    throw std::runtime_error("Missing EDGE_WEIGHT_SECTION");
}