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
#include <memory>
#include <list>
#include <unordered_map>
#include <typeindex>

#include <mozart++/function>

namespace mpp {
    /**
     * The NodeJS-like type safe EventEmitter.
     */
    class event_emitter {
    private:
        /**
         * Inner container class, storing event handler
         */
        class handler_container {
        private:
            size_t _args_count = 0;
            std::type_index _args_info;
            std::shared_ptr<char> _handler;

        public:
            template <typename Handler>
            explicit handler_container(Handler &&handler)
                :_args_info(typeid(void))
            {
                // handler-dependent types
                using wrapper_type = decltype(make_function(handler));
                using arg_types = typename function_parser<wrapper_type>::decayed_arg_types;

                auto *m = std::malloc(sizeof(wrapper_type));
                if (m == nullptr) {
                    // should panic oom
                    return;
                }

                // generate the handler wrapper dynamically according to
                // the callback type, so we can pass varied and arbitrary
                // count of arguments to trigger the event handler.
                auto fn = new(m) wrapper_type(make_function(handler));

                // store argument info for call-time type check.
                _args_count = typelist::size<arg_types>::value;
                _args_info = typeid(arg_types);

                // use std::shared_ptr to manage the allocated memory
                // (char *) and (void *) are known as universal pointers.
                _handler = std::shared_ptr<char>(
                               // wrapper function itself
                               reinterpret_cast<char *>(fn),

                               // wrapper function deleter, responsible to call destructor
                [](char *ptr) {
                    if (ptr != nullptr) {
                        reinterpret_cast<wrapper_type *>(ptr)->~wrapper_type();
                        std::free(ptr);
                    }
                }
                           );
            }

            template <typename F>
            function_alias<F> *callable_ptr()
            {
                using callee_arg_types = typename function_parser<function_alias<F>>::decayed_arg_types;
                if (_args_info == typeid(callee_arg_types)) {
                    return reinterpret_cast<function_alias<F> *>(_handler.get());
                }
                return nullptr;
            }

            template <>
            function_alias<void()> *callable_ptr()
            {
                // Directly check argument count
                // because _args_info == typeid(typelist::nil) is much slower here.
                if (_args_count == 0) {
                    return reinterpret_cast<function_alias<void()> *>(_handler.get());
                }
                return nullptr;
            }
        };

    private:
        std::unordered_map<std::string, std::list<handler_container>> _events;

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
        void on(const std::string &name, Handler handler)
        {
            _events[name].push_back(handler_container(handler));
        }

        /**
         * Clear all handlers registered to event.
         *
         * @param name Event name
         */
        void unregister_event(const std::string &name)
        {
            auto it = _events.find(name);
            if (it != _events.end()) {
                _events.erase(it);
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
        void emit(const std::string &name, Args &&...args)
        {
            auto it = _events.find(name);
            if (it == _events.end()) {
                return;
            }

            for (auto &&fn : it->second) {
                auto handler = fn.callable_ptr<void(Args...)>();
                assert(handler && "Invalid call to event handler: mismatched argument list");
                (*handler)(std::forward<Args>(args)...);
            }
        }

        /**
         * Call all event handlers associated with event name.
         *
         * @param name Event name
         */
        void emit(const std::string &name)
        {
            auto it = _events.find(name);
            if (it == _events.end()) {
                return;
            }

            for (auto &&fn : it->second) {
                auto handler = fn.callable_ptr<void()>();
                assert(handler && "Invalid call to event handler: mismatched argument list");
                (*handler)();
            }
        }
    };
}
