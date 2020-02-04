/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <mozart++/exception>
#include <mozart++/system/fdstream.hpp>

#ifdef _WIN32
#include <sstream>
#else
#include <sys/wait.h>
#endif

namespace mpp_impl {
    static constexpr int PIPE_READ = 0;
    static constexpr int PIPE_WRITE = 1;

    struct redirect_info {
        fd_type _target = FD_INVALID;

        bool redirected() const {
            return _target != FD_INVALID;
        }
    };

    struct process_startup {
        std::vector<std::string> _cmdline;
        redirect_info _stdin;
        redirect_info _stdout;
        redirect_info _stderr;
        bool _redirect_error = false;
    };

    struct process_info {
        /**
         * Unused on *nix systems.
         */
        fd_type _tid = FD_INVALID;

        fd_type _pid = FD_INVALID;
        fd_type _stdin = FD_INVALID;
        fd_type _stdout = FD_INVALID;
        fd_type _stderr = FD_INVALID;
    };

    void close_fd(fd_type &fd) {
        if (fd == FD_INVALID) {
            return;
        }
#ifdef _WIN32
        CloseHandle(fd);
#else
        ::close(fd);
#endif
        fd = FD_INVALID;
    }

    void close_pipe(fd_type fds[2]) {
        close_fd(fds[PIPE_READ]);
        close_fd(fds[PIPE_WRITE]);
    }

    bool create_pipe(fd_type fds[2]) {
#ifdef _WIN32
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = true;
        sa.lpSecurityDescriptor = nullptr;
        return CreatePipe(&fds[PIPE_READ], &fds[PIPE_WRITE], &sa, 0);
#else
        return ::pipe(fds) == 0;
#endif
    }

#ifdef _WIN32
    void create_process_win32(const process_startup &startup,
                              process_info &info,
                              fd_type *pstdin, fd_type *pstdout, fd_type *pstderr) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags |= STARTF_USESTDHANDLES;

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = true;
        sa.lpSecurityDescriptor = nullptr;
        si.hStdInput = pstdin[PIPE_READ];
        si.hStdOutput = pstdout[PIPE_WRITE];

        /*
         * pay special attention to stderr,
         * there are 2 cases:
         *      1. redirect stderr to stdout
         *      2. redirect stderr to a file
         */
        if (startup._redirect_error) {
            // redirect stderr to stdout
            si.hStdError = pstdout[PIPE_WRITE];
        } else {
            // redirect stderr to a file
            si.hStdError = pstderr[PIPE_WRITE];
        }

        ZeroMemory(&pi, sizeof(pi));

        std::stringstream ss;
        for (const auto &s : startup._cmdline) {
            ss << s << " ";
        }

        std::string command = ss.str();

        if (!CreateProcessA(nullptr, &command[0], nullptr, nullptr, false, 0,
                            nullptr, /* Current Directory in const char* */nullptr, &si, &pi)) {
            mpp::throw_ex<mpp::runtime_error>("unable to fork subprocess");
        }

        info._pid = pi.hProcess;
        info._tid = pi.hThread;
        info._stdin = pstdin[PIPE_WRITE];
        info._stdout = pstdout[PIPE_READ];
        info._stderr = pstderr[PIPE_READ];
    }
#else

    void create_process_unix(const process_startup &startup,
                             process_info &info,
                             fd_type *pstdin, fd_type *pstdout, fd_type *pstderr) {
        pid_t pid = fork();
        if (pid < 0) {
            close_pipe(pstdin);
            close_pipe(pstdout);
            close_pipe(pstderr);
            mpp::throw_ex<mpp::runtime_error>("unable to fork subprocess");

        } else if (pid == 0) {
            // in child process
            if (!startup._stdin.redirected()) {
                close_fd(pstdin[PIPE_WRITE]);
            }
            if (!startup._stdout.redirected()) {
                close_fd(pstdout[PIPE_READ]);
            }

            dup2(pstdin[PIPE_READ], STDIN_FILENO);
            dup2(pstdout[PIPE_WRITE], STDOUT_FILENO);

            /*
             * pay special attention to stderr,
             * there are 2 cases:
             *      1. redirect stderr to stdout
             *      2. redirect stderr to a file
             */
            if (startup._redirect_error) {
                // redirect stderr to stdout
                dup2(pstdout[PIPE_WRITE], STDOUT_FILENO);
            } else {
                // redirect stderr to a file
                if (!startup._stderr.redirected()) {
                    close_fd(pstderr[PIPE_READ]);
                }
                dup2(pstderr[PIPE_WRITE], STDERR_FILENO);
            }

            // copy command-line arguments
            size_t size = startup._cmdline.size();
            char *argv[size];
            for (std::size_t i = 0; i < size; ++i) {
                argv[i] = strdup(startup._cmdline[i].c_str());
            }

            // run subprocess
            ::execve(argv[0], argv, nullptr);

            // throw if failed to execve()
            for (std::size_t i = 0; i < size; ++i) {
                ::free(argv[i]);
            }
            mpp::throw_ex<mpp::runtime_error>("unable to exec commands in subprocess");

        } else {
            // in parent process
            if (!startup._stdin.redirected()) {
                close_fd(pstdin[PIPE_READ]);
            }
            if (!startup._stdout.redirected()) {
                close_fd(pstdout[PIPE_WRITE]);
            }

            /*
             * pay special attention to stderr,
             * there are 2 cases:
             *      1. redirect stderr to stdout
             *      2. redirect stderr to a file
             */
            if (startup._redirect_error) {
                // redirect stderr to stdout
                // do nothing
            } else {
                // redirect stderr to a file
                if (!startup._stderr.redirected()) {
                    close_fd(pstderr[PIPE_WRITE]);
                }
            }

            info._pid = pid;
            info._stdin = pstdin[PIPE_WRITE];
            info._stdout = pstdout[PIPE_READ];
            info._stderr = pstderr[PIPE_READ];

            // on *nix systems, fork() doesn't create threads to run process
            info._tid = FD_INVALID;
        }
    }

