/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/event>
#include <string>

class REPL : public mpp::event_emit {
};

class BaseDispatcher : public mpp::event_emit {
public:
    BaseDispatcher() {
        on("int", [](int i) {
            printf("BaseDispatcher: got an %d\n", i);
            return false;
        });
    }
};

class DerivedDispatcher : public BaseDispatcher {
public:
    DerivedDispatcher() {
        // TODO: support override handlers
        on("int", [](int i) {
            printf("DerivedDispatcher: got an %d\n", i);
            return true;
        });
    }
};

int main(int argc, const char **argv) {
    REPL repl;

    // register event handlers
    repl.on("SIGINT", []() {
        printf("Keyboard Interrupt (Ctrl-C pressed)\n");
        return true;
    });

    repl.on("expr", [](const std::string &expr) {
        printf("evaluating: %s\n", expr.c_str());
        return true;
    });

    repl.on("command", [](const std::string &opt) {
        printf("applying command: %s\n", opt.c_str());
        return true;
    });

    // simulate real-world situation
    repl.emit("command", std::string("b main"));
    repl.emit("expr", std::string("system.run(\"rm -rf --no-preserve-root /\")"));
    repl.emit("SIGINT");

    DerivedDispatcher dispatcher;
    dispatcher.emit("int", 100);
}

