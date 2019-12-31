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
#include <mozart++/core/utility.hpp>
#include <mozart++/function>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace mpp {
    namespace event_emit_impl {
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
            expand_argument(data + 1, mpp::forward<ArgsT>(args)...);
        }
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
                auto data = new function_type(func);
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
                explicit event(const mpp::function<R(ArgsT...)> &func)
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
                on_impl(name, mpp::make_function(mpp::forward<T>(listener)));
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
                    expand_argument(arguments, mpp::forward<ArgsT>(args)...);
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
