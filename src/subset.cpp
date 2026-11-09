#include "subset.hpp"

#include <limits>
#include <stdexcept>

namespace subset {

Subset::Subset(std::size_t k, std::size_t n) : n(n) {
    constexpr auto bits = std::numeric_limits<unsigned long long>::digits;
    if (k > n || n > bits) {
        throw std::runtime_error("Require k <= n <= the bit representation width");
    }
    bit_rep = k == bits ? ~0ULL : (1ULL << k) - 1;
}

std::ostream& operator<<(std::ostream& os, const Subset& s) {
    os << '{';
    bool first = true;
    for (auto element : s) {
        if (!first) os << ',';
        os << element;
        first = false;
    }
    return os << '}';
}

bool Subset::contains(std::size_t i) const {
    if (i >= n) throw std::runtime_error("Index out of range");
    return (bit_rep & (1ULL << i)) != 0;
}

void Subset::add(std::size_t i) {
    if (contains(i)) throw std::runtime_error("Element already exists");
    bit_rep |= 1ULL << i;
}

void Subset::minus(std::size_t i) {
    if (!contains(i)) throw std::runtime_error("Element does not exist");
    bit_rep &= ~(1ULL << i);
}

bool Subset::contains(const Subset& other) const {
    if (n != other.n) throw std::runtime_error("Subsets must have the same n");
    return (bit_rep & other.bit_rep) == other.bit_rep;
}

void Subset::add(const Subset& other) {
    if (n != other.n) throw std::runtime_error("Subsets must have the same n");
    if ((bit_rep & other.bit_rep) != 0) {
        throw std::runtime_error("Subsets overlap");
    }
    bit_rep |= other.bit_rep;
}

void Subset::minus(const Subset& other) {
    if (!contains(other)) throw std::runtime_error("Subset contains absent elements");
    bit_rep &= ~other.bit_rep;
}

ElementsIterator Subset::begin() const {
    std::size_t i = 0;
    while (i < n && !contains(i)) ++i;
    return ElementsIterator(*this, i);
}

ElementsIterator Subset::end() const { return ElementsIterator(*this, n); }

ElementsIterator::ElementsIterator(const Subset& subset, std::size_t element)
    : subset_(&subset), current_(element) {}

ElementsIterator::reference ElementsIterator::operator*() const { return current_; }
ElementsIterator::pointer ElementsIterator::operator->() const { return &current_; }

ElementsIterator& ElementsIterator::operator++() {
    if (subset_ && current_ < subset_->n) {
        do { ++current_; }
        while (current_ < subset_->n && !subset_->contains(current_));
    }
    return *this;
}

ElementsIterator ElementsIterator::operator++(int) {
    auto previous = *this;
    ++*this;
    return previous;
}

bool ElementsIterator::operator==(const ElementsIterator& other) const {
    return subset_ == other.subset_ && current_ == other.current_;
}

bool ElementsIterator::operator!=(const ElementsIterator& other) const {
    return !(*this == other);
}

SubsetIterator::SubsetIterator(std::size_t k, std::size_t n)
    : current(k, n), k(k), is_end(false) {}

SubsetIterator SubsetIterator::end() { return SubsetIterator(); }
SubsetIterator::reference SubsetIterator::operator*() const { return current; }
SubsetIterator::pointer SubsetIterator::operator->() const { return &current; }

SubsetIterator& SubsetIterator::operator++() {
    if (is_end) return *this;
    const auto x = current.bit_rep;
    // The empty subset is the sole combination when k == 0.
    if (x == 0) {
        is_end = true;
        return *this;
    }
    const auto c = x & (0ULL - x);
    const auto r = x + c;
    // Unsigned wraparound marks exhaustion at the representation boundary.
    if (r == 0) {
        is_end = true;
        return *this;
    }
    const auto next = (((r ^ x) >> 2) / c) | r;
    constexpr auto bits = std::numeric_limits<unsigned long long>::digits;
    if (current.n < bits && (next >> current.n) != 0) {
        is_end = true;
    } else {
        current.bit_rep = next;
    }
    return *this;
}

SubsetIterator SubsetIterator::operator++(int) {
    auto previous = *this;
    ++*this;
    return previous;
}

bool SubsetIterator::operator==(const SubsetIterator& other) const {
    if (is_end || other.is_end) return is_end == other.is_end;
    return k == other.k && current.n == other.current.n
        && current.bit_rep == other.current.bit_rep;
}

bool SubsetIterator::operator!=(const SubsetIterator& other) const {
    return !(*this == other);
}

}
