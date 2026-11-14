#include "Parser.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>

namespace {
std::string trim(const std::string& value)
{
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

double geo_radians(double coordinate)
{
    const double pi = 3.141592;
    const double degrees = std::trunc(coordinate);
    const double minutes = coordinate - degrees;
    return pi * (degrees + 5.0 * minutes / 3.0) / 180.0;
}

std::vector<std::vector<double>> calculate_distances(const CoordinateData& data)
{
    const std::string type = data.metadata.at("EDGE_WEIGHT_TYPE");
    if (type != "EUC_2D" && type != "ATT" && type != "GEO") {
        throw std::runtime_error("Unsupported EDGE_WEIGHT_TYPE: " + type);
    }

    const std::size_t n = data.indexs.size();
    std::vector<std::vector<double>> distances(n, std::vector<double>(n, 0.0));

    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < i; ++j) {
            double distance;

            if (type == "GEO") {
                const double latitude_i = geo_radians(data.xs[i]);
                const double longitude_i = geo_radians(data.ys[i]);
                const double latitude_j = geo_radians(data.xs[j]);
                const double longitude_j = geo_radians(data.ys[j]);
                const double q1 = std::cos(longitude_i - longitude_j);
                const double q2 = std::cos(latitude_i - latitude_j);
                const double q3 = std::cos(latitude_i + latitude_j);
                double cosine = 0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3);
                cosine = std::clamp(cosine, -1.0, 1.0);
                distance = std::floor(6378.388 * std::acos(cosine) + 1.0);
            } else {
                const double dx = data.xs[i] - data.xs[j];
                const double dy = data.ys[i] - data.ys[j];
                if (type == "ATT") {
                    const double raw = std::sqrt((dx * dx + dy * dy) / 10.0);
                    const double rounded = std::floor(raw + 0.5);
                    distance = rounded < raw ? rounded + 1.0 : rounded;
                } else {
                    distance = std::floor(std::hypot(dx, dy) + 0.5);
                }
            }

            if (!std::isfinite(distance)) {
                throw std::runtime_error("Coordinate distance is not finite");
            }
            distances[i][j] = distance;
            distances[j][i] = distance;
        }
    }
    return distances;
}
}

// records, and calculation of the full distance matrix before returning.
CoordinateData parse_coordinates(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    CoordinateData data;
    bool in_coordinates = false;
    std::set<int> ids;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line == "EOF") break;
        if (line == "NODE_COORD_SECTION") {
            in_coordinates = true;
            continue;
        }

        if (!in_coordinates) {
            const std::size_t colon = line.find(':');
            if (colon != std::string::npos) {
                const std::string key = trim(line.substr(0, colon));
                const std::string value = trim(line.substr(colon + 1));
                data.metadata[key] = value;
            }
        } else {
            std::istringstream row(line);
            int id;
            double x, y;
            if (!(row >> id >> x >> y) || id < 0 || !ids.insert(id).second
                || !std::isfinite(x) || !std::isfinite(y)) {
                throw std::runtime_error("Invalid or duplicate coordinate record in: " + filename);
            }
            data.indexs.push_back(id);
            data.xs.push_back(x);
            data.ys.push_back(y);
        }
    }

    if (!in_coordinates) {
        throw std::runtime_error("Missing NODE_COORD_SECTION in: " + filename);
    }
    std::size_t dimension = 0;
    std::istringstream dimension_stream(data.metadata["DIMENSION"]);
    if (!(dimension_stream >> dimension) || dimension == 0 || dimension != data.indexs.size()) {
        throw std::runtime_error("DIMENSION does not match coordinate count in: " + filename);
    }
    if (data.metadata.find("EDGE_WEIGHT_TYPE") == data.metadata.end()) {
        data.metadata["EDGE_WEIGHT_TYPE"] = "EUC_2D";
    }

    data.distances = calculate_distances(data);
    return data;
}
