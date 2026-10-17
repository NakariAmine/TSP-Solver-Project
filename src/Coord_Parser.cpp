#include "Parser.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

CoordinateData parse_coordinates(const std::string& s)
{
    std::vector<std::string> file_lines;
    CoordinateData data;
    std::map<std::string, std::string>& metadata = data.metadata;
    int coord_section_index = 0;
    std::vector<double>& ys = data.ys;
    std::vector<double>& xs = data.xs;
    std::vector<int>& indexs = data.indexs;

    // read_file
    {
    std::ifstream file(s);
    if(!file.is_open()){
        std::cout<<"file is empty or nos avaible"<<"\n";

    }
    std::string lines;
    while(std::getline(file,lines)){
        file_lines.push_back(lines);
    }
    }

    // parse_metadata
    {
    for(const std::string& line  : file_lines){
        if (line=="NODE_COORD_SECTION"){
            break;
        }
    if(line.find(":")!=std::string::npos){
        std::string key=line.substr(0,line.find(":"));
        std::string value=line.substr(line.find(":"));
        metadata[key]=value;
    }
    }
    }

    // find_coord_section
    {
    for (int i = 0; i < static_cast<int>(file_lines.size()); i++) {
        if (file_lines[i] == "NODE_COORD_SECTION"){
            coord_section_index=i;

        }
}
    }

    // parse_data
    {
    for(int i= coord_section_index+1;i<static_cast<int>(file_lines.size());i++){

        if(file_lines[i]=="EOF" || file_lines[i]==" EOF"){ //added the second condition because the ulysses16.tsp file has some problems in the formatting!
            break;
        }
        std::stringstream ss(file_lines[i]);
        int index;
        double x;
        double y;
        ss>>index>>x>>y;
        indexs.push_back(index);
        xs.push_back(x);
        ys.push_back(y);


    }
    
    }

    return data;
}
