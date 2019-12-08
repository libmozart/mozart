/**
 * Mozart++ Template Library: Utility
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
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
#else
    #warning Can not detect C++ standard version with MSVC Compiler automatically, please using C++14 or newer standard
#endif

/**
 * Mozart++ Version: 19.12.1
 */

#define __Mozart 191201L

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
 * ::mozart
 *      Standard Mozart++ Namespace
 * ::mozart_impl
 *      Mozart++ Implement Namespace
 */

#include <cstdint>
#include <cstddef>

namespace mozart
{
// Path seperator and delimiter
#ifdef MOZART_PLATFORM_WIN32
	constexpr char path_separator = '\\';
	constexpr char path_delimiter = ';';
#else
	constexpr char path_separator = '/';
	constexpr char path_delimiter = ':';
#endif
    using byte_t = std::uint8_t;
    using size_t = std::size_t;
}

namespace mozart_impl
{
    // Not implemented yet
}