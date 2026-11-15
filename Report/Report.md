# TSP Solver — Final Project Report

**Authors:** Amine Nakari, Mohammed Mehdi Berrahou  
**Course:** Programming 

## 1. Introduction

The goal of this project is to implement a program that solves a restricted version of the Travelling Salesman Problem: metric TSP on complete graphs. Solving this problem requires finding the shortest tour that visits every city exactly once and returns to the starting city. In this project, we implemented a trivial method that lists cities in ascending index order, the Held-Karp Algorithm, and Ant Colony Optimization (with some enhancements using 2-opt, Or-opt, or their combination, with the local-search method selected by a bandit mechanism). The input can be either the lower triangle of a weight matrix, including its diagonal, or a list of coordinates. We evaluate these methods on instances of different sizes, comparing execution time and tour length to examine the quality of the solution.

This version was implemented by Amine Nakari and Mohammed Mehdi Berrahou. This submission contains the source code for the solutions and comparisons along with this report.

## 2. Chronological Account of the Work

| Date | Stage | Summary |
|---|---|---|
| September 21 | Project setup | Set up the project and reused graph classes from the previous lab. |
| September 30 | Matrix parser | Implemented lower-diagonal parsing and the trivial method (outputting to a file). |
| September 30 | Coordinate parser | Implemented the coordinate parser and its trivial method (outputting to a file). |
| October 14 | Cleanup | Cleaned up the code and adjusted file handling. |
| October 17 | Integration | Combined both parsers into one program and organized the source files. |
| October 17 | Testing and documentation | Added basic tests, corrected issues, and documented program usage. |
| October 18 | Mid-term submission | Completed the trivial-method report and prepared the first submission. |
| November 9 | Subsets and Held–Karp | Added the subset library and implemented Held–Karp with tour reconstruction. |
| November 14 | Coordinate distances and solver integration | Added coordinate distance calculations, connected Held–Karp to the command line, and added initial benchmarking. |
| November 15 | ACO integration | Integrated plain ACO and enhanced ACO with bandit-selected local search. |
| November 15 | Performance comparison | Compared execution time and tour length for all four variants across the 16 instances, recording skipped and timed-out runs. |
| November 15 | Local-search changes | Replaced repeated searches with single-pass 2-opt and Or-opt, renamed VND to Combined, and reran the comparison. |
| November 15 | Documentation and submission cleanup | Updated the README, removed generated build files, added ignore rules, and merged the changes into main. |


### 2.1 Understanding the Requirements and Planning

We started by reading the project instructions and looking through the provided files to understand their different sections and how to run the program. The coordinate format seemed easier for this first stage: we only had to read the cities and their coordinates, without worrying about how distances were calculated. The lower-diagonal format was a bit more challenging because we had to rebuild the full matrix from its lower-diagonal representation.

We split the work as follows: Amine took care of the lower-diagonal matrix parser, and Mehdi took care of the coordinate parser. 

### 2.2 Developing the Matrix Parser

When it came to developing the matrix parser, we first started by looking through the lower-diagonal instance files to understand how the weights were listed. We read the instance name, type, and number of cities before reaching the most important part: `EDGE_WEIGHT_SECTION`. The number of cities was used to create an empty matrix. We then rebuilt the whole matrix by reading one number for the first row, two for the second, and so on, then copying each value to its matching position in the upper half (since the matrix is symmetric). The `AdjacenceMatrixWeightedGraph` class from the previous lab was reused to store the completed matrix. The matrix was also saved in a `.matrix.txt` file for inspection; these exports were later removed.

### 2.3 Developing the Coordinate Parser

We initially implemented the coordinate parser as a class called `parser`, with its declaration in `parser.hpp`, its methods in `parser.cpp`, and a separate `main.cpp` to run it. The class first read and stored the file's lines, then extracted the metadata and located `NODE_COORD_SECTION`. From there, it read each city's ID and x/y coordinates until reaching `EOF`. It also included a method to write the trivial solution, and the initial program used `berlin52.tsp` as its input. During integration, the class methods were combined into a function in `Coord_Parser.cpp`, keeping the same parsing steps.

### 2.4 Integrating and Organizing the Program


At first, the two parsers were two separate programs. However, when refactoring, the parsers were combined into one program that detected the input format from the section marker inside the file. We ended up with three main `.cpp` files: `Trivial_Solver.cpp`, `Coord_Parser.cpp`, and `Parser_Matrix.cpp`. Additionally, instead of writing from each of the parsers, both writing functions were moved to `Trivial_Solver.cpp`.


### 2.5 Testing and Corrections

At first, we tried the program on lower-diagonal and coordinate instances manually and checked that the resulting tour output was correct (ascending ID order, followed by `-1` and `EOF`). One issue that arose was that the `ulysses16.tsp` file had a space before `EOF`, so a condition was added so the file could be parsed correctly. We also added two automated parser tests for `gr17` and `berlin52`. The checks are described in Section 7. 


