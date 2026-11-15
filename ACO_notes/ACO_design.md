# Ant Colony Optimization with adaptive local search using a fractional Beta bandit

## Integrated architecture: improve the top-q ants before pheromone deposition

## Contents

1. Objective and general principle
2. Notation and parameters
3. Construction phase: ACO transition rule
4. Adaptive local search on the top-q ants
5. Pheromone deposition
6. Stopping condition
7. Pseudocode
8. Why improve the top-q tours before deposition?
9. Safeguards against stagnation
10. Planned tests
11. Pitfalls to keep in mind
12. Conclusion

## 1. Objective and general principle

Consider a tour-optimization problem (TSP or VRP) on a fixed, known instance.
Let `G = (V, E)` be the graph, `d_ij` the distance between vertices `i` and `j`,
and `L(S)` the length of a tour `S`.

The algorithm combines two complementary mechanisms in a single loop:

- An ant colony (ACO) constructs `m` pheromone-guided tours in each iteration.
- Adaptive local search, controlled by a fractional Beta bandit, is applied to the
  best `q` ants in each iteration, before pheromone deposition.

The essential difference from a sequential "ACO then local search" scheme is
that detours are corrected during the ACO run. Pheromone is deposited on tours
that have already been improved, guiding the colony toward better regions early
rather than reinforcing detours throughout the construction phase.

Why use a small q instead of improving every ant? Applying local search to all
`m` ants would require `m * T_ACO` calls. Besides its cost, this could reduce colony
diversity: tours might converge toward the same local optima, concentrating
pheromone in one region and causing premature stagnation. Improving only the
best `q` ants bounds the number of calls by `q * T_ACO`. The remaining ants deposit
pheromone on their raw tours, preserving exploration, while the bandit learns
from promising tours.

The current code implements symmetric TSP; the broader VRP discussion in this
design does not imply that capacity constraints are implemented.

## 2. Notation and parameters

- `tau_ij`: pheromone quantity on edge `(i, j)`.
- `eta_ij = 1 / d_ij`: proximity heuristic.
- `a`: influence of pheromone in the transition rule.
- `b`: influence of the distance heuristic.
- `rho` in `(0, 1]`: evaporation rate.
- `Q`: pheromone-deposit constant.
- `m`: number of ants per iteration.
- `q`: number of ants receiving local search per iteration, with `q << m`.
- `T_ACO`: maximum number of ACO iterations.
- `L_k`: length of ant k's tour.
- `gamma` in `(0, 1)`: bandit forgetting factor.
- `W`: stopping-window size, measured in local-search calls.
- `tau_min`, `tau_max`: pheromone bounds (MMAS).

Naming convention: ACO's parameters are called `a` and `b`, rather than `alpha`
and `beta`, to distinguish them from the Beta-distribution parameters used by
the bandit. This separation is intentional.

## 3. Construction phase: ACO transition rule

In each iteration, ant `k` builds a tour step by step. At vertex `i`, it chooses
the next vertex `j` from `N_i^k`, the set of unvisited vertices. In a VRP extension,
this set would also account for capacity feasibility.

```text
                     tau_ij^a * eta_ij^b
p_ij^k = ------------------------------------------------
         sum over l in N_i^k of (tau_il^a * eta_il^b)
```

When no candidate remains, the ant closes its tour by returning to the depot or
starting vertex.

Construction is short-sighted: it does not directly account for the detours it
creates. Local search is intended to correct these detours, but only for the
best `q` tours to control evaluation effort.

## 4. Adaptive local search on the top-q ants

### 4.1. Bandit arms

- **2-opt:** apply the 2-opt operator only.
- **Or-opt:** apply the Or-opt operator only.
- **VND:** apply 2-opt followed by Or-opt as one combined arm.

The VND arm lets the algorithm learn whether the complete 2-opt/Or-opt pipeline
is more useful than either operator alone for certain detours. Credit is assigned
to the entire pipeline, not to its individual sub-operators.

"Do nothing" is not a bandit arm. It is replaced by the separate stopping
condition in section 6.

### 4.2. Reward

For each local-search application, measure:

- `L0`: tour length before the operator.
- `L1`: tour length after the operator.
- `c`: number of candidate evaluations.
- For VND, `c = c_2opt + c_OrOpt`.

Define the gain and raw reward:

```text
g = max(0, L0 - L1)
r = (g / max(L0, epsilon_L)) / max(c, epsilon_c)
```

Then normalize the reward:

```text
r_tilde = r / (1 + r), in [0, 1)
```

Dividing by `L0` makes improvement relative to tour length. Dividing by `c`
penalizes computationally expensive operators. The final transformation bounds
the reward in the range used by the fractional Beta update. The epsilon terms
prevent division by zero.

### 4.3. Reminder: the Beta distribution

For `alpha > 0` and `beta > 0`, the Beta distribution has density on `[0, 1]`:

```text
f(x; alpha, beta) = x^(alpha-1) * (1-x)^(beta-1) / B(alpha, beta)
```

