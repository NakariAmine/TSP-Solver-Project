# TSP Solver — Trivial Method Progress Report

**Authors:** Amine Nakari, Mohammed Mehdi Berrahou  
**Course:** Programming 

## 1. Introduction

The goal of this project is to implement a program that solves a restricted version of the Travelling Salesman Problem: metric TSP on complete graphs. Solving this problem requires finding the shortest tour that visits every city exactly once and returns to the starting city. However, at this stage, we are only implementing the trivial method, which establishes an input-to-output system that lists cities in ascending order of their indices, without optimizing the tour length. The input can be either the lower triangle of a weight matrix, including its diagonal, or a list of coordinates. The Held–Karp algorithm will be implemented at a later stage to find an optimal tour.

This version was implemented by Amine Nakari and Mohammed Mehdi Berrahou. This submission contains the source code for the trivial solution along with this report.

## 2. Chronological Account of the Work

| Date | Stage | Summary |
|---|---|---|
| September 21 | Project setup | Set up the project and reused graph classes from the previous lab. |
| September 30 | Matrix parser | Implemented lower-diagonal parsing and the trivial method (outputting to a file). |
| September 30 | Coordinate parser | Implemented the coordinate parser and its trivial method (outputting to a file) |
| October 14 | Cleanup | Cleaned up the code and adjusted file handling. |
| October 17 | Integration | Combined both parsers into one program and organized the source files. |
| October 17 | Testing and documentation | Added basic tests, corrected issues, and documented program usage. |


### 2.1 Understanding the Requirements and Planning

We started by reading the project instructions and looking through the provided files to understand their different sections and how to run the program. The coordinate format seemed easier for this first stage: we only had to read the cities and their coordinates, without worrying about how distances were calculated. The lower-diagonal format was a bit more challenging because we had to rebuild the full matrix from its lower-diagonal representation.

We split the work as follows: Amine took care of the lower-diagonal matrix parser, and Mehdi took care of the coordinate parser. 

### 2.2 Developing the Matrix Parser

When it came to developing the matrix parser, we first started by looking through the lower-diagonal instance files to understand how the weights were listed. We read the instance name, type, and number of cities before reaching the most important part: `EDGE_WEIGHT_SECTION`. The number of cities was used to create an empty matrix. We then rebuilt the whole matrix by reading one number for the first row, two for the second, and so on, then copying each value to its matching position in the upper half (since the matrix is symmetric). The `AdjacenceMatrixWeightedGraph` class from the previous lab was reused to store the completed matrix. The matrix was also saved in a `.matrix.txt` file for inspection; these exports were later removed. Initially, reconstructing the matrix and solving were done within the same file, but the solving and output code was later moved to `Trivial_Solver.cpp` when refactoring for cleaner code.

### 2.3 Developing the Coordinate Parser

We initially implemented the coordinate parser as a class called `parser`, with its declaration in `parser.hpp`, its methods in `parser.cpp`, and a separate `main.cpp` to run it. The class first read and stored the file's lines, then extracted the metadata and located `NODE_COORD_SECTION`. From there, it read each city's ID and x/y coordinates until reaching `EOF`. It also included a method to write the trivial solution, and the initial program used `berlin52.tsp` as its input. During integration, the class methods were combined into a function in `Coord_Parser.cpp`, keeping the same parsing steps. As with the matrix parser, the tour-writing code was later moved to `Trivial_Solver.cpp`. An extra condition was also added to handle the space before `EOF` in `ulysses16.tsp`.

### 2.4 Integrating and Organizing the Program


At first, the two parsers were two separate programs. However, when refactoring, the parsers were combined into one program that detected the input format from the section marker inside the file. We ended up with three main `.cpp` files: `Trivial_Solver.cpp`, `Coord_Parser.cpp`, and `Parser_Matrix.cpp`. Additionally, instead of writing from each of the parsers, both writing functions were moved to `Trivial_Solver.cpp`.


