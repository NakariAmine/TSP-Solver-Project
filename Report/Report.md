# TSP Solver — Trivial Method Progress Report

**Authors:** Amine Nakari, Mohammed Mehdi Berrahou  
**Course:** Programming 

## 1. Introduction

**Purpose:** explain what the project is and what this version does.

- Define the travelling salesman problem in your own words: cities, weighted
  connections, visiting every city once, and returning to the starting city.
- Distinguish finding a shortest tour from the current milestone, which constructs
  a trivial tour without optimizing its length.
- Introduce the two input representations: coordinates and explicit lower-diagonal weights.
- Name the team members and explain the actual division of work: matrix parser,
  coordinate parser, integration, and testing.
- Briefly identify the deliverables: sources, build script, solver, and basic tests.

**Suggested length:** one or two short paragraphs. Keep implementation details for Section 3.

## 2. Chronological Account of the Work

**Purpose:** describe how the project developed, rather than only its final features.
For each stage, explain the starting situation, action, difficulty, result, and lesson.
Use actual dates where known; otherwise describe stages in their real order.

### 2.1 Understanding the Requirements and Planning

- Which requirements did you identify first? How did they influence your plan?
- How did you understand the expected arguments, file formats, and trivial method?
- Why did you divide the work between the two parsers?
- What did each member already know, and what needed learning?
- Identify tools actually used: editor, compiler, CMake, Bash, Git if used, and AI.
  Explain each tool's role rather than simply listing names.

### 2.2 Developing the Matrix Parser

- Describe the initial instance you inspected and how you understood its header and data.
- Explain the implementation stages: metadata, dimension, weights, symmetric matrix,
  and graph construction.
- Discuss difficulties you actually encountered with indexing or reading weights
  across lines. Explain how you checked the results.
- If matrix text exports were used during development, explain their temporary
  purpose and why they were removed.
- Distinguish your original implementation from subsequent assistance and modifications.

### 2.3 Developing the Coordinate Parser

- Ask your teammate to describe their actual process and design decisions.
- Describe the original class and its stages: storing lines, extracting metadata,
  locating the coordinate section, and reading IDs/x/y.
- Explain the integration as a function while retaining those stages.
- Describe initial checks, difficulties, and lessons learned by your teammate.

### 2.4 Integrating and Organizing the Program

- Explain the starting situation: separate parsers and separate entry points.
- Describe moving source files out of the instance directory.
- Explain automatic input-format detection and the move to one executable.
- Describe why you settled on three main implementation files and a shared header.
- Mention revisions when they explain a decision or lesson, without recounting every edit.
- Describe the tools and AI assistance actually used for integration and restructuring.

### 2.5 Testing and Corrections

- Describe progression from trying individual instances to basic automated tests.
- Explain how problems were found and how fixes were checked.
- Mention the Ulysses EOF issue, ID ordering, and output paths where relevant to your process.
- Reflect on why success on one instance does not establish correctness on every instance.

## 3. Code Structure and Design Choices

**Purpose:** explain the final organization and its reasons, without repeating the chronology.

### 3.1 Organization of the Source Files

Explain these responsibilities in paragraphs or a table:

- `Parser_Matrix.cpp`: reads lower-diagonal weights and returns the instance name and graph data.
- `Coord_Parser.cpp`: returns coordinate metadata, city IDs, and x/y values.
- `Trivial_Solver.cpp`: handles arguments, detects the input format, generates the
  trivial tour, and writes the solution.
- `Parser.hpp`: declares parser functions and the shared data structures. Explain
  why callers need declarations when implementations are in separate files.
- Graph classes: distinguish the general graph interface, weighted interface, and
  concrete adjacency-matrix implementation.
- `tests/tests.cpp`: contains the two parser checks.
- `CMakeLists.txt` and `build-TSPsolver.sh`: configure compilation and provide the build command.
- `TSP-instances/`: holds datasets, rather than implementation files.

Explain the separation of parsing from solving and the choice of one executable.
Do not imply that DFS or path-weight calculation is used by the trivial solver.

### 3.2 Data Representations

- Describe the fields of `TSPData` and `CoordinateData` and their purposes.
- Explain why symmetric distances need only a lower triangle in the input.
- Consider a three-city example showing how each weight fills both `matrix[i][j]`
  and `matrix[j][i]`.
- Distinguish reconstruction in memory from creating a matrix file on disk.
- Explain why coordinates are retained even though trivial ordering only needs IDs.
- Discuss retaining the existing representations versus immediately introducing a unified graph model.

### 3.3 Interaction Between Components

Trace both paths in your own words:

- Matrix: arguments → detect `EDGE_WEIGHT_SECTION` → parse weights → construct
  graph → generate indices → convert to output IDs → write tour.
- Coordinates: arguments → detect `NODE_COORD_SECTION` → read IDs/x/y → sort
  tour IDs → write tour.

Explain that detection uses contents rather than folder names. Explain zero-based
matrix indices versus original IDs in coordinate inputs.

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