Here, `B(alpha, beta)` is Euler's beta function. Its mean is:

```text
E[X] = alpha / (alpha + beta)
```

The document uses the fractional-observation update:

```text
Beta(alpha, beta) -> Beta(alpha + y, beta + 1 - y), for y in [0, 1]
```

Non-abandonment rationale: a Beta distribution has positive density throughout
`(0, 1)` while both parameters are positive. Each arm therefore retains a positive
chance of producing a high draw, rather than being permanently excluded. This
is a statement about possible selection, not a guarantee of selection within a
particular finite window.

### 4.4. Selection and update

Each arm stores `(alpha_i, beta_i)`, initially `(1, 1)`. For each selected ant:

1. Draw `theta_i ~ Beta(alpha_i, beta_i)` independently for each arm.
2. Choose `i_star = argmax_i(theta_i)`.
3. Apply that operator, measure `g` and `c`, and calculate `r_tilde`.
4. Update the chosen arm:

```text
alpha_i_star = gamma * alpha_i_star + r_tilde
beta_i_star  = gamma * beta_i_star + (1 - r_tilde)
```

The forgetting factor `gamma` handles non-stationarity: the quality of incoming
tours changes during ACO, so older observations should gradually carry less weight.

What theta means: it is a random draw from the arm's current usefulness model,
not literally the probability that the operator will improve this particular
tour. The distribution represents uncertainty as well as estimated quality.
Random sampling allows exploration alongside preference for useful operators.

## 5. Pheromone deposition

After constructing tours and improving the top `q`, perform evaporation and
then deposition.

Evaporation:

```text
tau_ij = (1 - rho) * tau_ij, for every edge (i, j)
```

Each ant deposits on its tour `S_k`: improved for the top `q`, raw for the others.

```text
delta_tau_ij^k = Q / L_k  if edge (i, j) belongs to S_k
                 0        otherwise

tau_ij = tau_ij + sum over k=1..m of delta_tau_ij^k
```

MMAS bounds constrain every pheromone value to `[tau_min, tau_max]`:

```text
tau_max = 1 / (rho * L_best)
tau_min = tau_max / (2 * n), where n = |V|
```

These bounds are intended to prevent excessive concentration in one region and
preserve exploration when the best tours are repeatedly improved.

## 6. Stopping condition

Maintain a sliding window of improvement indicators from the last `W`
local-search calls. Stop when the window is full and none of those calls
produced a strictly positive gain:

```text
Stop if sum of [g_k > 0] over the last W calls equals 0.
```

The maximum iteration count `T_ACO` remains a safeguard against continuing
indefinitely on a flat instance.

The window counts local-search calls, not ACO iterations. With `q` calls per
iteration, it fills `q` times faster than a window measured in iterations.

## 7. Pseudocode

```text
Input: graph G; m, a, b, rho, Q, q, T_ACO, gamma, W, epsilon_L, epsilon_c
Output: best tour found

Set tau_ij = tau_0 for every edge (i, j).
Set S_best = empty.
For each arm in {2-opt, Or-opt, VND}:
    Set alpha_arm = 1 and beta_arm = 1.
Create an empty sliding window with capacity W.

For iteration = 1 to T_ACO:
    # 1. Construct m tours.
    For ant k = 1 to m:
        Construct S_k using the ACO transition probabilities.

    # 2. Sort ants by tour length, shortest first.
    Sort S_1, ..., S_m.

    # 3. Improve the top-q tours before depositing pheromone.
    For ant k = 1 to q:
        For each arm i in {2-opt, Or-opt, VND}:
            Draw theta_i from Beta(alpha_i, beta_i).
        Select i_star with the highest theta_i.
        Set L0 = L(S_k), and c = 0.
        Apply arm i_star to S_k, counting candidate evaluations in c.
        Set L1 = L(S_k).
        Set g = max(0, L0 - L1).
        Set r = (g / max(L0, epsilon_L)) / max(c, epsilon_c).
        Set r_tilde = r / (1 + r).
        Update alpha_i_star = gamma * alpha_i_star + r_tilde.
        Update beta_i_star = gamma * beta_i_star + (1 - r_tilde).
        Append the indicator [g > 0] to the window.
        If its size exceeds W, remove the oldest indicator.

    # 4. Update the global best.
    For ant k = 1 to m:
        If S_best is empty or L(S_k) < L(S_best): set S_best = S_k.

    # 5. Evaporate and deposit pheromone.
    Set tau_ij = (1 - rho) * tau_ij for every edge.
    For ant k = 1 to m:
        For each edge (i, j) of S_k: add Q / L_k to tau_ij.

    # 6. Apply MMAS bounds.
    Clamp each tau_ij between tau_min and tau_max.

    # 7. Check the stopping condition.
    If the window is full and contains no positive-gain indicator: stop.

Return S_best.
```

## 8. Why improve the top-q tours before deposition?

**Improved pheromone information.** Without online correction, detours in raw
constructed tours are reinforced by pheromone. Improving the top tours before
deposition lets those ants reinforce routes with fewer detours.

