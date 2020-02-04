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

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace mpp_impl {
#ifdef _WIN32
    using fd_type = HANDLE;

    ssize_t read(fd_type handle, void *buf, size_t count) {
        DWORD dwRead;
        if (ReadFile(handle, buf, count, &dwRead, nullptr)) {
            return dwRead;
        } else {
            return 0;
        }
    }

    ssize_t write(fd_type handle, const void *buf, size_t count) {
        DWORD dwWritten;
        if (WriteFile(handle, buf, count, &dwWritten, nullptr)) {
            return dwWritten;
        } else {
            return 0;
        }
    }

#else
    using fd_type = int;
    using ::read;
    using ::write;
#endif
}

namespace mpp {
    class fdoutbuf : public std::streambuf {
    private:
        mpp_impl::fd_type _fd;

    public:
        explicit fdoutbuf(mpp_impl::fd_type fd)
            : _fd(fd) {
        }

    protected:
        int_type overflow(int_type c) override {
            if (c != EOF) {
                char z = c;
                if (mpp_impl::write(_fd, &z, 1) != 1) {
                    return EOF;
                }
            }
            return c;
        }

        std::streamsize xsputn(const char *s,
                               std::streamsize num) override {
            return mpp_impl::write(_fd, s, num);
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
        mpp_impl::fd_type _fd;

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
        explicit fdinbuf(mpp_impl::fd_type fd)
            : _fd(fd) {
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
            int num = mpp_impl::read(_fd, _buffer + PUTBACK_SIZE, BUFFER_SIZE);
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

