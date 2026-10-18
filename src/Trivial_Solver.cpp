#include "Parser.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

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

    output.close();
    if (!output) {
        throw std::runtime_error("Failed to write tour: " + filename);
    }
}

void write_coordinate_tour(const std::string& s, const std::string& filename,
                           const std::vector<int>& indexs)
{
    std::ofstream out(filename);

    if (!out.is_open()) {
        throw std::runtime_error("Cannot open output file: " + filename);
    }

    out << "NAME : "<<s<<"\n";
    out << "TYPE : TOUR\n";
    out << "DIMENSION : " << indexs.size() << "\n";
    out << "TOUR_SECTION\n";

    for (int id : indexs) {
        out << id << "\n";
    }

    out << "-1\n";
    out << "EOF\n";

    out.close();
    if (!out) {
        throw std::runtime_error("Failed to write tour: " + filename);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3 && argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <instance_path> <output_path> [-m trivial]\n";
        return 1;
    }
    if (argc == 5 && (std::string(argv[3]) != "-m" ||
                      std::string(argv[4]) != "trivial")) {
        std::cerr << "Only the trivial method is available for now.\n"; 
        return 1;
    }

    try {
        std::filesystem::path output_path(argv[2]);

        std::ifstream file(argv[1]);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + std::string(argv[1]));
        }

        std::string line;
        std::string section;
        while (std::getline(file, line)) {
            std::istringstream row(line);
            row >> section;
            if (section == "NODE_COORD_SECTION" || section == "EDGE_WEIGHT_SECTION") {
                break;
            }
        }

        if (section == "NODE_COORD_SECTION") {
            CoordinateData data = parse_coordinates(argv[1]);
            write_coordinate_tour(std::filesystem::path(argv[1]).stem().string() + ".tour",
                                  output_path.string(), data.indexs);
        } else if (section == "EDGE_WEIGHT_SECTION") {
            std::pair<std::string, TSPData> result = parse_tsp(argv[1]);
            std::string name = result.first;
            TSPData data = result.second;
            std::vector<size_t> tour = trivial_solution(data.graph);
            write_tour(output_path.string(), name, tour);
        } else {
            throw std::runtime_error("Missing supported data section");
        }
        std::cout << "Tour saved to " << output_path.string() << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