**More observations for the bandit.** With `q` calls per iteration and `T_ACO`
iterations, the bandit can receive `q * T_ACO` observations. For example, `q = 3`
and `T_ACO = 100` provide 300 observations. The source argues that this gives a
richer learning signal than applying local search only to a small final set.

**Preserving diversity.** The `m - q` uncorrected ants still deposit pheromone
on their raw tours. This is intended to maintain exploration. Correcting every
ant could instead make the colony converge prematurely. The document suggests
keeping `q` small, typically 1 to 3.

**Bounded local-search calls.** The number of calls is bounded by `q * T_ACO`,
independently of `m`. Increasing the number of ants therefore need not increase
the number of local-search calls. This bounds the number of calls, not the
number of candidate evaluations within each call.

## 9. Safeguards against stagnation

Improving the top-q tours can accelerate convergence but also concentrate
pheromone and increase the risk of premature stagnation. Three safeguards are
recommended:

1. MMAS bounds: keep `tau_ij` between `tau_min` and `tau_max`.
2. A small q: use 1 or 2 so most ants still deposit pheromone on raw tours.
3. Periodic diversification: partially reinitialize pheromone values, or restart
   after a long period without improvement.

The source refers to Dorigo and Stutzle, *Ant Colony Optimization* (2004), for
ACO stagnation safeguards, and emphasizes pheromone bounds in particular.

## 10. Planned tests

### 10.1. Unit tests

1. Numerical bounds: zero gain should give zero reward; zero length and zero
   candidate evaluations must not cause division by zero.
2. Reward range: verify `0 <= r_tilde < 1` for tested valid inputs.
3. ACO construction: every ant visits each vertex exactly once and closes its tour.
4. MMAS bounds: verify `tau_min <= tau_ij <= tau_max` after updates.
5. Reproducibility: the same seed produces the same trajectory in the same environment.

### 10.2. Integration tests

1. Bandit exploration: inspect whether each arm is selected in windows of 100
   local-search calls along a long seeded trajectory. This is an empirical test,
   not a guarantee implied by positive sampling probability.
2. Value of VND: investigate whether its posterior mean exceeds the means of
   2-opt and Or-opt on at least one instance, to assess the combined pipeline.
3. Colony diversity: measure the standard deviation of ant tour lengths in each
   iteration. Collapse to zero can indicate stagnation or a poorly chosen q or
   pheromone bounds.
4. Stopping condition: verify that a full window of W calls without improvement
   triggers stopping, and that stopping is not triggered prematurely.

### 10.3. Overall comparison

Compare four configurations across a set of instances:

- ACO alone, without local search.
- ACO with a fixed local-search operator, such as 2-opt on the top-q tours.
- ACO with bandit-selected local search: the integrated algorithm.
- Sequential ACO followed by bandit-selected local search at the end.

The design hypothesizes that the integrated bandit version can find shorter
tours at equal computational budgets, and that online improvement can outperform
end-only improvement by influencing pheromone during construction. Experiments
must establish whether those expectations hold; they are not proven by the design.

## 11. Pitfalls to keep in mind

**Interaction between ACO and the bandit.** Improving the top-q tours before
pheromone deposition increases concentration. A large q or missing bounds can
cause stagnation. The document recommends MMAS bounds and a small q, at most 3.

**Reward relative to tour length.** Dividing by `L0` makes improvement relative
to the tour being processed. In a VRP extension with tours of different sizes,
normalization may need adapting, using an average tour length or individual lengths.

**Stopping window.** W counts local-search calls rather than ACO iterations.
For `q = 3`, a window of 30 calls corresponds to 10 iterations.

**Evaluation cost.** The bandit does not reduce the number of evaluations within
one local-search call. It selects which operator to run. A small q, the stopping
condition, and avoiding an expensive combined operator when a simpler one is
sufficient are the mechanisms intended to control total cost.

**ACO parameters.** Very large a and b make construction nearly deterministic,
which can cause stagnation. A very small rho slows evaporation and can have the
same effect. The source gives typical ranges of `a` and `b` from 1 to 5, and rho
from 0.1 to 0.5; these are design suggestions, not universally optimal settings.

## 12. Conclusion

The proposed algorithm integrates local search into ACO rather than applying it
only after the colony has finished:

- The m ants construct tours in each iteration.
- The best q tours receive local search, with 2-opt, Or-opt, or VND chosen by a
  fractional Beta bandit.
- Ants deposit pheromone on improved tours for the top-q and raw tours for the
  others, combining guidance with continued exploration.
- MMAS bounds and a small q aim to prevent stagnation.

The bandit receives up to q observations per iteration, while the number of
local-search calls is bounded by `q * T_ACO`. The fractional reward preserves
information about improvement magnitude and evaluation effort. Beta sampling
maintains the possibility of exploring every arm. The sliding-window stopping
condition separates the question "which operator should be used?" from "when
should the run stop?".
