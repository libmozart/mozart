/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <mozart++/core>
#include <vector>
#include <deque>
#include <list>

namespace mpp {
    /**
     * The stream class provides a Java-like Stream operation.
     *
     * @tparam T Element type
     */
    template <typename T>
    class stream {
    public:
        template <typename R>
        using mapper_type = mpp::function<R(T x)>;

        using predicate_type = mapper_type<bool>;
        using producer_type = mapper_type<T>;
        using consumer_type = mapper_type<void>;

    private:
        T _head;
        std::deque<T> _finite_data;
        bool _remaining = true;
        bool _finite_stream = false;

        producer_type _producer;
        predicate_type _predicate;
        mapper_type<T> _mapper;

    private:
        T produce_next(const T &head) {
            if (_finite_stream) {
                if (this->_finite_data.empty()) {
                    this->_remaining = false;
                    return head;
                }
                T x = this->_finite_data.front();
                this->_finite_data.pop_front();
                return x;
            } else {
                return _producer(head);
            }
        }

        T take_head() {
            T head = _head;
            if (_remaining) {
                _head = produce_next(_head);
            }
            return std::move(head);
        }

        T eval_head() {
            T mapped = _mapper(take_head());
            while (!_predicate(mapped)) {
                mapped = _mapper(take_head());
            }
            return std::move(mapped);
        }

        void drop_head() {
            (void) eval_head();
        }

        stream<T> &iterate(const producer_type &iterator) {
            this->_producer = compose<T, T, T>(iterator, this->_producer);
            return *this;
        }

        explicit stream(const T &head)
                : _head(head),
                  _remaining(true),
                  _finite_stream(false),
                  _producer([](T x) {
                      return x;
                  }),
                  _predicate([](T x) {
                      return true;
                  }),
                  _mapper([](T x) {
                      return x;
                  }) {
        }

        explicit stream(std::deque<T> list)
                : _head(),
                  _finite_data(std::move(list)),
                  _finite_stream(true),
                  _remaining(!_finite_data.empty()),
                  _producer([](T x) {
                      return x;
                  }),
                  _predicate([](T x) {
                      return true;
                  }),
                  _mapper([](T x) {
                      return x;
                  }) {
            // Bind the head to the first element of list
            drop_head();
        }

    public:
        stream() = delete;

        ~stream() = default;

        stream<T> &operator=(const stream<T> &rhs) {
            if (this != &rhs) {
                this->~stream();
                new(this) stream(rhs);
            }
            return *this;
        }

    public:
        stream<T> &filter(const predicate_type &predicate) {
            this->_predicate = boolean_compose<T>(predicate, this->_predicate);
            return *this;
        }

        stream<T> &map(const mapper_type<T> &mapper) {
            this->_mapper = compose<T, T, T>(mapper, this->_mapper);
            return *this;
        }

        stream<T> &drop(int n) {
            while (_remaining && n-- > 0) {
                drop_head();
            }
            return *this;
        }

        stream<T> &drop_while(const predicate_type &predicate) {
            return filter([=](T x) {
                return !predicate(x);
            });
        }

        stream<T> &travel(const predicate_type &predicate) {
            if (_remaining) {
                T head = eval_head();
                while (_predicate(head)) {
                    if (!predicate(head) || !_remaining) {
                        break;
                    }
                    head = eval_head();
                }
            }
            return *this;
        }

        stream<T> &peek(const consumer_type &consumer) {
            return travel([&](T t) {
                consumer(t);
                return true;
            });
        }

        stream<T> take(int n) {
            return stream<T>::of(collect(n));
        }

        stream<T> take_while(const predicate_type &predicate) {
            return stream<T>::of(collect(predicate));
        }

        std::vector<T> collect() {
            return collect([](T) {
                return true;
            });
        }

        std::vector<T> collect(int n) {
            std::vector<T> values;
            values.reserve(n);
            while (_remaining && n-- > 0) {
                values.emplace_back(eval_head());
            }
            return std::move(values);
        }

        std::vector<T> collect(const predicate_type &predicate) {
            std::vector<T> values;
            if (_remaining) {
                T head = eval_head();
                while (predicate(head)) {
                    values.emplace_back(std::move(head));
                    if (!_remaining) {
                        break;
                    }
                    head = eval_head();
                }
            }
            return std::move(values);
        }

        /**
         * Drop the first element
         * @return stream containing the rest elements
         */
        stream<T> &tail() {
            return drop(1);
        }

        /**
         * Take the first element of the stream
         * @return the first element
         */
        T head() {
            return collect(1)[0];
        }

        /**
         * Take the first element of the stream
         * @return the first element
         */
        T head_or(T backup) {
            auto &&ts = collect(1);
            return ts.empty() ? backup : ts[0];
        }

        void for_each(const consumer_type &consumer) {
            peek(consumer);
        }

        template <typename U = T>
        U reduce(U identity, const mpp::function<U(U, T)> &f) {
            U acc = identity;
            for_each([&](T t) {
                acc = f(acc, t);
            });
            return acc;
        }

        bool any(const predicate_type &predicate) {
            bool match = false;
            travel([&](T t) {
                if (predicate(t)) {
                    match = true;
                }
                return !match;
            });
            return match;
        }

        bool none(const predicate_type &predicate) {
            return any([&](T x) {
                return !predicate(x);
            });
        }

        bool all(const predicate_type &predicate) {
            return !none(predicate);
        }

        size_t count(const predicate_type &predicate) {
            return collect(predicate).size();
        }

        size_t count() {
            return collect().size();
        }

    public:
        /**
         * Construct a stream by repeating a value.
         *
         * @param head
         * @return stream
         */
        static stream<T> repeat(const T &head) {
            return stream<T>(head);
        }

        /**
         * Construct a stream by repeatedly applying a function.
         *
         * @param head The first element
         * @param iterator The mapper function
         * @return stream
         */
        static stream<T> iterate(T head, const producer_type &iterator) {
            return stream<T>::repeat(head).iterate(iterator);
        }

        /**
         * Construct a stream from a list.
         *
         * @param list The list
         * @return stream
         */
        static stream<T> of(const std::vector<T> &list) {
            std::deque<T> d;
            std::copy(list.begin(), list.end(), std::back_inserter(d));
            return stream<T>(std::move(d));
        }

        /**
         * Construct a stream from a list.
         *
         * @param list The list
         * @return stream
         */
        static stream<T> of(const std::list<T> &list) {
            std::deque<T> d;
            std::copy(list.begin(), list.end(), std::back_inserter(d));
            return stream<T>(std::move(d));
        }

        /**
         * Construct a stream from a list.
         *
         * @param list The list
         * @return stream
         */
        static stream<T> of(std::deque<T> list) {
            return stream<T>(std::move(list));
        }
    };
}
