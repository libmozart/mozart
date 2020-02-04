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

void test_basic() {
    process p = process::exec("/bin/bash");
    p.in() << "ls /\n";
    p.in() << "exit\n";
    p.wait_for();

    std::string s;
    while (std::getline(p.out(), s)) {
        printf("process: test-basic: %s\n", s.c_str());
    }
}

void test_stderr() {
    // the following code is equivalent to "/bin/bash 2>&1"
    process p = process_builder().command("/bin/bash")
        .redirect_error(true)
        .start();

    // write to stderr
    p.in() << "echo fuckcpp 1>&2\n";
    p.in() << "exit\n";
    p.wait_for();

    // and we can get from stdout
    std::string s;
    p.out() >> s;

    if (s != "fuckcpp") {
        printf("process: test-stderr: failed\n");
    }
}

void test_env() {
    process p = process_builder().command("/bin/bash")
        .environment("VAR1", "fuck")
        .environment("VAR2", "cpp")
        .start();

    p.in() << "echo $VAR1$VAR2\n";
    p.in() << "exit\n";
    p.wait_for();

    std::string s;
    p.out() >> s;

    if (s != "fuckcpp") {
        printf("process: test-env: failed\n");
    }
}

void test_r_file() {
    // VAR=fuckcpp bash <<< "echo $VAR; exit" > output-all.txt

    FILE *fout = fopen("output-all.txt", "w");

    process p = process_builder().command("/bin/bash")
        .environment("VAR", "fuckcpp")
        .redirect_stdout(fileno(fout))
        .redirect_error(true)
        .start();

    p.in() << "echo $VAR\n";
    p.in() << "exit\n";
    p.wait_for();

    fclose(fout);

    fout = fopen("output-all.txt", "r");
    mpp::fdistream fin(fileno(fout));
    std::string s;
    fin >> s;

    if (s != "fuckcpp") {
        printf("process: test-redirect-file: failed\n");
    }
}

int main(int argc, const char **argv) {
    test_basic();
    test_stderr();
    test_env();
    test_r_file();
    return 0;
}
