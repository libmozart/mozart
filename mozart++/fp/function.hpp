/**
 * Mozart++ Template Library: Utility
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <mozart++/core/base.hpp>
#include <functional>

namespace mpp {
    using std::function;

    template <typename A, typename B, typename C>
    static mpp::function<C(A)> compose(const mpp::function<C(B)> &f,
                                       const mpp::function<B(A)> &g) {
        return std::bind(f, std::bind(g, std::placeholders::_1));
    }

    template <typename A>
    static mpp::function<bool(A)> boolean_compose(const mpp::function<bool(A)> &f,
                                                  const mpp::function<bool(A)> &g) {
        return [=](A x) { return f(x) && g(x); };
    }

    template <typename Handler>
    struct function_parser : public function_parser<decltype(&Handler::operator())> {
    };

    template <typename ClassType, typename R, typename... Args>
    struct function_parser<R(ClassType::*)(Args...) const> {
        using function_type = mpp::function<R(Args...)>;
    };

    template <typename Handler>
    using function_type = typename function_parser<Handler>::function_type;
}
