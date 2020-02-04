//
// Created by kiva on 2020/2/4.
//

#include <cstdio>
#include <mozart++/system/fdstream.hpp>

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
