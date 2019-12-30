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
    std::string cxx_demangle(const char *);
    namespace event_emit_impl {
        template <unsigned int... seq>
        struct sequence final {};
        template <unsigned int N, unsigned int... seq>
        struct make_sequence : public make_sequence<N - 1, N - 1, seq...> {};
        template <unsigned int... seq>
        struct make_sequence<0, seq...> {
            using result = sequence<seq...>;
        };
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
                    throw_ex<mpp::runtime_error>(std::string("Wrong argument. Expect \"") + cxx_demangle(arr[idx].name()) + "\", provided \"" + cxx_demangle(typeid(T).name()));
                else
                    check_typeinfo<idx + 1, ArgsT...>::check(arr);
            }
        };
        template <unsigned int N, unsigned int idx, typename T, typename... ArgsT>
        struct get_typename : public get_typename<N, idx + 1, ArgsT...> {};
        template <unsigned int N, typename T, typename... ArgsT>
        struct get_typename<N, N, T, ArgsT...> {
            using result = T;
        };
        template <unsigned int N, typename T>
        typename std::remove_reference<T>::type &get_argument(void **data)
        {
            return *reinterpret_cast<typename std::remove_reference<T>::type *>(data[N]);
        }
        inline void expand_argument(void **) {}
        template <typename T, typename... ArgsT>
        void expand_argument(void **data, T &&val, ArgsT &&... args)
        {
            *data = reinterpret_cast<void *>(&val);
            expand_argument(data, std::forward<ArgsT>(args)...);
        }
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
        class event_emit {
            class event_base {
            public:
                virtual ~event_base() = default;
                virtual const std::vector<std::type_index> &get_argument_types() const
                noexcept = 0;
                virtual bool call_on(void **) const noexcept = 0;
            };
            template <typename... ArgsT>
            class event_impl : public event_base {
                typename make_sequence<sizeof...(ArgsT)>::result m_seq;
                mpp::function<bool(ArgsT...)> m_func;
                std::vector<std::type_index> m_types;
                template <unsigned int... seq>
                inline bool call_impl(void **data, const sequence<seq...> &) const
                noexcept
                {
                    return m_func(
                               get_argument<seq, typename get_typename<seq, 0, ArgsT>::result>(
                                   data)...);
                }

            public:
                ~event_impl() override = default;
                template <typename T>
                event_impl(T &&func) : m_func(mpp::forward<T>(func))
                {
                    convert_typeinfo<ArgsT...>::convert(m_types);
                }
                const std::vector<std::type_index> &get_argument_types() const
                noexcept override
                {
                    return m_types;
                }
                bool call_on(void **data) const noexcept override
                {
                    return call_impl(data, m_seq);
                }
            };
            std::unordered_map<std::string,
                std::forward_list<std::unique_ptr<event_base>>>
                m_events;
            template <typename R, typename... ArgsT>
            void on_impl(const std::string &name,
                         const std::function<R(ArgsT...)> &func)
            {
                m_events[name].emplace_front(new event_impl<ArgsT...>(func));
            }

        public:
            template <typename T>
            void on(const std::string &name, T &&listener)
            {
                static_assert(!std::is_function<T>::value, "Event must be function");
                static_assert(std::is_same<bool, parse_function_return_t<T>>::value, "Function must returns bool");
                on_impl(name, parse_function<T>(std::forward<T>(listener)));
            }
            template <typename... ArgsT>
            void emit(const std::string &name, ArgsT &&... args)
            {
                static void *arguments[sizeof...(ArgsT)];
                if (m_events.count(name) == 0)
                    throw_ex<mpp::runtime_error>("Event not exist.");
                auto &event = m_events.at(name);
                expand_argument(arguments, std::forward<ArgsT>(args)...);
                for (auto &it : event) {
                    if (sizeof...(ArgsT) != it->get_argument_types().size())
                        throw_ex<mpp::runtime_error>("Wrong size of arguments.");
                    check_typeinfo<0, ArgsT...>::check(it->get_argument_types());
                    if (it->call_on(arguments)) break;
                }
            }
        };
    }  // namespace event_emit_impl
    using event_emit_impl::event_emit;
}  // namespace mpp
