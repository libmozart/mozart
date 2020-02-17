/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <array>
#include <utility>
#include <functional>
#include <mozart++/core>

namespace mpp {
    struct optional_none_t {
    };
    static constexpr optional_none_t none{};

    template <typename T>
    class optional {
    private:
        /**
         * One extra byte to store the status of this memory
         * memory[0] == true: this optional has a value
         * memory[0] == false: this optional has no value
         */
        std::array<unsigned char, 1 + sizeof(T)> _memory{0};

    public:
        optional() = default;

        /*implicit*/ optional(optional_none_t) : optional() {}

        /*implicit*/ optional(const T &t) {
            new(_memory.data() + 1) T(t);
            _memory[0] = static_cast<unsigned char>(true);
        }

        /*implicit*/ optional(T &&t) {
            new(_memory.data() + 1) T(std::forward<T>(t));
            _memory[0] = static_cast<unsigned char>(true);
        }

        template <typename... Args>
        constexpr explicit optional(mpp_in_place_type_t(T), Args &&... args)
                : optional(T{std::forward<Args>(args)...}) {}

        template <typename U, typename... Args>
        constexpr explicit optional(mpp_in_place_type_t(T), std::initializer_list<U> il, Args &&... args)
                : optional(T{il, std::forward<Args>(args)...}) {}

        optional(const optional<T> &other) {
            if (other.has_value()) {
                new(_memory.data() + 1) T(other.get());
                _memory[0] = static_cast<unsigned char>(true);
            }
        }

        optional(optional<T> &&other) noexcept {
            swap(other);
        }

        ~optional() {
            if (has_value()) {
                ptr()->~T();
                _memory[0] = static_cast<unsigned char>(false);
            }
        }

        optional &operator=(const optional<T> &other) {
            if (this == &other) {
                return *this;
            }

            optional<T> t(other);
            swap(t);
            return *this;
        }

        optional &operator=(optional<T> &&other) noexcept {
            if (this == &other) {
                return *this;
            }

            swap(other);
            return *this;
        }

        void swap(optional<T> &&other) {
            std::swap(this->_memory, other._memory);
        }

        void swap(optional<T> &other) {
            std::swap(this->_memory, other._memory);
        }

        /**
         * Get the pointer to the real object inside optional.
         *
         * Note that: if this optional wrapped a nullptr,
         * the nullptr will be returned.
         * Note: make sure you have checked {@code has_value()} before.
         *
         * @return pointer to the object
         */
        T *ptr() {
            return has_value() ? reinterpret_cast<T *>(_memory.data() + 1) : nullptr;
        }

        /**
         * Get the pointer to the real object inside optional.
         *
         * Note that: if this optional wrapped a nullptr,
         * the nullptr will be returned.
         * Note: make sure you have checked {@code has_value()} before.
         *
         * @return pointer to the object
         */
        const T *ptr() const {
            return has_value() ? reinterpret_cast<const T *>(_memory.data() + 1) : nullptr;
        }

        /**
         * Get the reference to the real object inside optional.
         *
         * Note that: if this optional wrapped a nullptr,
         * undefined behavior will happen (as dereferencing nullptr).
         * Note: make sure you have checked {@code has_value()} before.
         *
         * @return reference to the object
         */
        T &get() {
            return *ptr();
        }

        /**
         * Get the reference to the real object inside optional.
         *
         * Note that: if this optional wrapped a nullptr,
         * undefined behavior will happen (as dereferencing nullptr).
         * Note: make sure you have checked {@code has_value()} before.
         *
         * @return reference to the object
         */
        const T &get() const {
            return *ptr();
        }

        /**
         * Get the reference to the real object inside optional.
         *
         * Note that: if this optional wrapped a nullptr,
         * the default value {@param o} will be returned.
         *
         * @param o default value
         * @return reference to the object or to the default object
         */
        template <typename U>
        std::enable_if_t<std::is_same<T, U>::value, T> &get_or(U &&o) {
            if (ptr() == nullptr) {
                return o;
            }
            return *ptr();
        }

        /**
         * Get the reference to the real object inside optional.
         *
         * Note that: if this optional wrapped a nullptr,
         * the default value {@param o} will be returned.
         *
         * @param o default value
         * @return reference to the object or to the default object
         */
        template <typename U>
        const std::enable_if_t<std::is_same<T, U>::value, T> &get_or(U &&o) const {
            if (ptr() == nullptr) {
                return o;
            }
            return *ptr();
        }

        /**
         * Use the object inside optional.
         *
         * Note that: if this optional wrapped a nullptr,
         * nothing will happen.
         *
         * @param consumer The object receiver
         */
        void apply(const std::function<void(T &)> &consumer) {
            if (ptr() != nullptr) {
                consumer(get());
            }
        }

        /**
         * Use the object inside optional.
         *
         * Note that: if this optional wrapped a nullptr,
         * nothing will happen.
         *
         * @param consumer The object receiver
         */
        void apply(const std::function<void(const T &)> &consumer) const {
            if (ptr() != nullptr) {
                consumer(get());
            }
        }

        /**
         * Use the object inside optional.
         *
         * Note that: if this optional wrapped a nullptr,
         * the default value {@param r} will be returned.
         *
         * @param r default value
         * @param consumer The object receiver
         */
        template <typename R>
        R apply_or(const R &r, const std::function<R(T &)> &consumer) {
            if (ptr() != nullptr) {
                return consumer(get());
            }
            return r;
        }

        /**
         * Use the object inside optional.
         *
         * Note that: if this optional wrapped a nullptr,
         * the default value {@param r} will be returned.
         *
         * @param r default value
         * @param consumer The object receiver
         */
        template <typename R>
        R apply_or(const R &r, const std::function<R(const T &)> &consumer) const {
            if (ptr() != nullptr) {
                return consumer(get());
            }
            return r;
        }

        /**
         * Check whether this optional contains a valid value.
         *
         * @return true if the wrapped object is not nullptr.
         */
        bool has_value() const {
            return static_cast<bool>(_memory[0]);
        }

        /*implicit*/ operator bool() {
            return has_value();
        }
    };

    template <typename T>
    constexpr optional<typename std::decay<T>::type> some(T &&v) {
        return optional<typename std::decay<T>::type>(std::forward<T>(v));
    }

    template <typename T, typename... Args>
    constexpr optional<T> some(Args &&... args) {
        return optional<T>(mpp_impl::in_place, std::forward<Args>(args)...);
    }

    template <typename T, typename U, typename... Args>
    constexpr optional<T> some(std::initializer_list<U> il, Args &&... args) {
        return optional<T>(mpp_impl::in_place, il, std::forward<Args>(args)...);
    }
}

