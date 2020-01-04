/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/fp/curry.hpp>

int main() {
    using mpp::curry;
    auto fn1 = curry([]() { return 10086; });
    auto fn2 = curry([](int a, int b, int c) { return a + b + c; });
    static_assert(mpp::is_full_curried<decltype(fn1)>::value, "You wrote a bug");

    printf("%d\n", fn1());
    printf("%d\n", fn2(1)(2)(3));

    auto add10 = fn2(2)(8);
    static_assert(!mpp::is_full_curried<decltype(add10)>::value, "You wrote a bug");

    printf("%d\n", add10(6));
}