### 2.6 Implementing Held–Karp

We implemented Held–Karp by following the pseudocode provided in the handout and using the subset classes imported from a previous TP. Understanding how the DP and parent tables worked together was challenging, so we traced a four-city example step by step before completing the tour reconstruction. Section 5 explains the algorithm and its implementation.

### 2.7 Coordinate Distances and Solver Integration


Using Held-Karp or any other algorithm required us to calculate the distances from the coordinate TSP instances based on the `EDGE_WEIGHT_TYPE`. The implemented distance calculations were for `EUC_2D` (straight-line distance rounded to the nearest integer), `ATT` (pseudo-Euclidean distance using a scaled formula and rounding up), and `GEO` (distance along the Earth’s surface using geographic coordinates).

For two cities $i$ and $j$, the formulas are:

**EUC_2D:**

$$
d(i,j) = \left\lfloor \sqrt{(x_i-x_j)^2+(y_i-y_j)^2}+0.5 \right\rfloor
$$

**ATT:**

$$
r = \sqrt{\frac{(x_i-x_j)^2+(y_i-y_j)^2}{10}},
\qquad d(i,j) = \lceil r \rceil
$$

**GEO:** The coordinates use TSPLIB's degrees-and-minutes format. For each coordinate $c$, we first convert it to radians:

$$
D = \operatorname{trunc}(c), \qquad M = c-D,
\qquad \theta = \frac{3.141592}{180}\left(D+\frac{5M}{3}\right)
$$

Let $\phi_i$ and $\lambda_i$ be the converted latitude and longitude of city $i$. We then calculate:

$$
\begin{aligned}
q_1 &= \cos(\lambda_i-\lambda_j),\\
q_2 &= \cos(\phi_i-\phi_j),\\
q_3 &= \cos(\phi_i+\phi_j),\\
u &= \frac{(1+q_1)q_2-(1-q_1)q_3}{2},\\
d(i,j) &= \left\lfloor 6378.388\,\arccos\!\left(\max(-1,\min(1,u))\right)+1 \right\rfloor.
\end{aligned}
$$

Here, $6378.388$ is the Earth radius in kilometres used by TSPLIB. Clamping $u$ to $[-1,1]$ avoids numerical errors in $\arccos$. The diagonal entries $d(i,i)$ are set to zero for all three distance types.

We stored the calculated distances in a full matrix so the coordinate instances could use the same graph representation and solvers as matrix instances. We then proceeded to connect our Held-Karp implementation to the main program by using the `-m held-karp` option.

### 2.8 Integrating Ant Colony Optimization


We integrated Ant Colony Optimization into our project using the existing graph representation, allowing it to work with both matrix and coordinate instances. We connected the enhanced version to the main program through `-m aco` and kept plain ACO for comparison. Section 6 explains both versions.


### 2.9 Comparing the Algorithms


We created `compare_algorithms.cpp` and `compare_algorithms.py` to compare the four methods across the 16 instances and save their execution times and tour lengths in Markdown tables. This allowed us to compare them under the same settings. Section 8 presents the results.

## 3. Code Structure and Design Choices

### 3.1 Organization of the Source Files

We organized the files according to what each part of the program does:

- `Parser_Matrix.cpp`: reads the lower-diagonal values and rebuilds the matrix, then returns the name and graph data.
- `Coord_Parser.cpp`: reads the information at the start of the file, followed by the city IDs and their coordinates, and calculates the distance matrix.
- `main.cpp`: contains `main()`. It reads the arguments, chooses the right parser and solving method, and saves the tour to the output file.
- `Held_Karp.cpp` and `Held_Karp.hpp`: implement and declare the Held–Karp solver, including tour reconstruction.
- `subset.hpp`, `subset_iterator.hpp`, and `subset.cpp`: represent subsets and allow the Held–Karp solver to iterate over them (imported directly from TP work).
- `ACO_Solver.cpp` and `ACO_Solver.hpp`: implement and declare the ACO solver, its configuration, and its local-search methods.
- `Parser.hpp`: declares the two parser functions so the solver can call them. It also defines the structures used to pass their results back to the solver.
- Graph classes: these come from the previous lab. `Graph` describes the basic graph operations, `WeightedGraph` adds operations involving weights, and `AdjacenceMatrixWeightedGraph` stores our matrix.
- `tests/tests.cpp`: checks a few values read from `gr17` and `berlin52` to see whether both parsers work as expected.
- `compare_algorithms.cpp`: calls the solvers, measures execution time, checks the returned tours, and calculates tour length.
- `compare_algorithms.py`: runs the comparison program on each instance, handles timeouts and skipped runs, and writes the results in Markdown tables.
- `CMakeLists.txt`: lists what needs to be compiled to build the solver, comparison program, and tests.
- `build-TSPsolver.sh`: lets us build everything with one command.
- `TSP-instances/`: keeps the input files together, separate from the source code.

