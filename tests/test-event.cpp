/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/core>
#include <string>

class REPL : public mpp::event_emitter {
};

class BaseDispatcher : public mpp::event_emitter {
public:
    BaseDispatcher() {
        on("int", [](int i) {
            printf("BaseDispatcher: got an %d\n", i);
        });
    }
};

class DerivedDispatcher : public BaseDispatcher {
public:
    DerivedDispatcher() {
        // TODO: support override handlers
        this->unregister_event("int");
        on("int", [](int i) {
            printf("DerivedDispatcher: got an %d\n", i);
        });
    }
};

void test_func(char a, int b, double c, std::string &d) {
    printf("%c, %d, %lf, %s\n", a, b, c, d.c_str());
    d = "World";
}

int main(int argc, const char **argv) {
    REPL repl;

    // register event handlers
    repl.on("SIGINT", []() {
        printf("Keyboard Interrupt (Ctrl-C pressed)\n");
    });

    repl.on("expr", [](const std::string &expr) {
        printf("evaluating: %s\n", expr.c_str());
    });

    repl.on("command", [](const std::string &opt) {
        printf("applying command: %s\n", opt.c_str());
    });

    repl.on("test", test_func);

    // simulate real-world situation
    repl.emit("command", std::string("b main"));
    repl.emit("expr", std::string("system.run(\"rm -rf --no-preserve-root /\")"));
    repl.emit("SIGINT");
    std::string str("Hello");
    repl.emit("test", '@', 12, 3.14, str);
    printf("%s\n", str.c_str());

    DerivedDispatcher dispatcher;
    dispatcher.emit("int", 100);
}
