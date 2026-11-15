# TSP algorithm comparison

Generated: 2026-11-15 16:27 CET

**One run per algorithm per file.** Execution time measures the solver only; file parsing, distance calculation, output, and tour validation are excluded. Tour length includes returning to the starting city; smaller is better.

Both ant-colony variants use seed **7**, **30 ants**, and at most **200 iterations**. “Ant colony + optimization” is the adaptive bandit hybrid: it applies 2-opt, Or-opt, or their combination to the best two tours per iteration. “Ant colony only” uses no local search. All other settings retain their defaults.
Local search uses one pass per operator without restarting after improvements. The combined operator performs one 2-opt pass followed by one Or-opt pass.

**Held–Karp keeps its current 20-city limit. Each process has a 120-second timeout**, including startup and parsing. A timeout has no completed solver time or tour length. “Did not run” is not a measured failure. Times come from a Release build on this machine.

## sample-0 — 10 cities

File: `TSP-instances/generated/sample-0.tsp`  
Input: **Coordinates** · Distance: **EUC_2D** · Edge-weight format: **Not specified**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 4,157 | Completed — valid tour |
| Held–Karp | 0.218 ms | 2,449 | Completed — valid tour |
| Ant colony only | 22.442 ms | 2,449 | Completed — valid tour |
| Ant colony + optimization | 21.325 ms | 2,449 | Completed — valid tour |

## burma14 — 14 cities

File: `TSP-instances/coord/burma14.tsp`  
Input: **Coordinates** · Distance: **GEO** · Edge-weight format: **FUNCTION**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 4,562 | Completed — valid tour |
| Held–Karp | 5.305 ms | 3,323 | Completed — valid tour |
| Ant colony only | 35.073 ms | 3,371 | Completed — valid tour |
| Ant colony + optimization | 39.041 ms | 3,323 | Completed — valid tour |

## ulysses16 — 16 cities

File: `TSP-instances/coord/ulysses16.tsp`  
Input: **Coordinates** · Distance: **GEO** · Edge-weight format: **Not specified**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 9,665 | Completed — valid tour |
| Held–Karp | 28.648 ms | 6,859 | Completed — valid tour |
| Ant colony only | 43.093 ms | 6,900 | Completed — valid tour |
| Ant colony + optimization | 46.064 ms | 6,859 | Completed — valid tour |

## gr17 — 17 cities

File: `TSP-instances/lower_diag/gr17.tsp`  
Input: **Matrix** · Distance: **EXPLICIT** · Edge-weight format: **LOWER_DIAG_ROW**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 4,722 | Completed — valid tour |
| Held–Karp | 68.413 ms | 2,085 | Completed — valid tour |
| Ant colony only | 48.487 ms | 2,094 | Completed — valid tour |
| Ant colony + optimization | 111.105 ms | 2,085 | Completed — valid tour |

## gr21 — 21 cities

File: `TSP-instances/lower_diag/gr21.tsp`  
Input: **Matrix** · Distance: **EXPLICIT** · Edge-weight format: **LOWER_DIAG_ROW**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 6,620 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 62.684 ms | 2,830 | Completed — valid tour |
| Ant colony + optimization | 87.878 ms | 2,707 | Completed — valid tour |

## gr24 — 24 cities

File: `TSP-instances/lower_diag/gr24.tsp`  
Input: **Matrix** · Distance: **EXPLICIT** · Edge-weight format: **LOWER_DIAG_ROW**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 3,436 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 266.174 ms | 1,393 | Completed — valid tour |
| Ant colony + optimization | 261.258 ms | 1,272 | Completed — valid tour |

## dantzig42 — 42 cities

File: `TSP-instances/lower_diag/dantzig42.tsp`  
Input: **Matrix** · Distance: **EXPLICIT** · Edge-weight format: **LOWER_DIAG_ROW**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 699 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 199.541 ms | 792 | Completed — valid tour |
| Ant colony + optimization | 226.612 ms | 701 | Completed — valid tour |

## att48 — 48 cities

File: `TSP-instances/coord/att48.tsp`  
Input: **Coordinates** · Distance: **ATT** · Edge-weight format: **Not specified**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 49,840 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 305.429 ms | 12,257 | Completed — valid tour |
| Ant colony + optimization | 335.555 ms | 10,742 | Completed — valid tour |

## gr48 — 48 cities

File: `TSP-instances/lower_diag/gr48.tsp`  
Input: **Matrix** · Distance: **EXPLICIT** · Edge-weight format: **LOWER_DIAG_ROW**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 19,837 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 268.951 ms | 5,822 | Completed — valid tour |
| Ant colony + optimization | 301.737 ms | 5,056 | Completed — valid tour |

## berlin52 — 52 cities

File: `TSP-instances/coord/berlin52.tsp`  
Input: **Coordinates** · Distance: **EUC_2D** · Edge-weight format: **Not specified**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 22,205 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 365.716 ms | 8,565 | Completed — valid tour |
| Ant colony + optimization | 451.981 ms | 7,704 | Completed — valid tour |

## st70 — 70 cities

File: `TSP-instances/coord/st70.tsp`  
Input: **Coordinates** · Distance: **EUC_2D** · Edge-weight format: **Not specified**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 3,410 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 475.935 ms | 817 | Completed — valid tour |
| Ant colony + optimization | 558.357 ms | 675 | Completed — valid tour |

## eil76 — 76 cities

File: `TSP-instances/coord/eil76.tsp`  
Input: **Coordinates** · Distance: **EUC_2D** · Edge-weight format: **Not specified**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 1,969 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 590.012 ms | 677 | Completed — valid tour |
| Ant colony + optimization | 700.617 ms | 559 | Completed — valid tour |

## pr76 — 76 cities

File: `TSP-instances/coord/pr76.tsp`  
Input: **Coordinates** · Distance: **EUC_2D** · Edge-weight format: **Not specified**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 150,781 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 556.747 ms | 132,338 | Completed — valid tour |
| Ant colony + optimization | 674.650 ms | 109,306 | Completed — valid tour |

## ch130 — 130 cities

File: `TSP-instances/coord/ch130.tsp`  
Input: **Coordinates** · Distance: **EUC_2D** · Edge-weight format: **Not specified**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 47,797 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 1636.337 ms | 7,765 | Completed — valid tour |
| Ant colony + optimization | 1823.482 ms | 6,346 | Completed — valid tour |

## pr299 — 299 cities

File: `TSP-instances/coord/pr299.tsp`  
Input: **Coordinates** · Distance: **EUC_2D** · Edge-weight format: **Not specified**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 83,506 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 8084.854 ms | 70,542 | Completed — valid tour |
| Ant colony + optimization | 9050.972 ms | 51,605 | Completed — valid tour |

## pr1002 — 1002 cities

File: `TSP-instances/coord/pr1002.tsp`  
Input: **Coordinates** · Distance: **EUC_2D** · Edge-weight format: **Not specified**

| Algorithm | Execution time | Tour length | Result |
|---|---:|---:|---|
| Trivial | <0.001 ms | 349,403 | Completed — valid tour |
| Held–Karp | — | — | Did not run — exceeds 20-city limit |
| Ant colony only | 82108.414 ms | 406,390 | Completed — valid tour |
| Ant colony + optimization | 99187.067 ms | 286,395 | Completed — valid tour |