We wanted to avoid keeping everything in one file. Each parser takes care of reading its own format, while the selected solver creates the tour and the main program writes it. Keeping these parts separate makes the code easier to follow and lets us work on one part without mixing it with the rest. This allowed us to add new solving methods and keep using the same parsers. There is still one main solver executable to run, and it chooses the parser from the contents of the input file.

### 3.2 Data Representations


**Matrix Instances:** We used the `AdjacenceMatrixWeightedGraph` class to represent the full matrix. Each position `[i][j]` contains the distance between cities i and j. The `TSPData` structure holds that graph, the number of cities, and the type of problem, with the parser returning the instance name as well.

**Coordinate Instances:** `CoordinateData` stores the metadata in a map and uses the three vectors `indexs`, `xs` and `ys`, with `indexs[i]`, `xs[i]`, and `ys[i]` holding the ID and coordinates of the city at position i. It also stores the calculated distance matrix in `distances`. This matrix is used to construct an `AdjacenceMatrixWeightedGraph` for Held–Karp and ACO. The trivial method only requires the IDs of the cities.

We kept both representations to support the two input formats, while using the same graph representation for the solvers.

### 3.3 Interaction Between Components

The components interact in the following manner:

**Matrix**: `main()` in `main.cpp` detects the format, and `parse_tsp()` returns an instance name and graph data. Depending on the selected method, `trivial_solution()`, `held_karp()`, or `aco_solve()` creates the tour, while `write_tour()` saves it.

**Coordinates**: As with the matrix, `main()` detects the format, and `parse_coordinates()` returns the IDs, coordinates, and calculated distances. For Held–Karp and ACO, the distances are used to create the graph, and the returned tour positions are mapped back to the original city IDs. For the trivial method, the IDs remain in the order they were read. `write_coordinate_tour()` then saves the tour.


**Comparison**: `compare_algorithms.py` runs `compare_algorithms.cpp` for each instance and method. The C++ program loads the graph, calls the existing solver, and returns its execution time, tour length, and tour. The Python script collects these results and writes the tables.


## 4. Trivial Method and Program Usage

### 4.1 Trivial Algorithm

The trivial algorithm visits cities in ascending ID order. With four cities, this gives 1 → 2 → 3 → 4 → 1. The return to city 1 means returning to the start. The tour is valid because every city is visited once before returning to the start, but it is not necessarily the shortest. Distances do not affect the order because the method only uses city IDs. For coordinate inputs, we assume cities are listed in ascending ID order.

### 4.2 Building and Running the Program

From the project root, we build the solver and tests with:

```bash
bash build-TSPsolver.sh
```

This creates `build/TSPsolver` and `build/tests`. To run the tests:

```bash
./build/tests
```

The solver takes the input file path and output file path as its two arguments. For example:

```bash
./build/TSPsolver TSP-instances/coord/berlin52.tsp berlin52.tour
./build/TSPsolver TSP-instances/lower_diag/gr17.tsp gr17.tour -m trivial
```

The trivial method is considered the default, so `-m trivial` is optional.

### 4.3 Solution File Format

The solution file is formatted as per the requirements:

The output starts with the tour name (`NAME`), `TYPE : TOUR`, and the number of cities (`DIMENSION`). After `TOUR_SECTION`, each city ID is written on its own line, followed by `-1` and `EOF`. The starting city is not repeated.

The file is saved exactly where the output argument specifies. The parent folder must already exist if the output path includes a folder.

## 5. Held–Karp Algorithm

Held–Karp is an exact algorithm based on dynamic programming. Instead of examining each complete tour independently, it stores the shortest paths for smaller subsets and uses them to construct paths for larger subsets. We fix city $0$ as the starting city, since changing the starting point of a cycle does not change its length.

### 5.1 States and Initialization

Let $V=\{0,\ldots,n-1\}$ and $U=V\setminus\{0\}$. For $S\subseteq U$ and $v\in S$, we define

$$
C(S,v)=\text{minimum cost of a path from }0\text{ visiting exactly the cities in }S\text{ and ending at }v.
$$

In `Held_Karp.cpp`, this state is stored in `dp[mask][j]`, where `mask` represents $S$ and column $j$ corresponds to graph city $j+1$. Each bit indicates whether a non-starting city belongs to the subset. For four cities $A,B,C,D$, with $A$ fixed as the start, the masks $001$, $011$, and $111$ represent $\{B\}$, $\{B,C\}$, and $\{B,C,D\}$ respectively.

The table contains $2^{n-1}$ rows and $n-1$ columns, initially set to infinity. The initialization loop fills the singleton states:

$$
C(\{v\},v)=w(0,v).
$$

This corresponds to `dp[1 << j][j] = graph.weight_between_vertices(0, j + 1)`: a path visiting only one city is simply the direct edge from the start.

### 5.2 Recurrence and Subset Iteration

For larger subsets, the final edge must come from another city in the subset. Therefore,

