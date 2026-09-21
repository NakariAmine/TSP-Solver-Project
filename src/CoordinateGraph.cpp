#include "CoordinateGraph.hpp"
#include<cmath>


double euclidean_distance(
    const std::pair<double,double>& x,
    const std::pair<double,double>& y){

        double dx = x.first - y.first;
        double dy = x.second - y.second;

        return std::sqrt(dx * dx + dy * dy);
    }
    
double manhattan_distance( const std::pair<double,double>& x,
    const std::pair<double,double>& y){
        return ((std::abs(x.first - y.first) + std::abs(x.second - y.second)));

    }

CoordinatesGraph::CoordinatesGraph(
    const std::vector<std::pair<double, double>>& input, double (*function)(const std::pair<double,double>& ,const std::pair<double,double>&)) 
    : array(input), distance_function(function)

{
}



size_t CoordinatesGraph::size() const
{
    return array.size();
}





double CoordinatesGraph::weight_between_vertices(
    size_t i,
    size_t j
) const
{
    return distance_function(array[i], array[j]);

;
}
