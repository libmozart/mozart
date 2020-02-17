/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <cstdio>
#include <mozart++/fdstream>

int main() {
    FILE *fp = fopen("hello.txt", "w");

    mpp::fdostream fdout(fileno(fp));
    fdout << "fuck";
    fdout << "cpp";
    fclose(fp);

    fp = fopen("hello.txt", "r");
    mpp::fdistream fdin(fileno(fp));

    std::string x;
    fdin >> x;

    fclose(fp);

    if (x != "fuckcpp") {
        fprintf(stderr, "test-fdstream: failed\n");
        return 1;
    }

    return 0;
}