$$
C(S,v)=\min_{u\in S\setminus\{v\}}
\left[C(S\setminus\{v\},u)+w(u,v)\right].
$$

The outer loop processes subsets by increasing size, and `SubsetIterator(k, m)` enumerates the subsets of size $k$, where $m=n-1$. For each possible endpoint `last`, `remaining.minus(last)` removes it from the subset. The inner loop then checks each possible predecessor `previous` using

```cpp
double candidate = dp[remaining.mask()][previous]
    + graph.weight_between_vertices(previous + 1, last + 1);
```

When a smaller cost is found, the code updates `dp` and stores the predecessor's column in `parent`. Processing subsets in increasing size ensures that every smaller state needed by the recurrence has already been calculated.

### 5.3 Four-City Trace and Tour Reconstruction

To check our understanding, we traced an example with the following symmetric distance matrix, which satisfies the triangle inequality:

$$
W=\begin{pmatrix}
0&10&15&20\\
10&0&25&25\\
15&25&0&30\\
20&25&30&0
\end{pmatrix},
\qquad \text{city order: }A,B,C,D.
$$

The singleton states are $C(\{B\},B)=10$, $C(\{C\},C)=15$, and $C(\{D\},D)=20$. For example, the two-city state ending at $B$ is

$$
C(\{B,C\},B)=C(\{C\},C)+w(C,B)=15+25=40.
$$

In the implementation, this is `dp[3][0] = dp[2][1] + w(C,B)`, with `parent[3][0] = 1`. Extending to the full subset gives

$$
\begin{aligned}
C(\{B,C,D\},B)
&=\min\left\{C(\{C,D\},C)+25,\ C(\{C,D\},D)+25\right\}\\
&=\min\{50+25,\ 45+25\}=70.
\end{aligned}
$$

The final path costs and complete tour costs are:

| Ending city $v$ | $C(U,v)$ | $C(U,v)+w(v,A)$ |
|---|---:|---:|
| B | 70 | 80 |
| C | 65 | 80 |
| D | 65 | 85 |

The final loop implements the closing step

$$
\operatorname{OPT}=\min_{v\in U}\left[C(U,v)+w(v,0)\right].
$$

Here, the minimum is $80$. Since the code checks $B$ first and only replaces a result when it is strictly smaller, it selects $B$ as the last city. Backtracking follows `parent[7][0] = 2`, then `parent[6][2] = 1`, then `parent[2][1] = -1`. This produces $B,D,C$ in reverse order. The code appends $A$ and reverses the vector, giving the tour $A\to C\to D\to B\to A$, of length $80$. The returned vector does not repeat $A$; the return edge is implicit in the tour format.

### 5.4 Complexity and Practical Limit

There are $O(n2^n)$ states, each checking up to $O(n)$ predecessors. The resulting time complexity is $O(n^2 2^n)$, and the DP and parent tables require $O(n2^n)$ memory. Our implementation rejects instances with more than 20 cities to limit resource use; this threshold was chosen by us as a practical limit and is not a theoretical restriction of the algorithm.

### 5.5 Running Held–Karp

After building the project as described in Section 4.2, we select Held–Karp with `-m held-karp`:

```bash
./build/TSPsolver TSP-instances/lower_diag/gr17.tsp gr17_held_karp.tour -m held-karp
```

This runs the solver on the 17-city instance and saves the tour in `gr17_held_karp.tour`. Instances above 20 cities are rejected by our implementation.

## 6. Additional Algorithm: Ant Colony Optimization

For the additional algorithm, we initially planned to implement the Christofides–Serdyukov algorithm using the blossom algorithm for the matching step. After discussing this idea with the professor, we were advised to choose another approach. We then decided to work on Ant Colony Optimization instead.

Ant Colony Optimization (ACO) uses randomness to build tours, taking inspiration from how ants leave pheromones along their routes. Routes with more pheromone become more likely to be followed by later ants. Unlike Held–Karp, ACO does not guarantee the shortest possible tour. We implemented it in `ACO_Solver.cpp`, where `aco_solve()` builds tours over several iterations and updates the pheromone matrix. We use two versions: `ACOOnly`, without local search, and `AdaptiveBandit`, which tries to improve selected tours before updating pheromones.

### 6.1 Tour Construction in Plain ACO

At each iteration, we create $m$ ants. Each ant starts at a randomly chosen city and builds a tour by visiting the remaining cities one at a time. We keep track of the visited cities so that each city is visited only once. If the ant is at city $i$, it chooses its next city $j$ from the unvisited set $\mathcal{U}$ using the probability

$$
p_{ij}=\frac{\tau_{ij}^{\,a}\eta_{ij}^{\,b}}
{\displaystyle\sum_{k\in\mathcal{U}}\tau_{ik}^{\,a}\eta_{ik}^{\,b}},
\qquad \eta_{ij}=\frac{1}{d(i,j)}.
$$

