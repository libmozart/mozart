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
         * Unpack the template type packages
         * i.e. using type = get_typename<index, 0, ArgsT...>::result;
         */
        template <unsigned int N, unsigned int idx, typename T, typename... ArgsT>
        struct get_typename : public get_typename<N, idx + 1, ArgsT...> {};
        template <unsigned int N, typename T, typename... ArgsT>
        struct get_typename<N, N, T, ArgsT...> {
            using result = T;
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
            expand_argument(data, std::forward<ArgsT>(args)...);
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
        struct function_parser : public functor_parser<T> {};
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
        using parse_function =
            typename function_parser<typename std::remove_cv<T>::type>::type;
        template <typename T>
        using parse_function_return_t =
            typename function_parser<typename std::remove_cv<T>::type>::return_type;
        /**
         * Event Implementation
         * @param args listener argument types
         */
        template <typename... ArgsT>
        class event_impl {
            using function_type = mpp::function<bool(ArgsT...)>;
            template <unsigned int... seq>
            inline static bool call_impl(void *func, void **data,
                                         const sequence<seq...> &) noexcept
            {
                return reinterpret_cast<function_type *>(func)->operator()(
                           get_argument<seq, typename get_typename<seq, 0, ArgsT...>::result>(
                               data)...);
            }

        public:
            /**
             * using static function to replace virtual function to avoid unnecessary vtable overhead
             */
            template <typename T>
            static void *create_event(T &&func)
            {
                function_type *data = new function_type(std::forward<T>(func));
                return reinterpret_cast<void *>(data);
            }
            static void destroy_event(void *data) noexcept
            {
                delete reinterpret_cast<function_type *>(data);
            }
            static bool call_on(void *func, void **data) noexcept
            {
                static typename make_sequence<sizeof...(ArgsT)>::result seq;
                return call_impl(func, data, seq);
            }
        };
        /**
         * Main Class
         */
        class event_emit {
            template <typename...>
            struct argument_carrier {};
            struct event {
                void *data;
                std::vector<std::type_index> types;
                void (*destroy_event)(void *) noexcept;
                bool (*call_on)(void *, void **) noexcept;
                template <typename T, typename... ArgsT>
                event(T &&func, argument_carrier<ArgsT...>)
                {
                    using impl = event_impl<ArgsT...>;
                    data = impl::create_event(std::forward<T>(func));
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
                         const std::function<R(ArgsT...)> &func)
            {
                m_events[name].emplace_front(func, argument_carrier<ArgsT...>());
            }

        public:
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
                 * 1. Listener must be a function
                 * 2. Listener must returns bool, true represents process finish
                 */
                static_assert(!std::is_function<T>::value, "Event must be function");
                static_assert(std::is_same<bool, parse_function_return_t<T>>::value,
                              "Function must returns bool");
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
                if (m_events.count(name) == 0)
                    throw_ex<mpp::runtime_error>("Event not exist.");
                auto &event = m_events.at(name);
                expand_argument(arguments, std::forward<ArgsT>(args)...);
                for (auto &it : event) {
                    if (sizeof...(ArgsT) != it.types.size())
                        throw_ex<mpp::runtime_error>("Wrong size of arguments.");
                    check_typeinfo<0, ArgsT...>::check(it.types);
                    if (it.call_on(it.data, arguments)) break;
                }
            }
        };
    }  // namespace event_emit_impl
    using event_emit_impl::event_emit;
}  // namespace mpp
