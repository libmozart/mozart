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
#include <locale>
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

            static constexpr std::uint32_t ascii_max = 0x7F;
        public:
            std::u32string local2wide(const std::string &str) override {
                return cvt.from_bytes(str);
            }

            std::string wide2local(const std::u32string &str) override {
                return cvt.to_bytes(str);
            }

            bool is_identifier(char32_t ch) override {
                /**
                 * Chinese Character in Unicode Charset
                 * Basic:    0x4E00 - 0x9FA5
                 * Extended: 0x9FA6 - 0x9FEF
                 * Special:  0x3007
                 */
                if (ch > ascii_max)
                    return (ch >= 0x4E00 && ch <= 0x9FA5) || (ch >= 0x9FA6 && ch <= 0x9FEF) || ch == 0x3007;
                else
                    return ch == '_' || std::iswalnum(ch);
            }
        };

        class gbk final : public charset {
            inline char32_t set_zero(char32_t ch) {
                return ch & 0x0000ffff;
            }

            static constexpr std::uint8_t u8_blck_begin = 0x80;
            static constexpr std::uint32_t u32_blck_begin = 0x8000;

        public:
            std::u32string local2wide(const std::string &local) override {
                std::u32string wide;
                std::uint32_t head = 0;
                bool read_next = true;
                for (auto it = local.begin(); it != local.end();) {
                    if (read_next) {
                        head = *(it++);
                        if (head & u8_blck_begin)
                            read_next = false;
                        else
                            wide.push_back(set_zero(head));
                    } else {
                        std::uint8_t tail = *(it++);
                        wide.push_back(set_zero(head << 8 | tail));
                        read_next = true;
                    }
                }
                if (!read_next)
                    throw_ex<mpp::runtime_error>("Codecvt: Bad encoding.");
                return std::move(wide);
            }

            std::string wide2local(const std::u32string &wide) override {
                std::string local;
                for (auto &ch:wide) {
                    if (ch & u32_blck_begin)
                        local.push_back(ch >> 8);
                    local.push_back(ch);
                }
                return std::move(local);
            }

            bool is_identifier(char32_t ch) override {
                /**
                 * Chinese Character in GBK Charset
                 * GBK/2: 0xB0A1 - 0xF7FE
                 * GBK/3: 0x8140 - 0xA0FE
                 * GBK/4: 0xAA40 - 0xFEA0
                 * GBK/5: 0xA996
                 */
                if (ch & u32_blck_begin)
                    return (ch >= 0xB0A1 && ch <= 0xF7FE) || (ch >= 0x8140 && ch <= 0xA0FE) ||
                           (ch >= 0xAA40 && ch <= 0xFEA0) || ch == 0xA996;
                else
                    return ch == '_' || std::iswalnum(ch);
            }
        };
    }
}