Here, $\tau_{ij}$ represents the pheromone on the edge, while $\eta_{ij}$ gives more weight to nearby cities. The parameters $a$ and $b$ control how much each factor affects the choice. In `construct_tour()`, we calculate these weights and pass them to `std::discrete_distribution`, which randomly selects the next city according to their relative weights. We also use a small positive minimum when calculating the weights to avoid division by zero.

These two parts of the code calculate the weight and then choose a city when the weights are valid:

```cpp
double weight = std::pow(tau, pheromone_weight)
              * std::pow(1.0 / safe_distance, heuristic_weight);
```

```cpp
std::discrete_distribution<std::size_t> distribution(
    weights.begin(), weights.end());
chosen_position = distribution(rng);
```

`std::discrete_distribution` normalizes the weights to obtain the selection probabilities.

For example, we can reuse the four-city matrix from Section 5. At the start, all edges have the same pheromone level. With $a=1$ and $b=3$, an ant at $A$ has the following probabilities:

$$
p_{AB}=\frac{216}{307},\qquad
p_{AC}=\frac{64}{307},\qquad
p_{AD}=\frac{27}{307}.
$$

City $B$ is the closest, so it has the highest chance of being chosen. However, the ant can still choose $C$ or $D$. Once the tour is built, `tour_length()` calculates its length, including the return to the starting city. With our setting of $m=30$, we get 30 tours per iteration.

### 6.2 Pheromone Updates and Diversification

We update pheromones after all ants have finished their tours. In the enhanced version, we first apply local search to the selected tours. The update has two steps: we reduce the existing pheromone through evaporation, then each ant adds pheromone to the edges it used, including the return edge:

$$
\tau_{ij}\leftarrow(1-\rho)\tau_{ij}
+\sum_{r=1}^{m}\Delta\tau_{ij}^{(r)},
\qquad
\Delta\tau_{ij}^{(r)}=
\begin{cases}
Q/L_r,&\text{if ant }r\text{ uses edge }\{i,j\},\\
0,&\text{otherwise},
\end{cases}
$$

Here, $L_r$ is the tour length for ant $r$, $\rho$ controls evaporation, and $Q$ controls the amount deposited. Since the deposit is $Q/L_r$, shorter tours add more pheromone. We update both directions of each edge because the distances are symmetric. Every ant contributes to the update, and `consider_best()` keeps track of the best tour found so far.

The evaporation step appears in the code as follows:

```cpp
pheromone[i][j] *= (1.0 - config.evaporation_rate);
pheromone[j][i] = pheromone[i][j];
```

The next loop calculates each ant’s deposit using `config.deposit_constant / ant.length`, which corresponds to $Q/L_r$.

In both versions, we also keep pheromone levels between a minimum and a maximum. These limits depend on the best tour length $L^*$ found so far:

$$
\tau_{\max}=\frac{1}{\rho L^*},\qquad
\tau_{\min}=\frac{\tau_{\max}}{2n}.
$$

`clamp_pheromone()` applies these limits to the edges between different cities. If 50 consecutive iterations pass without finding a new best tour, we perform a partial restart. `partially_reinitialize_pheromone()` moves the pheromone levels halfway toward the midpoint of the limits:

$$
\tau_{ij}\leftarrow(1-f)\tau_{ij}
+f\frac{\tau_{\min}+\tau_{\max}}{2},\qquad f=0.5.
$$

This keeps some of the information from earlier tours while giving other routes a better chance of being explored.

### 6.3 Enhancing Tours with Local Search

In the enhanced version, we sort the tours by length using `sort_ants_by_length()` and select the best $q=2$. We then try to improve each of these tours using one of the three methods in `apply_local_search()`:

- **2-opt:** `two_opt()` removes two edges, reconnects the tour, and reverses the segment between them. We replace $(a,b)$ and $(c,d)$ with $(a,c)$ and $(b,d)$ if the new edges are shorter:

  $$
  d(a,c)+d(b,d)<d(a,b)+d(c,d).
  $$

- **Or-opt:** `or_opt()` moves a segment of one, two, or three consecutive cities to another position without changing their internal order. We compare the edges before and after the move, then check the full tour length before accepting it.
- **Combined:** we apply one 2-opt pass, followed by one Or-opt pass to the resulting tour.

The following conditions in `apply_local_search()` decide which method to run:

```cpp
if (arm == LocalSearchArm::TwoOpt) {
    result.evaluations = two_opt(tour, graph);
} else if (arm == LocalSearchArm::OrOpt) {
    result.evaluations = or_opt(tour, graph);
} else {
    result.evaluations = two_opt(tour, graph);
    result.evaluations += or_opt(tour, graph);
}
```

Each method changes the tour directly and returns the number of moves it checked. We use this count later when calculating the bandit’s reward.

We use a single pass for each method. Several improvements can be accepted during that pass, but the search does not restart from the beginning after each improvement. This limits the work per call, although further improvements might still be possible afterward. For example, the tour $A\to B\to C\to D\to A$ from Section 5 has length $85$. A 2-opt move replacing $(B,C)$ and $(D,A)$ with $(B,D)$ and $(C,A)$ produces $A\to B\to D\to C\to A$, with length $80$.

