/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <cstdlib>
#include <string>
#include <list>
#include <unordered_map>

#include <mozart++/function>

namespace mpp {
    /**
     * The NodeJS like EventEmitter.
     */
    class event_emitter {
    private:
        std::unordered_map<std::string, std::list<std::shared_ptr<char>>> _event;

        template <typename Handler>
        struct FunctionParser : public FunctionParser<decltype(&Handler::operator())> {
        };

        template <typename ClassType, typename R, typename... Args>
        struct FunctionParser<R(ClassType::*)(Args...) const> {
            using FunctionType = mpp::function<R(Args...)>;
        };

        template <typename Handler>
        using FunctionType = typename FunctionParser<Handler>::FunctionType;

        template <typename Handler>
        FunctionType<Handler> makeFunction(Handler &cb) {
            return static_cast<FunctionType<Handler>>(cb);
        }

    public:
        event_emitter() = default;

        ~event_emitter() = default;

        /**
         * Register an event with handler.
         *
         * @tparam Handler Type of the handler
         * @param name Event name
         * @param handler Event handler
         */
        template <typename Handler>
        void on(const std::string &name, Handler handler) {
            using WrapperType = decltype(makeFunction(handler));

            auto *m = std::malloc(sizeof(WrapperType));
            if (m == nullptr) {
                // should panic oom
                return;
            }

            // generate the handler wrapper dynamically according to
            // the callback type, so we can pass varied and arbitrary
            // count of arguments to trigger the event handler.
            auto fn = new(m) WrapperType(makeFunction(handler));

            // use std::shared_ptr to manage the allocated memory
            // (char *) and (void *) are known as universal pointers.
            _event[name].push_back(std::shared_ptr<char>(
                // wrapper function itself
                reinterpret_cast<char *>(fn),

                // wrapper function deleter, responsible to call destructor
                [](char *ptr) {
                    if (ptr != nullptr) {
                        reinterpret_cast<WrapperType *>(ptr)->~WrapperType();
                        std::free(ptr);
                    }
                }
            ));
        }

        /**
         * Clear all handlers registered to event.
         *
         * @param name Event name
         */
        void unregister_event(const std::string &name) {
            auto it = _event.find(name);
            if (it != _event.end()) {
                _event.erase(it);
            }
        }

        /**
         * Call all event handlers associated with event name.
         *
         * @tparam Args Argument types
         * @param name Event name
         * @param args Event handler arguments
         */
        template <typename ...Args>
        void emit(const std::string &name, Args ...args) {
            auto it = _event.find(name);
            if (it != _event.end()) {
                auto &&callbacks = it->second;
                for (auto &&fn : callbacks) {
                    auto handler = reinterpret_cast<mpp::function<void(Args...)> *>(fn.get());
                    (*handler)(args...);
                }
            }
        }

        /**
         * Call all event handlers associated with event name.
         *
         * @param name Event name
         */
        void emit(const std::string &name) {
            auto it = _event.find(name);
            if (it != _event.end()) {
                auto &&callbacks = it->second;
                for (auto &&fn : callbacks) {
                    auto handler = reinterpret_cast<mpp::function<void(void)> *>(fn.get());
                    (*handler)();
                }
            }
        }
    };

}
