
#pragma once
#include "WeightedGraph.hpp"
#include "CompleteGraph.hpp"

#include <utility>
#include <vector>

double euclidean_distance(
    const std::pair<double,double>& x,
    const std::pair<double,double>& y);

double manhattan_distance(
    const std::pair<double,double>&x,
    const std::pair<double,double>&y);




class CoordinatesGraph :
    public WeightedGraph,
    public CompleteGraph
{
public:
    CoordinatesGraph(
        const std::vector<std::pair<double, double>>& input, double (*distance_function)(const std::pair<double,double>& ,const std::pair<double,double>&)
    );

    size_t size() const override;

    double weight_between_vertices(
        size_t i,
        size_t j
    ) const override;

    // double get_euclidian_distance(
    //     size_t i,
    //     size_t j
    // ) const;

private:
    std::vector<std::pair<double, double>> array;
    double (*distance_function)(
        const std::pair<double, double>&,
        const std::pair<double, double>&
    );

};