After local search, the selected ants use their improved tours when depositing pheromone. The other ants keep the tours they originally built. If a method does not improve its selected tour, we keep that tour unchanged and update the bandit with a zero reward.

### 6.4 Selecting Operators with the Bandit

We use a bandit to choose which local-search method to apply. It uses Thompson sampling: each method $r$ has a Beta distribution, initially $\operatorname{Beta}(1,1)$. `select_arm()` draws one random value from each distribution and selects the method with the largest value:

$$
\theta_r\sim\operatorname{Beta}(\alpha_r,\beta_r),
\qquad r^*=\operatorname*{arg\,max}_{r}\theta_r.
$$

The parameters $\alpha_r$ and $\beta_r$ describe the Beta distribution for method $r$.

After applying a method, `observe_local_search()` records how much the tour improved, $g=\max(0,L_{\mathrm{before}}-L_{\mathrm{after}})$, and how many moves were checked, $c$. We use both values in `normalized_local_search_reward()`:

$$
z=\frac{g}{\max(L_{\mathrm{before}},\varepsilon_L)\max(c,\varepsilon_c)},
\qquad R=\frac{z}{1+z}.
$$

This reward takes into account the improvement relative to the original tour length and the number of moves checked to obtain it. If the tour did not improve, $R=0$. We then update only the method that was selected, using `update_bandit()`:

$$
\alpha_{r^*}\leftarrow\gamma\alpha_{r^*}+R,
\qquad
\beta_{r^*}\leftarrow\gamma\beta_{r^*}+(1-R),
$$

We use $\gamma=0.99$ so older observations gradually have less influence. For each selected tour, we sample a method and update its distribution after the local-search call.

These parts of the main loop show the three steps: choose a method, apply it, and update the bandit with the result:

```cpp
selected = select_arm(arms, rng);
```

```cpp
LocalSearchResult local = apply_local_search(selected, ants[rank].tour, graph);
ants[rank].length = local.after;
```

```cpp
observe_local_search(local, arms, selected, config, result, recent_gains);
```

Inside `update_bandit()`, the two parameters are updated directly from the reward:

```cpp
arm.alpha = forgetting * arm.alpha + reward;
arm.beta = forgetting * arm.beta + (1.0 - reward);
```

### 6.5 Parameters and Stopping Conditions

For our comparisons, we kept the following default settings from `ACOConfig`:

| Parameter | Value | Role |
|---|---:|---|
| Ants $m$ | 30 | Tours constructed per iteration |
| Maximum iterations $T$ | 200 | Upper limit on ACO iterations |
| Pheromone exponent $a$ | 1 | Influence of pheromone on city selection |
| Heuristic exponent $b$ | 3 | Influence of inverse distance on city selection |
| Evaporation rate $\rho$ | 0.2 | Fraction of pheromone removed per iteration |
| Deposit constant $Q$ | 1 | Scale of pheromone deposits |
| Initial off-diagonal pheromone | 1 | Starting pheromone level |
| Selected tours $q$ | 2 | Tours receiving local search in enhanced ACO |
| Bandit forgetting factor $\gamma$ | 0.99 | Weight retained from earlier bandit observations |
| No-improvement window $W$ | 90 calls | Early stopping after consecutive unsuccessful local-search calls |
| Restart interval | 50 iterations | Consecutive iterations without a new global best before a partial restart |
| Restart fraction $f$ | 0.5 | Strength of the pheromone blend |
| Numerical guards $\varepsilon_L,\varepsilon_c$ | $10^{-12}$ | Avoid zero denominators in the reward |
| Random seed | 7 | Makes each mode's run reproducible with the same input and settings |

Plain ACO runs for 200 iterations. The enhanced version can stop earlier if none of the last 90 local-search calls improved its tour. We check this condition at the end of each iteration.

In one enhanced-ACO iteration, we therefore build the tours, select the best two, improve them with the methods chosen by the bandit, and update the bandit and best tour found so far. We then evaporate and deposit pheromones before starting the next iteration. Plain ACO builds the tours and updates pheromones without the local-search or bandit steps.

### 6.6 Running Ant Colony Optimization

After building the project, we select enhanced ACO with `-m aco`:

```bash
./build/TSPsolver TSP-instances/lower_diag/gr17.tsp gr17_aco.tour -m aco
```

This saves the tour in `gr17_aco.tour`. The `-m aco` option runs the enhanced version described above. Plain ACO is used in the performance comparison and is not a separate method available through `TSPsolver`.

## 7. Validation and Difficulties

### 7.1 Parser Tests

We added two simple tests using values from the input files. For `gr17`, we check that the graph has 17 cities and that the distance between the first two cities is 633 in both directions. For `berlin52`, we check the number of IDs and coordinates, the first and last IDs, and the first city's position, (565, 575). These tests only read the files and check the data in memory; they do not create output files.

