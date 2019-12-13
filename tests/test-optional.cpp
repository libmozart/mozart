/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/optional>
#include <string>
#include <vector>
#include <cstdio>

void sayHi(mpp::optional<std::string> name) {
    if (name.has_value()) {
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
    sayHi(mpp::optional<std::string>::none());
    sayHi(mpp::optional<std::string>::from("imkiva"));

    std::vector<int> v{1, 2, 3, 4};
    printf("sum1 = %d\n", sum(mpp::optional<decltype(v)>::from(v)));
    printf("sum2 = %d\n", sum(mpp::optional<decltype(v)>::none()));
    printf("sum3 = %d\n", sum(mpp::optional<decltype(v)>::emplace(1, 2, 3, 4, 5)));
}
