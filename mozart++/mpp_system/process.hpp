/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */
#pragma once

#include <mozart++/core>
#include <mozart++/fdstream>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <vector>
#include <string>
#include <memory>

#ifdef MOZART_PLATFORM_WIN32

#include <Windows.h>

#else
#include <sys/stat.h>
#include <sys/wait.h>
#include <csignal>
#endif

namespace mpp_impl {
    using mpp::fd_type;
    using mpp::FD_INVALID;
    using mpp::close_fd;
    using mpp::close_pipe;
    using mpp::create_pipe;
    using mpp::PIPE_READ;
    using mpp::PIPE_WRITE;

    struct redirect_info {
        fd_type _target = FD_INVALID;

        bool redirected() const {
            return _target != FD_INVALID;
        }
    };

    struct process_startup {
        std::vector<std::string> _cmdline;
        std::unordered_map<std::string, std::string> _env;
        std::string _cwd = ".";
        redirect_info _stdin;
        redirect_info _stdout;
        redirect_info _stderr;
        bool merge_outputs = false;
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

#ifdef MOZART_PLATFORM_WIN32

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

        if (!SetHandleInformation(pstdin[PIPE_WRITE], HANDLE_FLAG_INHERIT, 0)) {
            mpp::throw_ex<mpp::runtime_error>("unable to set handle information on stdin");
        }

        si.hStdInput = pstdin[PIPE_READ];
        si.hStdOutput = pstdout[PIPE_WRITE];