These tests are in `tests/tests.cpp` and run with `./build/tests` after building the project as described in Section 4.2. For the solvers, we used the comparison runs and the checks described below.

### 7.2 Solver Validation

When running the comparisons, `compare_algorithms.cpp` checks each tour before recording its result. It checks the number of cities, makes sure the indices are within range, and checks that no city appears twice. This tells us whether every city was visited exactly once. It then calculates the length of the tour, including the return to the starting city, and checks that the result is a finite number.

For Held–Karp, we also checked the returned lengths against the known optimal values for three small TSPLIB instances:

| Instance | Distance type | Held–Karp length | Known optimal length |
|---|---|---:|---:|
| gr17 | Explicit matrix | 2085 | 2085 |
| burma14 | GEO | 3323 | 3323 |
| ulysses16 | GEO | 6859 | 6859 |

The three lengths match the values in the TSPLIB documentation. For ACO, we checked that the returned tours were valid and evaluated their quality through the comparisons in Section 8.

### 7.3 Building and Running the Program

We also checked that the project could be built from scratch with `build-TSPsolver.sh` and that the parser tests passed afterward. We tried the three command-line methods on both matrix and coordinate instances and checked the generated tour files for valid city IDs and the required `TOUR_SECTION`, `-1`, and `EOF` markers. The comparison program also successfully read all 16 instances and calculated their trivial tour lengths.

Our checks covered the provided instances and a few additional cases, rather than every possible input.

ACO was also one of the most complicated parts to implement and understand. We had to connect the probability calculations with the construction of each ant's tour, keeping track of visited cities and selecting the next city from the remaining ones. The pheromone updates, local-search methods, and bandit added more steps to follow. Understanding how these parts worked together was more difficult than looking at each formula separately.

## 8. Results and Performance Evaluation

### 8.1 Comparison Setup

We ran the four methods on the 16 instances and recorded their execution time and tour length. Each method was run once per instance, with both ACO versions using seed 7 and the parameters listed in Section 6. The times were measured in a Release build and cover the solver itself, excluding parsing, distance calculations, output, and validation. Additionally, tour lengths include the return to the starting city. The complete tables are provided in `Algorithm Comparison Result/algorithm_comparison.md`; below, we discuss the cases that show different behaviors.

The comparisons were run on a MacBook Air with an Apple M1 chip (8 CPU cores: 4 performance and 4 efficiency), 8 GB of memory, and macOS 13.3.1 (a). We compiled the C++20 code using Apple Clang 14.0.3 with `-O3` optimization in Release mode.

Each result is shown as **tour length (execution time in ms)**. Bold lengths are the shortest recorded for that instance.

| Instance | Cities | Trivial | Held–Karp | Plain ACO | Enhanced ACO |
|---|---:|---:|---:|---:|---:|
| sample-0 | 10 | 4,157 (<0.001) | **2,449** (0.218) | **2,449** (22.442) | **2,449** (21.325) |
| gr17 | 17 | 4,722 (<0.001) | **2,085** (68.413) | 2,094 (48.487) | **2,085** (111.105) |
| berlin52 | 52 | 22,205 (<0.001) | Did not run | 8,565 (365.716) | **7,704** (451.981) |
| gr24 | 24 | 3,436 (<0.001) | Did not run | 1,393 (266.174) | **1,272** (261.258) |
| dantzig42 | 42 | **699** (<0.001) | Did not run | 792 (199.541) | 701 (226.612) |
| pr299 | 299 | 83,506 (<0.001) | Did not run | 70,542 (8084.854) | **51,605** (9050.972) |
| pr1002 | 1002 | 349,403 (<0.001) | Did not run | 406,390 (82108.414) | **286,395** (99187.067) |

Held–Karp was not run above 20 cities. These entries have no measured execution time.

### 8.2 The Same Tour Length with All Three Solvers

On `sample-0`, Held–Karp, plain ACO, and enhanced ACO all returned a tour length of 2449, compared with 4157 for the trivial method. Since Held–Karp is exact, both ACO versions reached the optimal length on this instance. However, Held–Karp took only 0.218 ms, while plain and enhanced ACO took 22.442 ms and 21.325 ms respectively. For this small instance, ACO did not provide an advantage in either solution quality or runtime. Equal lengths do not necessarily mean that the returned tours visit the cities in the same order.

### 8.3 Enhanced ACO Matches Held–Karp

On `gr17`, Held–Karp and enhanced ACO both obtained 2085, while plain ACO obtained 2094. Plain ACO was faster, taking 48.487 ms compared with 68.413 ms for Held–Karp and 111.105 ms for enhanced ACO. This shows a runtime and quality tradeoff: plain ACO returned a slightly longer tour in less time, while the enhanced version reached the exact solution but took longer than Held–Karp. The same quality pattern appeared on `burma14` and `ulysses16`, where enhanced ACO matched Held–Karp and plain ACO returned a longer tour.

