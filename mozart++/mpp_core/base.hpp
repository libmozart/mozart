/**
 * Mozart++ Template Library: Core Library/Foundation Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#ifndef __cplusplus
#error Please use cplusplus compiler such as g++ or clang++
#endif

/**
 * Value of __cplusplus with different standard version:
 * 199711L (C++98 or C++03)
 * 201103L (C++11)
 * 201402L (C++14)
 * 201703L (C++17)
 * Reference from https://stackoverflow.com/questions/26089319/is-there-a-standard-definition-for-cplusplus-in-c14
 */

#ifndef _MSC_VER
#if __cplusplus < 201402L
#error Please use newer cplusplus compiler fully supports C++14 standard
#endif
#endif

/**
 * Mozart++ Version: 20.2.1
 */

#define __Mozart 200201L

/**
 * Platform detection
 */

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define MOZART_PLATFORM_WIN32
#define MOZART_PLATFORM_NAME "MS Win32"
#endif

#if defined(__linux) || defined(__linux__) || defined(linux)
#define MOZART_PLATFORM_LINUX
#define MOZART_PLATFORM_NAME "GNU Linux"
#endif

#if defined(__APPLE__) || defined(__MACH__)
#define MOZART_PLATFORM_DARWIN
#define MOZART_PLATFORM_NAME "Apple Darwin"
#endif

#if defined(MOZART_PLATFORM_LINUX) || defined(MOZART_PLATFORM_DARWIN)
#define MOZART_PLATFORM_UNIX
#endif

/**
 * Namespace
 * ::mpp
 *      Standard Mozart++ Namespace
 * ::mpp_impl
 *      Mozart++ Implement Namespace
 */

#include <type_traits>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <string>

/**
 * Mozart++ Log System
 *
 * use MOZART_LOGEV(message) to log a normal event
 * use MOZART_LOGCR(message) to log a critical event
 *
 * define MOZART_DEBUG to enable the log
 * define MOZART_LOGCR_ONLY to disable normal events
 *
 * All macros must be defined before include
 */

#ifdef MOZART_DEBUG

#include <cstdio>

#ifndef MOZART_LOGCR_ONLY
// Event Log
#define MOZART_LOGEV(msg) ::printf("EV[%s] In file %s:%d: %s\n", __TIME__, __FILE__, __LINE__, msg);

#else

#define MOZART_LOGEV(msg)

#endif
// Critical Event Log
#define MOZART_LOGCR(msg) ::printf("CR[%s] In file %s:%d: %s\n", __TIME__, __FILE__, __LINE__, msg);

#else

#define MOZART_LOGEV(msg)
#define MOZART_LOGCR(msg)

#endif

namespace mpp {
    /**
     * Path seperator and delimiter
     */
#ifdef MOZART_PLATFORM_WIN32
    constexpr char path_separator = '\\';
    constexpr char path_delimiter = ';';
#else
    constexpr char path_separator = '/';
    constexpr char path_delimiter = ':';
#endif

    using byte_t = std::uint8_t;

    /* Import basic types and functions */
    using std::size_t;
    using std::declval;
    using std::forward;
    using std::memcpy;
    using std::move;
    using std::swap;

    /**
     * Alignment
     */
    template <typename type>
    using aligned_type = std::aligned_storage_t<sizeof(type), std::alignment_of<type>::value>;

    template <size_t len, typename ...types>
    using aligned_union = std::aligned_union_t<len, types...>;

    inline byte_t *uninitialized_copy(byte_t *dest, byte_t *src, size_t count) noexcept;

    template <typename T, typename ...Args>
    inline static void construct_at(byte_t *ptr, Args &&...args) {
        ::new(ptr) T(forward<Args>(args)...);
    }

    template <typename T>
    inline static void destroy_at(byte_t *ptr) {
        reinterpret_cast<T *>(ptr)->~T();
    }

    class object {
    public:
        constexpr object() = default;

        object(const object &) = default;

        object(object &&) noexcept = default;

        virtual ~object() = default;

        object &operator=(const object &) = default;

        object &operator=(object &&) = default;
    };

    class nocopyable {
    public:
        nocopyable() = default;

        nocopyable(const nocopyable &) = delete;

        nocopyable(nocopyable &&) noexcept = default;

        ~nocopyable() = default;

        nocopyable &operator=(const nocopyable &) = delete;

        nocopyable &operator=(nocopyable &&) = default;
    };

    class nomovable {
    public:
        nomovable() = default;

        nomovable(const nomovable &) = default;

        nomovable(nomovable &&) noexcept = delete;

        ~nomovable() = default;

        nomovable &operator=(const nomovable &) = default;

        nomovable &operator=(nomovable &&) = delete;
    };

    class singleton : public object, public nocopyable, public nomovable {
    protected:
        singleton() = default;
    };

    /**
     * Name demangle.
     *
     * @param mangled mangled type name.
     * @return Human-readable type name.
     */
    std::string cxx_demangle(const char *mangled);
}

namespace mpp_impl {
    // Not implemented yet
}
