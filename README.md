# TSP Solver

A C++20 solver for the Travelling Salesman Problem, with a Python script for comparing execution time and tour length.

## Requirements

- A C++20 compiler
- CMake 3.14 or later
- Bash
- Python 3.9 or later for the comparison script (standard library only)

Run the commands below from the project root.

## Build

```bash
bash build-TSPsolver.sh
```

Creates `build/TSPsolver`, `build/tests`, and `build/compare_algorithms`.

## Run tests

```bash
./build/tests
```

The tests check matrix parsing with `gr17` and coordinate parsing with `berlin52`.

## Run the solver

```bash
./build/TSPsolver <instance_path> <output_path> [-m <method>]
```

| Method | Description |
|---|---|
| `trivial` | Visits cities in their input/index order. This is the default method. |
| `held-karp` | Exact dynamic programming algorithm. This implementation supports at most 20 cities. |
| `aco` | Ant colony optimization with a bandit choosing between 2-opt, Or-opt, and their combination. |

Held–Karp's 20-city guard limits memory use and execution time; it is not a mathematical limit of the algorithm.

ACO uses seed 7, 30 ants, and at most 200 iterations by default. It applies local search to the best two tours each iteration. Each selected operator makes one pass without restarting after improvements. The combined operator performs one 2-opt pass followed by one Or-opt pass.

Examples:

```bash
# Default trivial method on a coordinate instance
./build/TSPsolver TSP-instances/coord/berlin52.tsp berlin52.tour

# Held–Karp on a matrix instance
./build/TSPsolver TSP-instances/lower_diag/gr17.tsp gr17.tour -m held-karp

# Enhanced ACO on a coordinate instance
./build/TSPsolver TSP-instances/coord/berlin52.tsp berlin52_aco.tour -m aco
```

The output is a TSPLIB-style TOUR file. Its parent directory must already exist. Plain ACO is available through the comparison runner; `-m aco` selects enhanced ACO.

## Supported inputs

- Coordinate files with `NODE_COORD_SECTION`, using `EUC_2D`, `ATT`, or `GEO` distances. If no distance type is specified, `EUC_2D` is used.
- Explicit distance matrices with `EDGE_WEIGHT_SECTION` and `EDGE_WEIGHT_FORMAT: LOWER_DIAG_ROW`.

## Compare the algorithms

```bash
python3 compare_algorithms.py --output new_results.md
```

This builds the C++ comparison runner and runs four methods on every `.tsp` file under `TSP-instances`:

1. Trivial
2. Held–Karp
3. Ant colony only, without local search
4. Ant colony with single-pass local optimization

The Markdown output contains a table for each instance, showing its city count, input format, distance type, execution time, tour length, and completion status. The current results are in [algorithm_comparison.md](algorithm_comparison.md).

Each method runs once per instance. Both ACO variants use the same default settings and seed. Solver timing excludes file parsing, distance calculation, output, and tour validation. Tour length includes the closing edge back to the starting city; smaller is better.

Held–Karp is marked “Did not run” above 20 cities. Each process has a 120-second timeout, including startup and parsing. Timed-out runs are marked explicitly. This timeout belongs to the comparison script; it does not apply to normal `TSPsolver` commands.

Without `--output`, it uses `algorithm_comparison.md`.

To choose an instance folder or change the timeout:

```bash
python3 compare_algorithms.py --instances-dir TSP-instances/coord --timeout 180 --output coordinate_results.md
```

## Main files

| Path | Purpose |
|---|---|
| `src/` and `include/` | Graph representation, parsers, solvers, and command-line program |
| `tests/` | Parser tests |
| `TSP-instances/` | Input instances |
| `compare_algorithms.cpp` | Calls the existing solvers, times them, validates tours, and calculates tour lengths |
| `compare_algorithms.py` | Runs the comparison, handles timeouts, and writes Markdown tables |
| `build-TSPsolver.sh` | Configures and builds the project using CMake |
