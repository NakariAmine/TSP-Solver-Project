# TSP Solver

Currently implements the trivial method: Visit cities in ascending ID order.
## Build

bash build-TSPsolver.sh

Creates `build/TSPsolver` and `build/tests`.

## Run tests

./build/tests

Runs two parser tests: gr17 (matrix) and berlin52 (coordinates).

## Run the solver

./build/TSPsolver <instance_path> <output_path>

or 

./build/TSPsolver <instance_path> <output_path> -m trivial

Examples:

./build/TSPsolver TSP-instances/coord/berlin52.tsp berlin52.tour
./build/TSPsolver TSP-instances/lower_diag/gr17.tsp gr17.tour -m trivial


