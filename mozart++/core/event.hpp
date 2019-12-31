/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <forward_list>
#include <mozart++/function>
#include <mozart++/exception>
#include <mozart++/type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

/**
 * The NodeJS-like type safe EventEmitter.
 */
namespace mpp {
    /**
     * Fast Implementation(for release)
     */
    class event_emitter_fast {
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
            template<typename Handler>
            explicit handler_container(Handler &&handler)
                    :_args_info(typeid(void)) {
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

            template<typename F>
            function_alias<F> *callable_ptr() {
                using callee_arg_types = typename function_parser<function_alias<F>>::decayed_arg_types;
                if (_args_info == typeid(callee_arg_types)) {
                    return reinterpret_cast<function_alias<F> *>(_handler.get());
                }
                return nullptr;
            }

            template<>
            function_alias<void()> *callable_ptr() {
                // Directly check argument count
                // because _args_info == typeid(typelist::nil) is much slower here.
                if (_args_count == 0) {
                    return reinterpret_cast<function_alias<void()> *>(_handler.get());
                }
                return nullptr;
            }
        };

    private:
        std::unordered_map<std::string, std::vector<handler_container>> _events;

    public:
        event_emitter_fast() = default;

        ~event_emitter_fast() = default;

        /**
         * Register an event with handler.
         *
         * @tparam Handler Type of the handler
         * @param name Event name
         * @param handler Event handler
         */
        template<typename Handler>
        void on(const std::string &name, Handler handler) {
            _events[name].push_back(handler_container(handler));
        }

        /**
         * Clear all handlers registered to event.
         *
         * @param name Event name
         */
        void unregister_event(const std::string &name) {
            _events.erase(name);
        }

        /**
         * Call all event handlers associated with event name.
         *
         * @tparam Args Argument types
         * @param name Event name
         * @param args Event handler arguments
         */
        template<typename ...Args>
        void emit(const std::string &name, Args &&...args) {
            auto it = _events.find(name);
            if (it == _events.end()) {
                return;
            }

            for (auto &&fn : it->second) {
                auto handler = fn.callable_ptr<void(Args...)>();
                if (handler == nullptr)
                    throw_ex<mpp::runtime_error>("Invalid call to event handler: mismatched argument list");
                (*handler)(std::forward<Args>(args)...);
            }
        }
    };
    /**
     * Attentive Implementation(for debug)
     */
    namespace event_emitter_attentive_impl {
        /**
         * Convert template type packages into std::vector<std::type_index>
         * i.e. convert_typeinfo<ArgsT...>::convert(info_arr);
         */
        template<typename...>
        struct convert_typeinfo;

        template<>
        struct convert_typeinfo<> {
            static void convert(std::vector<std::type_index> &) {}
        };

        template<typename T, typename... ArgsT>
        struct convert_typeinfo<T, ArgsT...> {
            static void convert(std::vector<std::type_index> &arr) {
                arr.emplace_back(typeid(T));
                convert_typeinfo<ArgsT...>::convert(arr);
            }
        };

        /**
         * Check template type packages fit the type info
         * i.e. check_typeinfo<0, ArgsT...>::check(info_arr);
         * @throw mpp::runtime_error
         */
        template<unsigned int idx, typename... ArgsT>
        struct check_typeinfo;

        template<unsigned int idx>
        struct check_typeinfo<idx> {
            static void check(const std::vector<std::type_index> &) {}
        };

        template<unsigned int idx, typename T, typename... ArgsT>
        struct check_typeinfo<idx, T, ArgsT...> {
            static void check(const std::vector<std::type_index> &arr) {
                if (arr[idx] != typeid(T))
                    throw_ex<mpp::runtime_error>(std::string("Wrong argument. Expect \"") +
                                                 cxx_demangle(arr[idx].name()) +
                                                 "\", provided \"" +
                                                 cxx_demangle(typeid(T).name()));
                else
                    check_typeinfo<idx + 1, ArgsT...>::check(arr);
            }
        };

        /**
         * Instancing argument from raw data
         * @param data pointed to raw data
         * @return reference of instanced data
         */
        template<unsigned int N, typename T>
        typename std::remove_reference<T>::type &get_argument(void **data) {
            return *reinterpret_cast<typename std::remove_reference<T>::type *>(data[N]);
        }

