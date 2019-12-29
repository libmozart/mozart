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
#include <forward_list>
#include <typeindex>

#include <mozart++/function>

namespace mpp {
    /**
     * The NodeJS like EventEmitter.
     */
    class event_emitter {
    private:
        std::unordered_map<std::string, std::list<std::shared_ptr<char>>> _event;

        template<typename Handler>
        function_type<Handler> make_wrapper(Handler &cb) {
            return static_cast<function_type<Handler>>(cb);
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
        template<typename Handler>
        void on(const std::string &name, Handler handler) {
            using wrapper_type = decltype(make_wrapper(handler));

            auto *m = std::malloc(sizeof(wrapper_type));
            if (m == nullptr) {
                // should panic oom
                return;
            }

            // generate the handler wrapper dynamically according to
            // the callback type, so we can pass varied and arbitrary
            // count of arguments to trigger the event handler.
            auto fn = new(m) wrapper_type(make_wrapper(handler));

            // use std::shared_ptr to manage the allocated memory
            // (char *) and (void *) are known as universal pointers.
            _event[name].push_back(std::shared_ptr<char>(
                    // wrapper function itself
                    reinterpret_cast<char *>(fn),

                    // wrapper function deleter, responsible to call destructor
                    [](char *ptr) {
                        if (ptr != nullptr) {
                            reinterpret_cast<wrapper_type *>(ptr)->~wrapper_type();
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
        template<typename ...Args>
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

    template<unsigned int...seq> struct sequence final {};
    template<unsigned int N, unsigned int...seq>struct make_sequence:public make_sequence<N - 1, N - 1, seq...> {};
    template<unsigned int...seq>struct make_sequence<0, seq...> {
        using result = sequence<seq...>;
    };
    template<typename...ArgsT> void convert_typeinfo(std::vector<std::type_index> &arr);
    template<> void convert_typeinfo<>(std::vector<std::type_index> &) {}
    template<typename T, typename...ArgsT> void convert_typeinfo<T, ArgsT...>(std::vector<std::type_index> &arr)
    {
        arr.emplace_back(typeid(T));
        convert_typeinfo<ArgsT...>(arr);
    }
    template<typename T> void convert_typeinfo<T>(std::vector<std::type_index> &arr)
    {
        arr.emplace_back(typeid(T));
    }
    template<unsigned int idx, typename...ArgsT> void check_typeinfo(const std::vector<std::type_index> &);
    template<unsigned int idx> void check_typeinfo<idx>(const std::vector<std::type_index> &) {}
    template<unsigned int idx, typename T, typename...ArgsT> void check_typeinfo<idx, T, ArgsT...>(const std::vector<std::type_index> &arr)
    {
        if (typeid(T) != arr[N])
            throw_ex<mpp:runtime_error>("Wrong argument.");
        else
            check_typeinfo<idx + 1, ArgsT...>(arr);
    }
    template<unsigned int N, unsigned int idx, typename T, typename...ArgsT>struct get_typename:public get_typename<N, idx + 1, ArgsT...> {};
    template<unsigned int N, typename T, typename...ArgsT>struct get_typename<N, N, T, ArgsT...>
    {
        using result = T;
    };
    template<unsigned int N, typename T> typename std::remove_reference<T>::type& get_argument(void **data)
    {
        return *reinterpret_cast<typename std::remove_reference<T>::type *>(*data[N]);
    }
    template<typename...ArgsT> void expand_argument(void **, ArgsT&&...);
    template<> void expand_argument<>(void **) {}
    template<typename T, typename...ArgsT> void expand_argument<T, ArgsT...>(void **data, T&& val, ArgsT&&...args)
    {
        data[idx] = reinterpret_cast<void*>(&val);
        expand_argument(data + 1, std::forward<ArgsT>(args)...);
    }
    template<typename T>struct functor_parser:public functor_parser<decltype(&T::operator())> {};
    template<typename R, typename Class, typename...ArgsT> struct functor_parser<R(Class::*)(ArgsT...) const> {
        using type = std::function<R(ArgsT...)>;
    };
    template<typename T>struct function_parser:public functor_parser<T> {};
    template<typename R, typename...ArgsT> struct function_parser<R(ArgsT...)> {
        using type = std::function<R(ArgsT...)>;
    };
    template<typename R, typename Class, typename...ArgsT> struct function_parser<R(Class::*)(ArgsT...)> {
        using type = std::function<R(Class&, ArgsT...)>;
    };
    template<typename R, typename Class, typename...ArgsT> struct function_parser<R(Class::*)(ArgsT...) const> {
        using type = std::function<R(const Class&, ArgsT...)>;
    };
    template<typename T>using parse_function = typename function_parser<typename std::remove_cv<T>::type>::type;
    class event_emit
    {
        class listener_base
        {
        public:
            virtual const std::vector<std::type_index> &
                     get_argument_types() const noexcept = 0;
            virtual bool call_on(void **) const noexcept = 0;
        };
        template<typename...ArgsT>
        class listener_impl:public listener_base
        {
            make_sequence<sizeof...(ArgsT)>::result m_seq;
            mpp::function<bool(ArgsT...)> m_func;
            std::vector<std::type_index> m_types;
            template<unsigned int...seq>
            inline bool call_impl(void **data, const sequence<seq...> &)
            {
                return m_func(get_argument<seq, typename get_typename<seq, 0, ArgsT>::result>(data)...);
            }
        public:
            template<typename T>
            listener_impl(T&& func) : m_func(mpp::forward<T>(func)) {
                convert_typeinfo<ArgsT...>(m_types);
            }
            const std::vector<std::type_index>&
                 get_argument_types() const noexcept override
            {
                return m_types;
            }
            bool call_on(void **data) const noexcept override
            {
                return call_impl(data, m_seq);
            }
        };
        std::unordered_map<std::string, std::forward_list<std::unique_ptr<listener_base *>>> m_events;
        template<typename R, typename...ArgsT>void on_impl(const std::string &name, const std::function<R(ArgsT...)> &func)
        {
            m_events[name].emplace_front(new listener_impl<ArgsT...>(func));
        }
    public:
        template<typename T>
        void on(const std::string &name, T&& listener)
        {
            on_impl(name, parse_function<T>(std::forward<T>(listener)));
        }
        template<typename...ArgsT> void emit(const std::string &name, ArgsT&&...args)
        {
            if (m_events.count(name) == 0)
                throw_ex<mpp::runtime_error>("Event not exist.");
            auto &event = m_events.at(name);
            void* arguments[sizeof...(ArgsT)];
            expand_argument(arguments, std::forward<ArgsT>(args)...);
            for (auto &it:event)
            {
                if (it->call_on(arguments))
                    break;
            }
        }
    };
}
