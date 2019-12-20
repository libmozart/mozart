/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/string/string.hpp>
#include <iostream>

using mpp::string_ref;

int sum(string_ref val) {
    return val.stream()
        .reduce<int>(0, [](int acc, char c) {
            return std::isdigit(c) ? (acc + c - '0') : acc;
        });
}

void process_command(string_ref command) {
    if (command.startswith_ignore_case("run")) {
        if (command.contains("rm")) {
            printf("Cannot run dangerous command: %s\n",
                command.substr(4).str().c_str());
        } else {
            printf("process: exec: %s\n",
                command.substr(4).str().c_str());
        }
    } else if (command.contains_ignore_case("I love you")) {
        printf("I love you too!\n");
    } else if (command.startswith_ignore_case("sum")) {
        printf("=> %d\n", sum(command.substr(4)));
    }
}

int main() {
    std::string s;
    while (std::getline(std::cin, s)) {
        process_command(s);
    }
}
