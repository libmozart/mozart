/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */
#pragma once

#include <istream>
#include <ostream>
#include <streambuf>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

namespace mpp {
    class fdoutbuf : public std::streambuf {
    private:
        int _fd;

    public:
        explicit fdoutbuf(int _fd)
            : _fd(_fd) {
        }

    protected:
        int_type overflow(int_type c) override {
            if (c != EOF) {
                char z = c;
                if (::write(_fd, &z, 1) != 1) {
                    return EOF;
                }
            }
            return c;
        }

        std::streamsize xsputn(const char *s,
                               std::streamsize num) override {
            return ::write(_fd, s, num);
        }
    };

    class fdostream : public std::ostream {
    private:
        fdoutbuf _buf;
    public:
        explicit fdostream(int fd)
            : std::ostream(nullptr), _buf(fd) {
            rdbuf(&_buf);
        }
    };

    class fdinbuf : public std::streambuf {
    private:
        int _fd;

    protected:
        /**
         * size of putback area
         */
        static constexpr size_t PUTBACK_SIZE = 4;

        /**
         * size of the data buffer
         */
        static constexpr size_t BUFFER_SIZE = 1024;

        char _buffer[BUFFER_SIZE + PUTBACK_SIZE]{0};

    public:
        explicit fdinbuf(int _fd)
            : _fd(_fd) {
            setg(_buffer + PUTBACK_SIZE,     // beginning of putback area
                _buffer + PUTBACK_SIZE,     // read position
                _buffer + PUTBACK_SIZE);    // end position
        }

    protected:
        // insert new characters into the buffer
        int_type underflow() override {
            // is read position before end of buffer?
            if (gptr() < egptr()) {
                return traits_type::to_int_type(*gptr());
            }

            // handle putback area
            size_t backSize = gptr() - eback();
            if (backSize > PUTBACK_SIZE) {
                backSize = PUTBACK_SIZE;
            }

            // copy up to PUTBACK_SIZE characters previously read into
            // the putback area
            std::memmove(_buffer + (PUTBACK_SIZE - backSize),
                gptr() - backSize,
                backSize);

            // read at most BUFFER_SIZE new characters
            int num = read(_fd, _buffer + PUTBACK_SIZE, BUFFER_SIZE);
            if (num <= 0) {
                // it might be error happened somewhere or EOF encountered
                // we simply return EOF
                return EOF;
            }

            // reset buffer pointers
            setg(_buffer + (PUTBACK_SIZE - backSize),
                _buffer + PUTBACK_SIZE,
                _buffer + PUTBACK_SIZE + num);

            // return next character
            return traits_type::to_int_type(*gptr());
        }
    };

    class fdistream : public std::istream {
    private:
        fdinbuf _buf;
    public:
        explicit fdistream(int fd)
            : std::istream(nullptr), _buf(fd) {
            rdbuf(&_buf);
        }
    };
}

