/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/mozart>

mpp::byte_t *mpp::uninitialized_copy(byte_t *dest, byte_t *src, size_t count) noexcept
{
    return reinterpret_cast<byte_t *>(memcpy(reinterpret_cast<void *>(dest), reinterpret_cast<void *>(src), count));
}

std::chrono::time_point<std::chrono::high_resolution_clock> mpp::timer::m_timer(std::chrono::high_resolution_clock::now());
