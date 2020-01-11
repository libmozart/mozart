/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <type_traits>

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