#endif

    void create_process_impl(const process_startup &startup,
                             process_info &info,
                             fd_type *pstdin, fd_type *pstdout, fd_type *pstderr) {
#ifdef _WIN32
        create_process_win32(startup, info, pstdin, pstdout, pstderr);
#else
        create_process_unix(startup, info, pstdin, pstdout, pstderr);
#endif
    }

    bool redirect_or_pipe(const redirect_info &r, fd_type fds[2]) {
        if (!r.redirected()) {
            // no redirect target specified
            return pipe(fds) == 0;
        }

        fds[PIPE_READ] = r._target;
        fds[PIPE_WRITE] = r._target;
        return true;
    }

    void create_process(const process_startup &startup,
                        process_info &info) {
        fd_type pstdin[2] = {FD_INVALID, FD_INVALID};
        fd_type pstdout[2] = {FD_INVALID, FD_INVALID};
        fd_type pstderr[2] = {FD_INVALID, FD_INVALID};

        if (!redirect_or_pipe(startup._stdin, pstdin)) {
            mpp::throw_ex<mpp::runtime_error>("unable to bind stdin");
        }

        if (!redirect_or_pipe(startup._stdout, pstdout)) {
            close_pipe(pstdin);
            mpp::throw_ex<mpp::runtime_error>("unable to bind stdout");
        }

        if (!startup._redirect_error) {
            // if the user doesn't redirect stderr to stdout,
            // we bind stderr to a new file descriptor
            if (!redirect_or_pipe(startup._stderr, pstderr)) {
                close_pipe(pstdin);
                close_pipe(pstdout);
                mpp::throw_ex<mpp::runtime_error>("unable to bind stderr");
            }
        }

        create_process_impl(startup, info, pstdin, pstdout, pstderr);
    }

    void close_process(process_info &info) {
#ifdef _WIN32
        mpp_impl::close_fd(info._pid);
        mpp_impl::close_fd(info._tid);
#endif
        mpp_impl::close_fd(info._stdin);
        mpp_impl::close_fd(info._stdout);
        mpp_impl::close_fd(info._stderr);
    }

    void wait_for(const process_info &info) {
#ifdef _WIN32
        WaitForSingleObject(info._pid, INFINITE);
#else
        int status;
        waitpid(info._pid, &status, 0);
#endif
    }
}

namespace mpp {
    using mpp_impl::redirect_info;
    using mpp_impl::process_info;
    using mpp_impl::process_startup;
    using mpp_impl::fd_type;

    class process {
        friend class process_builder;

    private:
        process_info _info;
        std::unique_ptr<fdostream> _stdin;
        std::unique_ptr<fdistream> _stdout;
        std::unique_ptr<fdistream> _stderr;

        explicit process(const process_info &info)
            : _info(info),
              _stdin(std::make_unique<fdostream>(_info._stdin)),
              _stdout(std::make_unique<fdistream>(_info._stdout)),
              _stderr(std::make_unique<fdistream>(_info._stderr)) {
        }

    public:
        process() = delete;

        process(const process &) = delete;

        process(process &&) = default;

        process &operator=(process &&) = delete;

        process &operator=(const process &) = delete;

    public:
        ~process() {
            mpp_impl::close_process(_info);
        }

        std::ostream &get_stdin() {
            return *_stdin;
        }

        std::istream &get_stdout() {
            return *_stdout;
        }

        std::istream &get_stderr() {
            return *_stderr;
        }

        void wait_for() const {
            mpp_impl::wait_for(_info);
        }

    public:
        static process exec(const std::string &command);
    };

    class process_builder {
    private:
        process_startup _startup;

    public:
        process_builder() = default;

        ~process_builder() = default;

        process_builder(process_builder &&) = default;

        process_builder(const process_builder &) = default;

        process_builder &operator=(process_builder &&) = default;

        process_builder &operator=(const process_builder &) = default;

    private:
        process_builder &redirect_stdin(fd_type target) {
            _startup._stdin._target = target;
            return *this;
        }

        process_builder &redirect_stdout(fd_type target) {
            _startup._stdout._target = target;
            return *this;
        }

        process_builder &redirect_stderr(fd_type target) {
            _startup._stderr._target = target;
            return *this;
        }

    public:
        process_builder &command(const std::string &command) {
            if (_startup._cmdline.empty()) {
                _startup._cmdline.push_back(command);
            } else {
                _startup._cmdline[0].assign(command);
            }
            return *this;
        }

        template <typename Container>
        process_builder &arguments(const Container &c) {
            if (_startup._cmdline.size() <= 1) {
                std::copy(c.begin(), c.end(), std::back_inserter(_startup._cmdline));
            } else {
                // invalid operation, do nothing
            }
            return *this;
        }

        process_builder &redirect_error() {
            _startup._redirect_error = true;
            return *this;
        }

        process start() {
            process_info info{};
            mpp_impl::create_process(_startup, info);
            return process(info);
        }
    };

    process process::exec(const std::string &command) {
        return process_builder().command(command).start();
    }
}
