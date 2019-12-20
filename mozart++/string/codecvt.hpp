/**
 * Mozart++ Template Library: String
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <mozart++/core/base.hpp>
#include <codecvt>
#include <string>
#include <locale>

namespace mpp {
    class codecvt final {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    public:
        static std::wstring to_wstr(const std::string &u8str) {
            return mpp::move(conv.from_bytes(u8str));
        }

        static std::string to_u8str(const std::wstring &wstr) {
            return mpp::move(conv.to_bytes(wstr));
        }
    };
}