        /*
         * pay special attention to stderr,
         * there are 2 cases:
         *      1. redirect stderr to stdout
         *      2. redirect stderr to a file
         */
        if (startup.merge_outputs) {
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

        char *envs = nullptr;

        if (!startup._env.empty()) {
            // starting from 1, which is the block terminator '\0'
            size_t env_size = 1;
            for (const auto &e : startup._env) {
                // need 2 more, which is the '=' and variable terminator '\0'
                env_size += e.first.length() + e.second.length() + 2;
            }

            envs = new char[env_size]{0};
            char *p = envs;

            for (const auto &e : startup._env) {
                strncat(p, e.first.c_str(), e.first.length());
                p += e.first.length();
                *p++ = '=';
                strncat(p, e.second.c_str(), e.second.length());
                p += e.second.length();
                *p++ = '\0'; // variable terminator
            }
            *p++ = '\0'; // block terminator

            // ensure envs are copied correctly
            if (p != envs + env_size) {
                delete[] envs;
                mpp::throw_ex<mpp::runtime_error>("unable to copy environment variables");
            }
        }

        if (!CreateProcess(nullptr, const_cast<char *>(command.c_str()),
                           nullptr, nullptr, true, CREATE_NO_WINDOW, envs,
                           startup._cwd.c_str(), &si, &pi)) {
            delete[] envs;
            mpp::throw_ex<mpp::runtime_error>("unable to fork subprocess");
        }

        delete[] envs;
        CloseHandle(pstdin[PIPE_READ]);
        CloseHandle(pstdout[PIPE_WRITE]);
        CloseHandle(pstderr[PIPE_WRITE]);

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
            if (startup.merge_outputs) {
                // redirect stderr to stdout
                dup2(pstdout[PIPE_WRITE], STDERR_FILENO);
            } else {
                // redirect stderr to a file
                if (!startup._stderr.redirected()) {
                    close_fd(pstderr[PIPE_READ]);
                }
                dup2(pstderr[PIPE_WRITE], STDERR_FILENO);
            }

            close_fd(pstdin[PIPE_READ]);
            close_fd(pstdout[PIPE_WRITE]);
            close_fd(pstderr[PIPE_WRITE]);

            // command-line and environments
            size_t asize = startup._cmdline.size();
            size_t esize = startup._env.size();
            char *argv[asize + 1];
            char *envp[esize + 1];

            // argv and envp are always terminated with a nullptr
            argv[asize] = nullptr;
            envp[esize] = nullptr;

            // copy command-line arguments
            for (std::size_t i = 0; i < asize; ++i) {
                argv[i] = const_cast<char *>(startup._cmdline[i].c_str());
            }

            // copy environment variables
            std::vector<std::string> envs;
            std::stringstream buffer;
            for (const auto &e : startup._env) {
                buffer.str("");
                buffer.clear();
                buffer << e.first << "=" << e.second;
                envs.emplace_back(buffer.str());
            }

            for (std::size_t i = 0; i < esize; ++i) {
                envp[i] = const_cast<char *>(envs[i].c_str());
            }

            // change cwd
            if (chdir(startup._cwd.c_str()) != 0) {
                mpp::throw_ex<mpp::runtime_error>("unable to change current working directory");
            }

            // run subprocess
            ::execve(argv[0], argv, envp);
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
            if (startup.merge_outputs) {
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
#ifdef MOZART_PLATFORM_WIN32
        create_process_win32(startup, info, pstdin, pstdout, pstderr);
#else
        create_process_unix(startup, info, pstdin, pstdout, pstderr);
#endif
    }

    bool redirect_or_pipe(const redirect_info &r, fd_type fds[2]) {
        if (!r.redirected()) {
            // no redirect target specified
            return create_pipe(fds);
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

        if (!startup.merge_outputs) {
            // if the user doesn't redirect stderr to stdout,
            // we bind stderr to a new file descriptor
            if (!redirect_or_pipe(startup._stderr, pstderr)) {
                close_pipe(pstdin);
                close_pipe(pstdout);
                mpp::throw_ex<mpp::runtime_error>("unable to bind stderr");
            }
        }

        try {
            create_process_impl(startup, info, pstdin, pstdout, pstderr);
        } catch (...) {
            // do rollback work
            // note: we should NOT close user provided redirect target fd,
            // let users to close.
            if (!startup._stdin.redirected()) {
                close_pipe(pstdin);
            }
            if (!startup._stdout.redirected()) {
                close_pipe(pstdout);
            }
            if (!startup._stderr.redirected()) {
                close_pipe(pstderr);
            }
            throw;
        }
    }

    void close_process(process_info &info) {
#ifdef MOZART_PLATFORM_WIN32
        mpp_impl::close_fd(info._pid);
        mpp_impl::close_fd(info._tid);
#endif
        mpp_impl::close_fd(info._stdin);
        mpp_impl::close_fd(info._stdout);
        mpp_impl::close_fd(info._stderr);
    }

    int wait_for(const process_info &info) {
#ifdef MOZART_PLATFORM_WIN32
        WaitForSingleObject(info._pid, INFINITE);
        DWORD code = 0;
        GetExitCodeProcess(info._pid, &code);
        return code;
#else
        int status;
        while (waitpid(info._pid, &status, 0) < 0) {
            switch (errno) {
                case ECHILD:
                    return 0;
                case EINTR:
                    break;
                default:
                    return -1;
            }
        }

        if (WIFEXITED(status)) {
            // The child exited normally, get its exit code
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            // The child exited because of a signal.
            // The best value to return is 0x80 + signal number,
            // because that is what all Unix shells do, and because
            // it allows callers to distinguish between process exit and
            // oricess death by signal.
            //
            // Breaking changes happens if we are running on solaris:
            // the historical behaviour on Solaris is to return the
            // original signal number, but we will ignore that!
            return 0x80 + WTERMSIG(status);
        } else {
            // unknown exit code, pass it through
            return status;
        }
#endif
    }

    void terminate_process(const process_info &info, bool force) {
#ifdef MOZART_PLATFORM_WIN32
        TerminateProcess(info._pid, 0);
#else
        kill(info._pid, force ? SIGKILL : SIGTERM);
#endif
    }

    bool process_exited(const process_info &info) {
#ifdef MOZART_PLATFORM_WIN32
        DWORD code = 0;
        GetExitCodeProcess(info._pid, &code);
        return code != STILL_ACTIVE;
#else
        // if WNOHANG was specified and one or more child(ren)
        // specified by pid exist, but have not yet changed state,
        // then 0 is returned. On error, -1 is returned.
        int result = waitpid(info._pid, nullptr, WNOHANG);

        if (result == -1) {
            if (errno != ECHILD) {
                // when WNOHANG was set, errno could only be ECHILD
                mpp::throw_ex<mpp::runtime_error>("should not reach here");
            }

            // waitpid() cannot find the child process identified by pid,
            // there are two cases of this situation depending on signal set
            struct sigaction sa{};
            if (sigaction(SIGCHLD, nullptr, &sa) != 0) {
                // only happens when kernel bug
                mpp::throw_ex<mpp::runtime_error>("should not reach here");
            }

#if defined(__APPLE__)
            void *handler = reinterpret_cast<void *>(sa.__sigaction_u.__sa_handler);
#elif defined(__linux__)
            void *handler = reinterpret_cast<void *>(sa.sa_handler);
#endif

            if (handler == reinterpret_cast<void *>(SIG_IGN)) {
                // in this situation we cannot check whether
                // a child process has exited in normal way, because
                // the child process is not belong to us any more, and
                // the kernel will move its owner to init without notifying us.
                // so we will try the fallback method.
                std::string path = std::string("/proc/") + std::to_string(info._pid);
                struct stat buf{};

                // when /proc/<pid> doesn't exist, the process has exited.
                // there will be race conditions: our process exited and
                // another process started with the same pid.
                // to eliminate this case, we should check /proc/<pid>/cmdline
                // but it's too complex and not always reliable.
                return stat(path.c_str(), &buf) == -1 && errno == ENOENT;

            } else {
                // we didn't set SIG_IGN for SIGCHLD
                // there is only one case here theoretically:
                // the child has exited too early before we checked it.
                return true;
            }
        }

        return result == 0;
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
        int _exit_code = -1;
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

        std::ostream &in() {
            return *_stdin;
        }

        std::istream &out() {
            return *_stdout;
        }

        std::istream &err() {
            return *_stderr;
        }

        int wait_for() {
            if (is_exited() && _exit_code >= 0) {
                return _exit_code;
            }
            _exit_code = mpp_impl::wait_for(_info);
            return _exit_code;
        }

        bool is_exited() const {
            return mpp_impl::process_exited(_info);
        }

        void interrupt(bool force = false) {
            mpp_impl::terminate_process(_info, force);
        }

    public:
        static process exec(const std::string &command);

        static process exec(const std::string &command,
                            const std::vector<std::string> &args);
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

        process_builder &environment(const std::string &key, const std::string &value) {
            _startup._env.emplace(key, value);
            return *this;
        }

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

#ifdef MOZART_PLATFORM_WIN32

        process_builder &redirect_stdin(int cfd) {
            return redirect_stdin(reinterpret_cast<fd_type>(_get_osfhandle(cfd)));
        }

        process_builder &redirect_stdout(int cfd) {
            return redirect_stdout(reinterpret_cast<fd_type>(_get_osfhandle(cfd)));
        }

        process_builder &redirect_stderr(int cfd) {
            return redirect_stderr(reinterpret_cast<fd_type>(_get_osfhandle(cfd)));
        }

#endif

        process_builder &directory(const std::string &cwd) {
            _startup._cwd = cwd;
            return *this;
        }

        process_builder &merge_outputs(bool r) {
            _startup.merge_outputs = r;
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

    process process::exec(const std::string &command,
                          const std::vector<std::string> &args) {
        return process_builder().command(command).arguments(args).start();
    }
}