### 2.5 Testing and Corrections

At first, we tried the program on lower-diagonal and coordinate instances manually and checked that the resulting tour output was correct (ascending ID order, followed by `-1` and `EOF`). One issue that arose was that the `ulysses16.tsp` file had a space before `EOF`, so a condition was added so the file could be parsed correctly. We also added two automated parser tests for `gr17` and `berlin52`. This validation process was necessary to have correct output, as incorrect output can be generated if the parsing or output handling is wrong. 


## 3. Code Structure and Design Choices

### 3.1 Organization of the Source Files

We organized the files according to what each part of the program does:

- `Parser_Matrix.cpp`: reads the lower-diagonal values and rebuilds the matrix, then returns the name and graph data.
- `Coord_Parser.cpp`: reads the information at the start of the file, followed by the city IDs and their coordinates.
- `Trivial_Solver.cpp`: contains `main()`. It reads the arguments, chooses the right parser, creates the trivial tour, and saves it to the output file.
- `Parser.hpp`: declares the two parser functions so the solver can call them. It also defines the structures used to pass their results back to the solver.
- Graph classes: these come from the previous lab. `Graph` describes the basic graph operations, `WeightedGraph` adds operations involving weights, and `AdjacenceMatrixWeightedGraph` stores our matrix.
- `tests/tests.cpp`: checks a few values read from `gr17` and `berlin52` to see whether both parsers work as expected.
- `CMakeLists.txt`: lists what needs to be compiled to build the program and its tests.
- `build-TSPsolver.sh`: lets us build everything with one command.
- `TSP-instances/`: keeps the input files together, separate from the source code.

We wanted to avoid keeping everything in one file. Each parser takes care of reading its own format, while the solver takes care of creating and writing the tour. Keeping these parts separate makes the code easier to follow and lets us work on one part without mixing it with the rest. Later, we can add new solving methods and keep using the same parsers. There is still only one executable to run, and it chooses the parser from the contents of the input file.

### 3.2 Data Representations


**Matrix Instances:** We used the `AdjacenceMatrixWeightedGraph` class to represent the full matrix. Each position `[i][j]` contains the distance between cities i and j. The matrix is rebuilt in memory, without creating another file. Once the matrix was reconstructed as explained previously, we used the `TSPData` structure to hold that graph, the number of cities, and the type of problem, with the parser returning the instance name as well.

**Coordinate Instances:** `CoordinateData` stores the metadata in a map and uses the three vectors `indexs`, `xs` and `ys`, with `indexs[i]`, `xs[i]`, and `ys[i]` holding the ID and coordinates of the city at position i. However, our coordinate parser does not calculate distances yet. The trivial method only requires the IDs of the cities.

We kept both representations because they already provide what the trivial method needs.

### 3.3 Interaction Between Components

The components interact in the following manner:

**Matrix**: `main()` in `Trivial_Solver.cpp` detects the format, and `parse_tsp()` returns an instance name and graph data, then `trivial_solution()` creates the tour, while `write_tour()` saves it.

**Coordinates**: As with the matrix, `main()` detects the format, `parse_coordinates()` returns the IDs and coordinates, then the solver sorts the IDs and `write_coordinate_tour()` saves them.

All these checks are done using the contents of the file rather than the folder names.


## 4. Trivial Method and Program Usage

### 4.1 Trivial Algorithm

- Explain ascending city-ID order using a small example.
- Explain the implicit return to the first city.
- State what makes the tour valid and why it need not be shortest.
- Explain why weights do not influence this method's ordering.
- If discussing complexity, separate linear matrix-tour creation, coordinate-ID
  sorting in O(n log n), and matrix parsing/storage in O(n²).

### 4.2 Building and Running the Program

Include commands you have verified:

```bash
bash build-TSPsolver.sh
./build/tests
./build/TSPsolver TSP-instances/coord/berlin52.tsp berlin52.tour
./build/TSPsolver TSP-instances/lower_diag/gr17.tsp gr17.tour -m trivial
```

