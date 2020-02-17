/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */


#pragma once

#include <utility>

namespace mpp {
    /**
     * A range adaptor for a pair of iterators.
     * This just wraps two iterators into a range-compatible interface.
     *
     * @tparam IteratorT
     */
    template <typename IteratorT>
    class iterator_range {
        IteratorT _begin_iterator;
        IteratorT _end_iterator;

    public:
        // TODO: Add SFINAE to test that the Container's iterators match the range's
        //      iterators.
        template <typename Container>
        iterator_range(Container &&c)
        // TODO: Consider std::begin()/std::end() calls.
                : _begin_iterator(c.begin()), _end_iterator(c.end()) {}

        iterator_range(IteratorT begin_iterator, IteratorT end_iterator)
                : _begin_iterator(std::move(begin_iterator)),
                  _end_iterator(std::move(end_iterator)) {}

        IteratorT begin() const {
            return _begin_iterator;
        }

        IteratorT end() const {
            return _end_iterator;
        }

        bool empty() const {
            return _begin_iterator == _end_iterator;
        }
    };

    /*
     * Convenience function for iterating over sub-ranges.
     * This provides a bit of syntactic sugar to make using sub-ranges
     * in for loops a bit easier. Analogous to std::make_pair().
     */

    template <class T>
    iterator_range<T> make_range(T x, T y) {
        return iterator_range<T>(std::move(x), std::move(y));
    }

    template <typename T>
    iterator_range<T> make_range(std::pair<T, T> p) {
        return iterator_range<T>(std::move(p.first), std::move(p.second));
    }
}
