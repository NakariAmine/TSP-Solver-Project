#include"parser.hpp"

parser::parser(const std::string& s):s(s){

}
void parser::read_file(){
    std::ifstream file(s);
    if(!file.is_open()){
        std::cout<<"file is empty or nos avaible"<<"\n";

    }
    std::string lines;
    while(std::getline(file,lines)){
        file_lines.push_back(lines);
    }

}
void parser::parse_metadata(){
    for(const auto& line  : file_lines){
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
void parser::find_coord_section(){
    for (int i = 0; i < file_lines.size(); i++) {
        if (file_lines[i] == "NODE_COORD_SECTION"){
            coord_section_index=i;

        }
}
}
void parser::parse_data(){
    for(int i= coord_section_index+1;i<file_lines.size();i++){

        if(file_lines[i]=="EOF"){
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
void parser::write_trivial_solution(const std::string& filename) {
    std::ofstream out(filename);

    if (!out.is_open()) {
        std::cout << "Erreur : impossible d'ouvrir le fichier solution.\n";
        return;
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
}


