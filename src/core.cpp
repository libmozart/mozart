/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/core>

#ifdef MOZART_PLATFORM_WIN32

#include <shlobj.h>

#pragma comment(lib, "shell32.lib")

#endif

#ifdef _MSC_VER
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <windows.h>
#include <Dbghelp.h>
#pragma comment(lib, "DbgHelp")

namespace mpp {
    std::string cxx_demangle(const char* name)
    {
        char buffer[1024];
        DWORD length = UnDecorateSymbolName(name, buffer, sizeof(buffer), 0);
        if (length > 0)
            return std::string(buffer, length);
        else
            return name;
    }
}

#elif defined __GNUC__

#include <cxxabi.h>

namespace mpp {
    std::string cxx_demangle(const char *mangled) {
        char *ptr = abi::__cxa_demangle(mangled, nullptr, nullptr, nullptr);
        if (ptr) {
            auto s = std::string(ptr);
            std::free(ptr);
            return std::move(s);
        } else
            return mangled;
    }
}

#endif

namespace mpp {
    mpp::byte_t *uninitialized_copy(byte_t *dest, byte_t *src, size_t count) noexcept {
        return reinterpret_cast<byte_t *>(
                memcpy(reinterpret_cast<void *>(dest), reinterpret_cast<void *>(src), count));
    }

    namespace event {
        event_emitter core_event;
    }
}
