/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/core>
#include <cstdio>

int main() {
    using namespace mpp;

    using t1 = typelist::list<int, double, float, char>;
    using size1 = typelist::size<t1>;
    using size2 = typelist::size<typelist::nil>;

    static_assert(size1::value == 4, "You wrote a bug");
    static_assert(size2::value == 0, "You wrote a bug");

    using e1 = typelist::nil;
    using e2 = typelist::cons<char, e1>;
    using e3 = typelist::cons<t1, e2>;

    static_assert(typelist::size<e1>::value == 0, "You wrote a bug");
    static_assert(typelist::size<e2>::value == 1, "You wrote a bug");
    static_assert(typelist::size<e3>::value == 2, "You wrote a bug");

    using e3t = typelist::tail<e3>;
    using e3h = typelist::head<e3>;

    static_assert(typelist::equals<e3t, e2>::value, "You wrote a bug");
    static_assert(typelist::equals<e3h, t1>::value, "You wrote a bug");

    using c1 = typelist::list<int, char>;
    using c2 = typelist::list<double, float>;
    using c3 = typelist::list<int, char, double, float>;

    static_assert(typelist::equals<c3, typelist::concat<c1, c2>>::value,
                  "You wrote a bug");
    static_assert(typelist::equals<c1, typelist::concat<c1, typelist::nil>>::value,
                  "You wrote a bug");
    static_assert(typelist::equals<c2, typelist::concat<typelist::nil, c2>>::value,
                  "You wrote a bug");
}
