#include "graph.hpp"

#include <stdexcept>

std::vector<size_t> Graph::find_path(
    size_t start,
    size_t end
) const
{
    if (start >= size() || end >= size()) {
        throw std::out_of_range("Invalid vertex index");
    }

    std::vector<bool> visited(size(), false);
    std::vector<size_t> path;

    if (dfs(start, end, visited, path)) {
        return path;
    }

    return {};
}

bool Graph::dfs(
    size_t current,
    size_t end,
    std::vector<bool>& visited,
    std::vector<size_t>& path
) const
{
    visited[current] = true;
    path.push_back(current);

    if (current == end) {
        return true;
    }

    for (size_t neighbor : get_neighbors(current)) {
        if (!visited[neighbor]) {
            if (dfs(neighbor, end, visited, path)) {
                return true;
            }
        }
    }

    path.pop_back();
    return false;
}