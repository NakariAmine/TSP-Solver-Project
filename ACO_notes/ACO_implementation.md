# Mapping the code to the ACO design PDF

English translation with ChatGPT/Codex assistance, 2026-11-15.
These notes describe the original implementation. Its ACO CLI options, trace
export, comparison script, and extended tests have not been integrated into this
project. The solver code is in `src/ACO_Solver.cpp` and `include/ACO_Solver.hpp`.

## Integrated algorithm

- `construct_tour()` applies the transition rule `tau_ij^a * eta_ij^b`, where `eta = 1 / distance`, among unvisited vertices.
- The `aco-bandit` mode sorts the `m` tours, applies one of the local-search arms to the top `q`, updates the bandit, then evaporates/deposits pheromone using the improved tours (top-q) and raw tours (the others).
- The arms are 2-opt, Or-opt (segments of 1, 2, or 3 vertices), and VND (2-opt followed by Or-opt).
- Each arm has a `Beta(alpha, beta)` distribution initialized to `(1, 1)`. A theta value is sampled for each arm using independent Gamma variables, and the arm with the largest theta is chosen.
- Implemented reward: `g = max(0, L0 - L1)`, `r = (g / max(L0, epsilon_L)) / max(c, epsilon_c)`, and `r_tilde = r / (1 + r)`. The update is `alpha = gamma * alpha + r_tilde` and `beta = gamma * beta + (1 - r_tilde)`.
- The stopping window counts local-search calls, not ACO iterations. `--window 0` disables this stopping condition so methods can be compared over the same number of iterations.
- Evaporation is followed by deposits of `Q / Lk`. After deposition, each pheromone value is bounded by `tau_min = tau_max / (2n)` and `tau_max = 1 / (rho * L_best)`.
- The partial restart recommended in section 9 of the PDF is optional (`--restart-interval 0` disables it). A restart occurs only after stagnation and at an interval boundary.

## Experimental variants

- `aco-only`: ACO construction without local search.
- `aco-fixed-2opt`: online local search on the top-q tours using fixed 2-opt.
- `aco-bandit`: the document's integrated algorithm.
- `aco-sequential`: no local search during the ACO loop; an archive of the best distinct tours (up to `q * T_ACO`) receives local search after the ACO phase.
- `aco-fixed-oropt` and `aco-fixed-vnd` are additional baselines for testing the three arms.
- In the original project, `benchmarks/compare_methods.py` implements the overall comparison in section 10.3 and writes runtime, length, candidate evaluations, and local-search calls to CSV. `--trace` exports diversity and convergence diagnostics for each iteration.

## Important interpretation

Automated tests check compilation, inputs, numerical safeguards, tour
permutations, reproducibility, and structural behavior of the variants. They do
not prove that the adaptive algorithm is better than every baseline. That
conclusion must come from measurements across multiple instances and seeds.

Colony diversity is measured by the standard deviation of tour lengths in each
iteration. The result is stored in `TourResult`; the original project's `--trace`
option exports it alongside the best length and work counters.
