/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */
#pragma once

#include <mozart++/system/file.hpp>

#ifdef MOZART_PLATFORM_WIN32
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
#ifdef MOZART_PLATFORM_WIN32
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
