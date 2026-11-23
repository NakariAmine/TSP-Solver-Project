# TSP solver demo commands

Run these commands from the project root.

## Build

```bash
bash build-TSPsolver.sh
```

## Write tour files for gr17

```bash
./build/TSPsolver TSP-instances/lower_diag/gr17.tsp gr17_trivial.tour -m trivial
./build/TSPsolver TSP-instances/lower_diag/gr17.tsp gr17_held_karp.tour -m held-karp
./build/TSPsolver TSP-instances/lower_diag/gr17.tsp gr17_aco.tour -m aco
```

The `aco` option runs enhanced ACO.

## Compare all four methods on gr17

```bash
./build/compare_algorithms TSP-instances/lower_diag/gr17.tsp trivial
./build/compare_algorithms TSP-instances/lower_diag/gr17.tsp held-karp
./build/compare_algorithms TSP-instances/lower_diag/gr17.tsp aco-only
./build/compare_algorithms TSP-instances/lower_diag/gr17.tsp aco-optimized
```

These commands print the time, length, and tour. The verified tour lengths were **4722**, **2085**, **2094**, and **2085**, respectively.

## Benchmark every instance

```bash
python3 compare_algorithms.py --output demo_comparison.md
```

Use a fresh output filename for each run.
