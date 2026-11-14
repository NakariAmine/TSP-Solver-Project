# Solver benchmarks

The benchmark is separate from the solver. It currently measures Held-Karp;
Ant Colony has not been implemented or measured yet.

## Build and run

Run from the project root. The normal build script also creates `build/benchmark`.
Use a Release build for performance measurements:

```bash
bash build-TSPsolver.sh
./build/benchmark benchmarks/held-karp-results.csv 5 \
    TSP-instances/generated/sample-0.tsp \
    TSP-instances/coord/burma14.tsp \
    TSP-instances/coord/ulysses16.tsp \
    TSP-instances/lower_diag/gr17.tsp
```

Arguments are the output CSV path, number of measured runs per instance, and one
or more instance paths. The parent output folder must exist. Existing CSV files
are not overwritten. Every instance must contain at most 20 cities; oversized
instances fail before the CSV is created. The existing graph class rejects
zero-cost edges; this benchmark does not change that behavior.

If the build folder was previously configured for Debug, select Release explicitly:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## What is measured

Each instance is parsed once and its distance graph is constructed before timing.
The benchmark performs one untimed Held-Karp run per instance, then the requested
number of measured runs. Timing uses `std::chrono::steady_clock` around the solver
call only. It includes the solver's table allocation, dynamic programming,
reconstruction, and cleanup, but excludes parsing, distance calculation, tour
validation, tour-length calculation, console messages, and CSV output.

The returned tour must visit every city exactly once. Its length includes the
closing edge back to the starting city. An empty or single-city tour has length 0.

The CSV contains one row per measured run:

```text
instance,cities,algorithm,run,seed,runtime_ms,tour_length,optimal_length,gap_percent
```

`optimal_length` is the result of the untimed exact Held-Karp run, rather than an
external certificate of optimality. Its own `gap_percent` is therefore 0.
Held-Karp does not use randomness, so `seed` is empty. Repeated runtimes can vary
even when every run returns the same length.

## Comparing with Ant Colony later

Once its function exists, call it on the same loaded graph and measure the same
solver-only interval. Record its algorithm name, one reproducible random seed
per run, and all algorithm parameters (ants, iterations, evaporation, etc.).
Keep the Held-Karp optimum as the baseline and calculate:

```text
gap_percent = 100 * (tour_length - optimal_length) / optimal_length
```

For a zero optimum, report a zero gap only for a zero-length tour; otherwise leave
the percentage undefined. Do not divide by zero. Larger instances can be timed
for Ant Colony separately, but should not be labeled as comparisons against a
Held-Karp optimum unless that optimum is available.

For the evaluation, summarize repeated runtimes with the median and spread;
summarize Ant Colony quality across its seeds as well as its runtime. Compare
runtime against city count to examine scalability. Use multiple instances of the
same size when possible, and keep the machine, compiler, build type, timing scope,
and background workload consistent. These measurements exclude preprocessing, so
they do not represent the total CLI runtime.

ChatGPT/Codex assistance, 2026-11-14: benchmark implementation and usage documentation.
