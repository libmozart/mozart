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

void test1() {
    process p = process::exec("/bin/bash");
    p.in() << "ls /\n";
    p.in() << "exit\n";

    std::string s;
    while (std::getline(p.out(), s)) {
        printf("test1: %s\n", s.c_str());
    }

    p.wait_for();
}

void test2() {
    // the following code is equivalent to "/bin/bash 2>&1"
    process p = process_builder().command("/bin/bash")
        .redirect_error(true)
        .start();

    // write to stderr
    p.in() << "echo fuckcpp 1>&2\n";
    p.in() << "exit\n";

    // and we can get from stdout
    std::string s;
    p.out() >> s;

    if (s != "fuckcpp") {
        printf("test-process: test2: failed\n");
    }
}

int main(int argc, const char **argv) {
    test1();
    test2();
    return 0;
}
