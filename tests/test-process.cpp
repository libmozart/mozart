/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <cstdio>
#include <mozart++/system/process.hpp>

using mpp::process;
using mpp::process_builder;

int main(int argc, const char **argv) {
    process p = process::exec("/bin/bash");
    p.get_stdin() << "ls /\n";
    p.get_stdin() << "exit\n";

    std::string s;
    while (std::getline(p.get_stdout(), s)) {
        printf("%s\n", s.c_str());
    }

    p.wait_for();
    return 0;
}
