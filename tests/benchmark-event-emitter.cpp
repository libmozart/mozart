//
// Created by kiva on 2019/12/30.
//
#include <mozart++/core>
#include <mozart++/timer>
#include <mozart++/string>
#include <string>

std::string show(const char *name) {
    return cxx_demangle(name);
}

static constexpr int TIMES = 1000000;

template <typename EE>
class BenchmarkClass : public EE {
};

template <typename EE>
struct BenchmarkRunner {
    static void doit() {
        BenchmarkClass<EE> ee;
        ee.on("bench-1", [](int x) {
            int z = x;
            return true;
        });
        ee.on("bench-2", []() { return true; });

        auto start = mpp::timer::time();
        for (int i = 0; i < TIMES; ++i) {
            ee.emit("bench-1", i);
            ee.emit("bench-2");
        }
        auto end = mpp::timer::time();

        auto s = show(typeid(EE).name());
        mpp::string_ref sr = s;

        printf("   benchmark of %16s: %zd(ms) for %d tests\n",
               sr.substr(sr.rfind("::") + 2).str().c_str(),
               end - start,
               TIMES);
    }
};

int main(int argc, const char **argv) {
    for (int i = 0; i < 5; ++i) {
        printf(":: Benchmark %d:\n", i);
        BenchmarkRunner<mpp::event_emitter_attentive>::doit();
        BenchmarkRunner<mpp::event_emitter_fast>::doit();
    }
}

