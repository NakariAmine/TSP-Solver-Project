# Guide to reading the project

English translation with ChatGPT/Codex assistance, 2026-11-15.

This guide accompanies the C++ code. It explains where to look and what each
part does, without replacing the design PDF.

Integration note: this guide describes the original project. Its entry point was
`src/Trivial_Solver.cpp`; this project's entry point is `src/main.cpp`. The ACO
CLI options, comparison script, trace export, and extended tests mentioned below
have not been integrated here yet.

## 1. The problem being solved

The TSP (*Travelling Salesperson Problem*) asks for a tour that visits each city
exactly once and then returns to its starting city. The objective is to minimize
the sum of the distances traveled.

The project targets symmetric TSP: traveling from city A to city B costs the same
as traveling from B to A. VRP capacity constraints are not implemented.

## 2. Important files and folders

- `src/ACO_Solver.cpp`: the ACO algorithm and local-search procedures.
- `include/ACO_Solver.hpp`: settings, modes, and results exposed to other files.
- `src/Trivial_Solver.cpp` in the original project: the entry point (`main`), terminal options, and tour output. It is now `src/main.cpp` here.
- `src/Coord_Parser.cpp`: reads coordinate instances and calculates distances.
- `src/Parser_Matrix.cpp`: reads instances whose distances are already supplied in a matrix.
- `tests/tests.cpp`: automated tests of essential behavior in the original project; this project's existing tests currently cover parsing.
- `benchmarks/compare_methods.py` in the original project: runs the comparison experiments described in the PDF.
- `TSP-instances/`: test instances.
- `ACO_notes/ACO_design.pdf`: the mathematical design, originally stored as `Notes/ACO_design.pdf`.

## 3. Solver data structures

In `ACO_Solver.cpp`, the `Ant` structure represents one ant:

```cpp
struct Ant {
    std::vector<std::size_t> tour;
    double length = std::numeric_limits<double>::infinity();
};
```

- `tour` is a list of city indices in visit order.
- `length` is the total length, including the edge returning to the first city.
- Infinity is the initial value before the tour is evaluated.

`ACOConfig` groups the algorithm settings, such as the number of ants, the number
of iterations, and the number `q` of best ants that receive local search.

`TourResult` groups the result and statistics: best tour, length, iterations,
candidate evaluations, and bandit information.

## 4. One ACO iteration, step by step

The main function is `aco_solve(...)` in `src/ACO_Solver.cpp`.

1. **Construction:** each ant chooses a starting city, then adds unvisited cities until it has a complete tour.
2. **Sorting:** ants are ordered by increasing tour length. The best come first.
3. **Online local search:** in the integrated modes, the top `q` tours are improved before pheromone deposition.
4. **Best-solution update:** retain the shortest tour found so far.
5. **Evaporation:** pheromones decrease according to the rate `rho`.
6. **Deposition:** each ant adds pheromone to the edges of its tour. Shorter tours deposit more because the deposit is `Q / length`.
7. **MMAS bounds:** constrain pheromones between minimum and maximum values.
8. **Stagnation:** if no new best solution appears over a configured period, a partial restart can rebalance the pheromones.

Repeat until reaching the maximum number of iterations or satisfying the stopping
condition based on local-search gains.

## 5. Choosing the next city

An ant at city `i` considers only unvisited cities. The weight for candidate city
`j` is:

```text
weight = pheromone(i,j)^a * (1 / distance(i,j))^b
```

- Stronger pheromone makes an edge more attractive.
- A shorter distance makes an edge more attractive.
- `a` controls the influence of pheromone.
- `b` controls the influence of distance.

`std::discrete_distribution` randomly chooses a city using these weights. This
allows ants to make different choices instead of always taking the same route.

## 6. The three local-search operators

### 2-opt

`two_opt(...)` selects two edges, replaces them with two new edges, and reverses
the order of the cities between them. Keep a change if it shortens the tour.

### Or-opt

`or_opt(...)` takes a segment of 1, 2, or 3 cities and tries relocating it elsewhere
in the tour. Keep a relocation only if the total length decreases.

### VND

`apply_local_search(...)` runs 2-opt followed by Or-opt when the selected arm is VND.

The functions also return the number of candidate moves evaluated. This measures
the computational effort spent on local search.

## 7. The Beta bandit explained simply

The bandit chooses among three arms: 2-opt, Or-opt, and VND. Each arm stores two
parameters, `alpha` and `beta`, initialized to 1.

For each call:

1. Draw a random value from each arm's Beta distribution.
2. Select the arm with the highest draw.
3. Apply that operator to the tour.
4. Calculate a reward from the reduction in length and the number of candidate evaluations.
5. Update `alpha` and `beta` for the selected arm.

First calculate the gain relative to the original tour length, then divide by the
cost in candidate evaluations. Finally, `r / (1 + r)` maps the reward into `[0, 1)`.

The forgetting factor `gamma` slightly reduces the influence of old observations.

Two C++ details are more technical:

- `std::gamma_distribution` generates a Beta sample using two independent Gamma samples; the C++ standard library has no direct Beta generator.
- `std::array<BetaArm, 3>` stores the three arms in a fixed-size array containing three `BetaArm` structures.

## 8. Why are some more advanced C++ structures used?

The code uses `std::map`, `std::deque`, `std::array`, and `std::mt19937` to avoid
reimplementing reliable facilities already supplied by C++.

- `std::deque<bool>` stores recent local-search outcomes for the sliding stopping window.
- `std::map` lets the sequential method identify tours already present in its archive.
- `std::mt19937` is the pseudo-random generator; its `seed` makes a run reproducible.

These structures serve specific purposes. The original code was commented, and
some compact constructs (lambdas, `std::clamp`, and `std::none_of`) were replaced
with named functions and more explicit `if` statements.

## 9. Building and checking the project

From the project root, containing `CMakeLists.txt`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The following commands belong to the original project's ACO CLI and comparison
script. They are retained as translated reference examples and do not work in
this project until that integration is completed.

Run adaptive mode on Berlin52:

```bash
./build/TSPsolver TSP-instances/coord/berlin52.tsp /tmp/berlin52.tour -m aco-bandit
```

Compare configurations:

```bash
python3 benchmarks/compare_methods.py \
  --runs 3 \
  --instances berlin52,att48,eil76 \
  --ants 30 \
  --iterations 100 \
  --q 2 \
  --output results/quick-comparison.csv
```

A successful test verifies the behavior checked for those test cases. It does not
prove that the bandit is always better; benchmark results must be examined
separately.
