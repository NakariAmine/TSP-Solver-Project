# Prim's algorithm: finding a minimum spanning tree (MST)

For a connected, undirected, weighted graph:

1. Create an empty set `mstSet` to track vertices included in the MST.
2. Create two dictionaries:
   - `key[v]`: cheapest known edge weight connecting `v` to the current tree. Initially ∞.
   - `parent[v]`: the vertex providing that connection. Initially none.
3. Choose a starting vertex and set `key[start] = 0`.
4. Repeat until every vertex is in `mstSet`:
   - Choose `u` outside `mstSet` with the smallest `key[u]`.
   - Add `u` to `mstSet`.
   - Check every neighbour `v` of `u` still outside `mstSet`.
   - If `weight(u, v) < key[v]`, update:
     - `key[v] = weight(u, v)`
     - `parent[v] = u`
5. The MST consists of the edges `(parent[v], v)`, for every vertex except the starting one.

**Each iteration adds one vertex, but may update the saved connections of several other vertices.**
