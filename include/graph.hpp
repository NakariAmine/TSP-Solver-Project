#pragma once

#include <vector>
#include <cstddef>

class Graph {
public:

    virtual size_t size() const = 0;

    virtual size_t nb_edges() const = 0;

    virtual bool edge_exists(size_t i, size_t j) const = 0;

    virtual std::vector<size_t> get_neighbors(size_t i) const = 0;

    std::vector<size_t> find_path(
        size_t start,
        size_t end
    ) const;

private:
bool dfs(
    size_t current,
    size_t end,
    std::vector<bool>& visited,
    std::vector<size_t>& path
) const;
};