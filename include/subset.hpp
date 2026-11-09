#pragma once

#include <cstddef>
#include <iterator>
#include <ostream>

namespace subset {

class SubsetIterator;

class Subset {
public:
    class ElementsIterator {
    public:
        using value_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::size_t*;
        using reference = std::size_t;
        using iterator_category = std::forward_iterator_tag;

        ElementsIterator() = default;
        ElementsIterator(const Subset& subset, std::size_t element);
        reference operator*() const;
        pointer operator->() const;
        ElementsIterator& operator++();
        ElementsIterator operator++(int);
        bool operator==(const ElementsIterator& other) const;
        bool operator!=(const ElementsIterator& other) const;

    private:
        const Subset* subset_ = nullptr;
        std::size_t current_ = 0;
    };

    Subset() = default;
    Subset(std::size_t k, std::size_t n);

    friend std::ostream& operator<<(std::ostream& os, const Subset& s);
    bool contains(std::size_t i) const;
    void add(std::size_t i);
    void minus(std::size_t i);
    bool contains(const Subset& other) const;
    void add(const Subset& other);
    void minus(const Subset& other);

    ElementsIterator begin() const;
    ElementsIterator end() const;
    // Compatibility with the original example program.
    ElementsIterator my_subset_begin() const { return begin(); }
    ElementsIterator my_subset_end() const { return end(); }

private:
    unsigned long long bit_rep = 0;
    std::size_t n = 0;
    friend class ElementsIterator;
    friend class SubsetIterator;
};

using ElementsIterator = Subset::ElementsIterator;

class SubsetIterator {
public:
    using value_type = Subset;
    using difference_type = std::ptrdiff_t;
    using pointer = const Subset*;
    using reference = const Subset&;
    using iterator_category = std::input_iterator_tag;

    SubsetIterator() = default;
    // Match Subset(k, n) and the supplied lab tests.
    SubsetIterator(std::size_t k, std::size_t n);
    ~SubsetIterator() = default;
    reference operator*() const;
    pointer operator->() const;
    SubsetIterator& operator++();
    SubsetIterator operator++(int);
    bool operator==(const SubsetIterator& other) const;
    bool operator!=(const SubsetIterator& other) const;
    static SubsetIterator end();

private:
    Subset current;
    std::size_t k = 0;
    bool is_end = true;
};

}
