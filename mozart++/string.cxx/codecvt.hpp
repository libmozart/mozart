/**
 * Mozart++ Template Library: String
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <mozart++/core>
#include <codecvt>
#include <cwctype>
#include <string>

namespace mpp {
    namespace codecvt {
        class charset {
        public:
            virtual ~charset() = default;

            virtual std::u32string local2wide(const std::string &) = 0;

            virtual std::string wide2local(const std::u32string &) = 0;

            virtual bool is_identifier(char32_t) = 0;
        };

        class ascii final : public charset {
        public:
            std::u32string local2wide(const std::string &local) override {
                return std::u32string(local.begin(), local.end());
            }

            std::string wide2local(const std::u32string &str) override {
                return std::string(str.begin(), str.end());
            }

            bool is_identifier(char32_t ch) override {
                return ch == '_' || std::iswalnum(ch);
            }
        };

        class utf8 final : public charset {
            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
        public:
            std::u32string local2wide(const std::string &str) override {
                return cvt.from_bytes(str);
            }

            std::string wide2local(const std::u32string &str) override {
                return cvt.to_bytes(str);
            }

            bool is_identifier(char32_t ch) override {
                if (ch >= 128)
                    return (ch >= 0x4E00 && ch <= 0x9FA5) || (ch >= 0x9FA6 && ch <= 0x9FEF) || ch == 0x3007;
                else
                    return ch == '_' || std::iswalnum(ch);
            }
        };

        class gbk final : public charset {
            inline char32_t set_zero(char32_t ch) {
                return ch & 0x0000ffff;
            }

        public:
            std::u32string local2wide(const std::string &local) override {
                std::u32string wide;
                uint32_t head = 0;
                int status = 0;
                for (auto it = local.begin(); it != local.end();) {
                    switch (status) {
                        case 0:
                            head = *(it++);
                            if (head & 0x80)
                                status = 1;
                            else
                                wide.push_back(set_zero(head));
                            break;
                        case 1: {
                            uint8_t tail = *(it++);
                            wide.push_back(set_zero(head << 8 | tail));
                            status = 0;
                            break;
                        }
                    }
                }
                if (status == 1)
                    throw_ex<runtime_error>("Codecvt: Bad encoding.");
                return std::move(wide);
            }

            std::string wide2local(const std::u32string &wide) override {
                std::string local;
                for (auto &ch:wide) {
                    if (ch & 0x8000)
                        local.push_back(ch >> 8);
                    local.push_back(ch);
                }
                return std::move(local);
            }

            bool is_identifier(char32_t ch) override {
                if (ch & 0x8000)
                    return (ch >= 0xB0A1 && ch <= 0xF7FE) || (ch >= 0x8140 && ch <= 0xA0FE) || ch == 0xA996;
                else
                    return ch == '_' || std::iswalnum(ch);
            }
        };
    }
}