        /**
         * Package the argument into raw data
         * @param data pointed to raw data
         * @param args arguments
         */
        inline void expand_argument(void **) {}

        template<typename T, typename... ArgsT>
        void expand_argument(void **data, T &&val, ArgsT &&... args) {
            *data = reinterpret_cast<void *>(&val);
            expand_argument(data + 1, mpp::forward<ArgsT>(args)...);
        }

        /**
         * Event Implementation
         * @param args listener argument types
         */
        template<typename R, typename... ArgsT>
        class event_impl {
            using function_type = mpp::function<R(ArgsT...)>;

            template<unsigned int... Seq>
            inline static void call_impl(void *func, void **data,
                                         const sequence<Seq...> &) noexcept {
                reinterpret_cast<function_type *>(func)->operator()(
                        get_argument<Seq, ArgsT>(data)...);
            }

        public:
            /**
             * using static function to replace virtual function to avoid unnecessary
             * vtable overhead
             */
            static void *create_event(const function_type &func) {
                auto data = new function_type(func);
                return reinterpret_cast<void *>(data);
            }

            static void destroy_event(void *data) noexcept {
                delete reinterpret_cast<function_type *>(data);
            }

            static void call_on(void *func, void **data) noexcept {
                static typename make_sequence<sizeof...(ArgsT)>::result seq;
                call_impl(func, data, seq);
            }
        };

        /**
         * Main Class
         */
        class event_emitter_attentive {
            struct event {
                void *data;
                std::vector<std::type_index> types;

                void (*destroy_event)(void *) noexcept;

                void (*call_on)(void *, void **) noexcept;

                template<typename R, typename... ArgsT>
                explicit event(const mpp::function<R(ArgsT...)> &func) {
                    using impl = event_impl<R, ArgsT...>;
                    data = impl::create_event(func);
                    convert_typeinfo<ArgsT...>::convert(types);
                    destroy_event = &impl::destroy_event;
                    call_on = &impl::call_on;
                }

                ~event() {
                    destroy_event(data);
                }
            };

            std::unordered_map<std::string, std::forward_list<event>> m_events;

            template<typename R, typename... ArgsT>
            void on_impl(const std::string &name,
                         const mpp::function<R(ArgsT...)> &func) {
                m_events[name].emplace_front(func);
            }

        public:
            event_emitter_attentive() = default;

            virtual ~event_emitter_attentive() = default;

            event_emitter_attentive(const event_emitter_attentive &) = delete;

            /**
             * Register an event with handler.
             * @tparam handler type of the handler
             * @param name event name
             * @param handler event handler
             */
            template<typename T>
            void on(const std::string &name, T &&listener) {
                // Check through type traits
                // Listener must be a function
                static_assert(!std::is_function<T>::value, "Event must be function");
                on_impl(name, mpp::make_function(mpp::forward<T>(listener)));
            }

            /**
             * Call all event handlers associated with event name.
             * @tparam args argument types
             * @param name event name
             * @param args event handler arguments
             */
            template<typename... ArgsT>
            void emit(const std::string &name, ArgsT &&... args) {
                if (m_events.count(name) > 0) {
                    auto &event = m_events.at(name);
                    void *arguments[sizeof...(ArgsT)];
                    expand_argument(arguments, mpp::forward<ArgsT>(args)...);
                    for (auto &it : event) {
                        if (sizeof...(ArgsT) != it.types.size())
                            throw_ex<mpp::runtime_error>("Invalid call to event handler: Wrong size of arguments.");
                        check_typeinfo<0, ArgsT...>::check(it.types);
                        it.call_on(it.data, arguments);
                    }
                }
            }

            /**
             * Clear all handlers registered to event.
             * @param name event name
             */
            void unregister_event(const std::string &name) {
                m_events.erase(name);
            }
        };
    }
    using event_emitter_attentive_impl::event_emitter_attentive;

    /**
     * Default Implementation
     * Release: Fast
     * Debug: Attentive
     */
#ifdef MOZART_DEBUG
    using event_emitter = event_emitter_attentive;
#else
    using event_emitter = event_emitter_fast;
#endif
}
