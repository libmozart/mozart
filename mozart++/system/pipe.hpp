//
// Created by kiva on 2020/2/8.
//
#pragma once

#include <mozart++/system/file.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace mpp {
    static constexpr int PIPE_READ = 0;
    static constexpr int PIPE_WRITE = 1;

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
}
