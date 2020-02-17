/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <type_traits>

#define mpp_in_place_type_t(T)  mpp_impl::in_place_t(&)(mpp_impl::in_place_type_tag<T>)
#define mpp_in_place_index_t(T)  mpp_impl::in_place_t(&)(mpp_impl::in_place_index_tag<I>)

namespace mpp_impl {
    template <typename T>
    struct in_place_type_tag {};

    template <std::size_t I>
    struct in_place_index_tag {};

    struct in_place_t {};

    template <typename T>
    inline in_place_t in_place(in_place_type_tag<T> = in_place_type_tag<T>()) {
        return in_place_t{};
    }

    template <std::size_t I>
    inline in_place_t in_place(in_place_index_tag<I> = in_place_index_tag<I>()) {
        return in_place_t{};
    }

    template <typename T>
    inline in_place_t in_place_type(in_place_type_tag<T> = in_place_type_tag<T>()) {
        return in_place_t{};
    }

    template <std::size_t I>
    inline in_place_t in_place_index(in_place_index_tag<I> = in_place_index_tag<I>()) {
        return in_place_t{};
    }
}

namespace mpp {
    template <typename ...>
    using void_t = void;

    struct true_type {
        constexpr static bool value = true;
    };

    struct false_type {
        constexpr static bool value = false;
    };

    /**
     * Integer Sequence
     * usage: using seq = make_sequence_t<Maximum>;
     * @see mpp::make_sequence_t
     */
    template <unsigned int... seq>
    struct sequence final {
    };

    template <unsigned int N, unsigned int... seq>
    struct make_sequence : public make_sequence<N - 1, N - 1, seq...> {
    };

    template <unsigned int... seq>
    struct make_sequence<0, seq...> {
        using type = sequence<seq...>;
    };

    /**
     * Short for {@code typename make_sequence<Maximum>::type}
     */
    template <unsigned int N>
    using make_sequence_t = typename make_sequence<N>::type;
}
