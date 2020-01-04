/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <mozart++/core/type_traits.hpp>
#include <mozart++/function>

namespace mpp {
    template<typename F, typename = mpp::void_t<>>
    struct is_full_curried : public mpp::false_type {
    };

    template<typename F>
    struct is_full_curried
            <F,
                    mpp::void_t<decltype(std::declval<F>()())>
            > : public mpp::true_type {
    };

    template<typename F>
    struct curried_function;

    template<typename R, typename Arg>
    struct curried_function<R(Arg)> {
        using type = mpp::function<R(Arg)>;
    };

    template<typename R, typename Arg1, typename ...Args>
    struct curried_function<R(Arg1, Args...)> {
        using reduced_type = typename curried_function<R(Args...)>::type;
        using type = mpp::function<reduced_type(Arg1)>;
    };

    template<typename F>
    static auto curry(F &&f) -> decltype(curry(function_type<F>(std::forward<F>(f))));

    template<typename R>
    static mpp::function<R()> curry(mpp::function<R()> &&f);

    template<typename R, typename Arg>
    static mpp::function<R(Arg)> curry(mpp::function<R(Arg)> &&f);

    template<typename R, typename Arg, typename ...ArgsT>
    static typename curried_function<R(Arg, ArgsT...)>::type curry(mpp::function<R(Arg, ArgsT...)> &&f) {
        // given the code:
        // curry(f)(1)(2)(3)
        // which is equal to f(1, 2, 3)

        // return a function that takes the (1)
        return [=](Arg &&arg) {
            // bind arg to the first argument of f
            // and return a new function that takes
            // the rest of the arguments (2)(3)
            return curry<R, ArgsT...>(mpp::function<R(ArgsT...)>(
                    [=](ArgsT &&...rest) {
                        // make the call
                        return f(arg, std::forward<ArgsT>(rest)...);
                    }));
        };
    }

    /**
     * Curry a function.
     *
     * @tparam R Function return type
     * @param f Function to be curried
     * @return Curried function
     */
    template<typename R>
    static mpp::function<R()> curry(mpp::function<R()> &&f) {
        return f;
    }

    /**
     * Curry a function.
     *
     * @tparam R Function return type
     * @tparam Arg Function argument types
     * @param f Function to be curried
     * @return Curried function
     */
    template<typename R, typename Arg>
    static mpp::function<R(Arg)> curry(mpp::function<R(Arg)> &&f) {
        return f;
    }

    /**
     * Curry a function.
     *
     * @tparam F Function type
     * @param f Function to be curried
     * @return Curried function
     */
    template<typename F>
    static decltype(auto) curry(F &&f) {
        return curry(function_type<F>(std::forward<F>(f)));
    }
}