- State that commands run from the project root.
- Identify dependencies: Bash, CMake 3.14 or newer, and a C++20 compiler.
- Explain that compilation produces `build/TSPsolver` and `build/tests`.
- Explain both mandatory arguments and the optional method argument.
- State that trivial is the default and other methods are not yet implemented.
- Explain that relative paths are interpreted from the current working directory.

### 4.3 Solution File Format

- Explain `NAME`, `TYPE : TOUR`, `DIMENSION`, and `TOUR_SECTION`.
- Describe one occurrence per city, followed by `-1` and `EOF`.
- Explain why the first city is not repeated in the stored list.
- State that output goes to the exact supplied path and its parent folder must exist.
  There is no automatic redirection into a tour directory.

## 5. Validation and Difficulties

### 5.1 Automated Tests

Describe the two tests delivered in `tests/tests.cpp`:

- `gr17`: 17 vertices and weight 633 between the first two vertices in both directions.
- `berlin52`: 52 IDs and coordinate pairs, first/last IDs, and first position (565, 575).

Explain where expected values come from. Describe the passing message and the
failure exit status. These tests read existing inputs without writing files.
They check selected parser results, not all weights or the complete solver/output flow.

### 5.2 Additional Checks

- Describe manual or separate automated checks actually performed or reviewed.
- Distinguish those checks from the two repository tests.
- For each, identify the input, command, expected result, and observed result.
- Useful checks include city count, uniqueness, order, output path, default method,
  and explicit `-m trivial`.
- If stating all supplied instances passed, describe the checking method and
  whether the check was performed by you or through AI assistance.

### 5.3 Problems Encountered and Solutions

For each problem, explain its symptom, cause, correction, and verification:

- Ulysses: the leading space before `EOF` breaks an exact comparison. Explain the
  additional condition and its limited scope, rather than claiming general whitespace handling.
- City ordering: coordinate file order need not match ascending ID order.
- Output paths: a created folder is not necessarily the requested output location;
  explain why the second argument determines the path.
- Describe other integration or compilation problems only if actually encountered.

### 5.4 Current Limitations

- Optimization algorithms are not implemented yet.
- The two tests provide limited coverage.
- The coordinate parser assumes expected formatting apart from the specific EOF fix.
- Coordinate distances are not calculated by the current trivial-method flow.
- Check the actual implementation before claiming any additional limitation or capability.

## 6. Reflections and Next Steps

- What did each member learn about C++, parsing, graphs, building, or collaboration?
- Which decisions helped integration, and which caused extra work?
- What can you now explain or implement independently?
- Identify specific improvements rather than simply saying the code should be cleaner.
- Outline Held–Karp with tour reconstruction, another algorithm discussed with the
  teacher, and later runtime/quality/scalability evaluation.
- Clearly distinguish plans from completed features.

## 7. References and AI-Use Disclosure

### 7.1 References

- List the assignment and resources actually consulted, with title, location, and purpose.
- Identify supplied datasets and the instance generator as provided material.

### 7.2 Use of AI Tools

- Identify the tool and actual assistance: explanations, debugging, restructuring,
  integration, generated tests, and generated or modified implementation.
- Identify affected files or functions. Distinguish your work, your teammate's work,
  and AI contributions.
- Explain how suggestions were inspected, tested, changed, accepted, or rejected.
- Reflect on learning and on suggestions you initially did not understand.
- Check that code comments identify the tool and date where required.
- Write this disclosure yourself; the assignment prohibits AI generation of the report.

### 7.3 DALIA Self-Assessment

- Assess the actual involvement using the course's definitions.
- Justify it with concrete activities rather than the number of prompts.
- Describe mixed contributions accurately and ask the teacher if classification is unclear.
- Do not choose a level solely because it is the permitted maximum.
