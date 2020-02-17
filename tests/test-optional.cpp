/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/optional>
#include <string>
#include <vector>
#include <cstdio>

void sayHi(mpp::optional<std::string> name) {
    if (name) {
        printf("hi: %s\n", name.get().c_str());
    } else {
        printf("you didn't tell me who you are) QwQ\n");
    }
}

int sum(const mpp::optional<std::vector<int>> &nums) {
    return nums.apply_or<int>(0, [](auto &&v) -> int {
        int s = 0;
        for (auto &&e : v) {
            s += e;
        }
        return s;
    });
}

int main(int argc, const char **argv) {
    sayHi(mpp::none);
    sayHi(mpp::optional<std::string>("hello"));
    sayHi(mpp::some<std::string>("world"));
    sayHi(std::string("imkiva"));

    std::vector<int> v{1, 2, 3, 4};
    printf("sum1 = %d\n", sum(mpp::some<decltype(v)>(v)));
    printf("sum2 = %d\n", sum(mpp::none));
    printf("sum3 = %d\n", sum(mpp::some<decltype(v)>(1, 2, 3, 4, 5)));
}