### 8.4 Local Search Improves ACO on Larger Instances

On `berlin52`, Held–Karp was skipped because the instance exceeds our implementation's 20-city limit. Enhanced ACO returned a length of 7704, compared with 8565 for plain ACO and 22205 for the trivial method. Enhanced ACO took 451.981 ms, while plain ACO took 365.716 ms. Here, the extra work of local search produced a shorter tour at the cost of more execution time. Without a Held–Karp result for this instance, this comparison alone does not establish that the enhanced result is optimal.

Enhanced ACO was not slower in every recorded run. On `gr24`, it returned 1272 in 261.258 ms, compared with 1393 in 266.174 ms for plain ACO. It is important to note that the small time difference is not evidence that the enhanced version is generally faster, because this was just one execution.

### 8.5 The Trivial Method Can Give a Better Result

On `dantzig42`, the trivial method returned 699, while enhanced ACO returned 701 and plain ACO returned 792. This is a case where the input order already gives a better tour than either heuristic found. Although enhanced ACO improved on plain ACO, it did not beat the baseline. This shows why keeping the trivial method in the comparison was useful: a more complicated method does not necessarily return the best result on every instance.

### 8.6 Runtime and Quality on the Largest Instances

On `pr299`, plain ACO took about 8.1 seconds and returned 70542, while enhanced ACO took about 9.1 seconds and returned 51605. On `pr1002`, these times increased to about 82.1 seconds and 99.2 seconds, with tour lengths of 406390 and 286395 respectively. Both versions therefore took considerably longer as the instance size increased, while enhanced ACO continued to produce the shorter tour in these runs.

The `pr1002` result also shows another limitation of plain ACO: its tour was longer than the trivial tour of 349403. Enhanced ACO improved on both. Both ACO runs completed within the comparison's 120-second limit, while Held–Karp was skipped because of its city limit. 

Across all 16 instances, enhanced ACO matched plain ACO on `sample-0` and returned a shorter tour on every other instance. However, both variants are stochastic, and these results use a single seed. Repeated runs with different seeds would be needed to assess how consistently the same differences appear.

## 9. Reflections and Conclusion

In this project, we learned how to use classes to represent graph problems, read different input formats, and connect several solving methods within the same program. Combining our work also taught us the value of separating responsibilities and agreeing on a common representation of the data.

Working on Held–Karp and ACO helped us understand the difference between guaranteeing an optimal solution and searching for a good one within a practical runtime. It also showed us that implementing an algorithm involves choices about resource use and how much work to spend improving a solution.

In our comparisons, enhanced ACO gave a good balance between tour quality and runtime. It matched Held–Karp's optimal lengths on the small instances and returned shorter tours than plain ACO on most instances, while also handling larger problems. Held–Karp was effective on small instances, but its exponential time and memory requirements limited its practical use. The result on `dantzig42` also reminded us that a more complicated method does not always beat the trivial baseline. Comparing the methods showed us why both execution time and tour length matter when evaluating a solver.


## 10. References and AI-Use Disclosure

### 10.1 References

We used the TSPLIB 95 documentation to understand the instance files and the tour format.

We also used C++ documentation throughout the project, especially for reading files and working with strings. Examples include `find()`, `substr()`, and `npos` for reading the headers, string streams for extracting values from a line, and `std::pair` for returning the instance name and data together.

### 10.2 Use of AI Tools

We used ChatGPT to help understand the problem and clarify parts of the requirements. We also used it sometimes when debugging, for example when dealing with index-out-of-range errors or looking for a suitable method to use. It helped explain C++ functions that made some tasks easier, such as checking whether a character was found using `std::string::npos`. ChatGPT/Codex also helped write the test file.

For the more complicated methods, especially Ant Colony Optimization, we used AI to find suitable C++ functions and understand how to use them. For example, it explained `std::discrete_distribution`, which selects a city according to the calculated weights, and `std::chrono::steady_clock`, which we used to measure execution time in the comparison program. It also explained how Python's `subprocess.run()` could run the C++ program and apply a timeout during benchmarking. We asked questions about the tour-construction probabilities, pheromone updates, and how the bandit selects between the local-search methods. For Held–Karp, it helped us trace a four-city example and understand the DP and parent tables.

The test file includes the following comment to indicate this assistance:

```cpp
// Partial ChatGPT/Codex assistance, 2026-10-16...
```

AI (specifically ChatGPT/Codex) was also used to suggest an outline for the report, draft and revise several paragraphs, and insert formulas, code excerpts, and tables. We used it to check technical explanations and correct grammar and sentence structure as well.

### 10.3 DALIA Self-Assessment

Our self-reported DALIA assessment is D2–D3. We used AI to explain unfamiliar methods and C++ functions, work through examples, and help with debugging and testing. Examples include tracing the Held–Karp tables and understanding probability-based city selection in ACO, and what is stated in the previous section. 
