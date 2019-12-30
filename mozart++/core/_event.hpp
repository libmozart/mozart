/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <forward_list>
#include <mozart++/core/exception.hpp>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace mpp {
// Name Demangling
    std::string cxx_demangle(const char *);
    namespace event_emit_impl {
        /**
         * Integer Sequence
         * i.e. using sequence_type = typename make_sequence<Maximum>::result;
         */
        template <unsigned int... seq>
        struct sequence final {};
        template <unsigned int N, unsigned int... seq>
        struct make_sequence : public make_sequence<N - 1, N - 1, seq...> {};
        template <unsigned int... seq>
        struct make_sequence<0, seq...> {
            using result = sequence<seq...>;
        };
        /**
         * Convert template type packages into std::vector<std::type_index>
         * i.e. convert_typeinfo<ArgsT...>::convert(info_arr);
         */
        template <typename...>
        struct convert_typeinfo;
        template <>
        struct convert_typeinfo<> {
            static void convert(std::vector<std::type_index> &) {}
        };
        template <typename T, typename... ArgsT>
        struct convert_typeinfo<T, ArgsT...> {
            static void convert(std::vector<std::type_index> &arr)
            {
                arr.emplace_back(typeid(T));
                convert_typeinfo<ArgsT...>::convert(arr);
            }
        };
        /**
         * Check template type packages fit the type info
         * i.e. check_typeinfo<0, ArgsT...>::check(info_arr);
         * @throw mpp::runtime_error
         */
        template <unsigned int idx, typename... ArgsT>
        struct check_typeinfo;
        template <unsigned int idx>
        struct check_typeinfo<idx> {
            static void check(const std::vector<std::type_index> &) {}
        };
        template <unsigned int idx, typename T, typename... ArgsT>
        struct check_typeinfo<idx, T, ArgsT...> {
            static void check(const std::vector<std::type_index> &arr)
            {
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
        template <unsigned int N, typename T>
        typename std::remove_reference<T>::type &get_argument(void **data)
        {
            return *reinterpret_cast<typename std::remove_reference<T>::type *>(data[N]);
        }
        /**
         * Package the argument into raw data
         * @param data pointed to raw data
         * @param args arguments
         */
        inline void expand_argument(void **) {}
        template <typename T, typename... ArgsT>
        void expand_argument(void **data, T &&val, ArgsT &&... args)
        {
            *data = reinterpret_cast<void *>(&val);
            expand_argument(data + 1, std::forward<ArgsT>(args)...);
        }
        /**
         * Function Parser
         * parse_function: target function type
         * parse_function_return_t: target function return type
         */
        template <typename T>
        struct functor_parser : public functor_parser<decltype(&T::operator())> {};
        template <typename R, typename Class, typename... ArgsT>
        struct functor_parser<R (Class::*)(ArgsT...) const> {
            using return_type = R;
            using type = std::function<R(ArgsT...)>;
        };
        template <typename T>
        struct function_parser;
        template <typename R, typename... ArgsT>
        struct function_parser<R(ArgsT...)> {
            using return_type = R;
            using type = std::function<R(ArgsT...)>;
        };
        template <typename R, typename Class, typename... ArgsT>
        struct function_parser<R (Class::*)(ArgsT...)> {
            using return_type = R;
            using type = std::function<R(Class &, ArgsT...)>;
        };
        template <typename R, typename Class, typename... ArgsT>
        struct function_parser<R (Class::*)(ArgsT...) const> {
            using return_type = R;
            using type = std::function<R(const Class &, ArgsT...)>;
        };
        template <typename T>
        class is_functor {
            template <typename X, typename = decltype(&X::operator())>
            static constexpr bool test(int)
            {
                return true;
            }
            template <typename>
            static constexpr bool test(...)
            {
                return false;
            }

        public:
            static constexpr bool value = test<T>(0);
        };
        template <bool condition, typename R, template <typename> class T,
                  template <typename> class X>
        struct conditional_dispatcher;
        template <typename R, template <typename> class T, template <typename> class X>
        struct conditional_dispatcher<true, R, T, X> {
            using result = T<R>;
        };
        template <typename R, template <typename> class T, template <typename> class X>
        struct conditional_dispatcher<false, R, T, X> {
            using result = X<R>;
        };
        template <typename T>
        using callable_parser =
            typename conditional_dispatcher<is_functor<T>::value, T, functor_parser,
            function_parser>::result;
        template <typename T>
        using parse_function = typename callable_parser<typename std::remove_reference<
                               typename std::remove_const<T>::type>::type>::type;
        template <typename T>
        using parse_function_return_t =
            typename callable_parser<typename std::remove_reference<
            typename std::remove_const<T>::type>::type>::return_type;
        /**
         * Event Implementation
         * @param args listener argument types
         */
        template <typename R, typename... ArgsT>
        class event_impl {
            using function_type = mpp::function<R(ArgsT...)>;
            template <unsigned int... Seq>
            inline static void call_impl(void *func, void **data,
                                         const sequence<Seq...> &) noexcept
            {
                reinterpret_cast<function_type *>(func)->operator()(
                    get_argument<Seq, ArgsT>(data)...);
            }

        public:
            /**
             * using static function to replace virtual function to avoid unnecessary
             * vtable overhead
             */
            static void *create_event(const function_type &func)
            {
                function_type *data = new function_type(func);
                return reinterpret_cast<void *>(data);
            }
            static void destroy_event(void *data) noexcept
            {
                delete reinterpret_cast<function_type *>(data);
            }
            static void call_on(void *func, void **data) noexcept
            {
                static typename make_sequence<sizeof...(ArgsT)>::result seq;
                call_impl(func, data, seq);
            }
        };
        /**
         * Main Class
         */
        class event_emit {
            struct event {
                void *data;
                std::vector<std::type_index> types;
                void (*destroy_event)(void *) noexcept;
                void (*call_on)(void *, void **) noexcept;
                template <typename R, typename... ArgsT>
                event(const mpp::function<R(ArgsT...)> &func)
                {
                    using impl = event_impl<R, ArgsT...>;
                    data = impl::create_event(func);
                    convert_typeinfo<ArgsT...>::convert(types);
                    destroy_event = &impl::destroy_event;
                    call_on = &impl::call_on;
                }
                ~event()
                {
                    destroy_event(data);
                }
            };
            std::unordered_map<std::string, std::forward_list<event>> m_events;
            template <typename R, typename... ArgsT>
            void on_impl(const std::string &name,
                         const mpp::function<R(ArgsT...)> &func)
            {
                m_events[name].emplace_front(func);
            }

        public:
            event_emit() = default;
            virtual ~event_emit() = default;
            event_emit(const event_emit &) = delete;
            /**
             * Register an event with handler.
             * @tparam handler type of the handler
             * @param name event name
             * @param handler event handler
             */
            template <typename T>
            void on(const std::string &name, T &&listener)
            {
                /**
                 * Check through type traits
                 * Listener must be a function
                 */
                static_assert(!std::is_function<T>::value, "Event must be function");
                on_impl(name, parse_function<T>(std::forward<T>(listener)));
            }
            /**
             * Call all event handlers associated with event name.
             * @tparam args argument types
             * @param name event name
             * @param args event handler arguments
             */
            template <typename... ArgsT>
            void emit(const std::string &name, ArgsT &&... args)
            {
                static void *arguments[sizeof...(ArgsT)];
                if (m_events.count(name) > 0) {
                    auto &event = m_events.at(name);
                    expand_argument(arguments, std::forward<ArgsT>(args)...);
                    for (auto &it : event) {
                        if (sizeof...(ArgsT) != it.types.size())
                            throw_ex<mpp::runtime_error>("Wrong size of arguments.");
                        check_typeinfo<0, ArgsT...>::check(it.types);
                        it.call_on(it.data, arguments);
                    }
                }
            }
            /**
             * Clear all handlers registered to event.
             * @param name event name
             */
            void unregister_event(const std::string &name)
            {
                m_events.erase(name);
            }
        };
    }  // namespace event_emit_impl
    using event_emit_impl::event_emit;
}  // namespace mpp
