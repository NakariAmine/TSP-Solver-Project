#include <iostream>
#include "parser.hpp"

int main() {

    // créer un parser avec le nom du fichier
    parser p("berlin52.tsp");

    // lire le fichier
    p.read_file();

    // parser les métadonnées
    p.parse_metadata();

    // trouver la section des coordonnées
    p.find_coord_section();

    // parser les coordonnées
    p.parse_data();

    // afficher les métadonnées
    std::cout << "=== METADATA ===" << std::endl;
    for (const auto& kv : p.metadata) {
        std::cout << kv.first << " : " << kv.second << std::endl;
    }

    // afficher les coordonnées
    std::cout << "\n=== COORDONNEES ===" << std::endl;
    for (int i = 0; i < p.xs.size(); i++) {
        std::cout << i+1 << " -> x = " << p.xs[i]
                  << ", y = " << p.ys[i] << std::endl;
    }

    p.write_trivial_solution("mehdi.txt");
}
