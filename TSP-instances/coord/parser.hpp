#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

class parser {
public:
    std::string s;
    std::vector<std::string> file_lines;
    std::map<std::string, std::string> metadata;
    int coord_section_index;
    std::vector<double> ys;
    std::vector<double> xs;
    std::vector<int> indexs;


    parser(const std::string& s);
    void write_trivial_solution(const std::string& filename);
    void read_file();
    void parse_metadata();
    void find_coord_section();
    void parse_data();
};

#endif
