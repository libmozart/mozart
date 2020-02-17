/**
 * Mozart++ Template Library: Memory
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <mozart++/core>
#include <memory>

namespace mpp {
    using std::allocator;

    template <typename T, size_t blck_size, template <typename> class allocator_t = allocator>
    class allocator_type final {
        T *mPool[blck_size];
        allocator_t<T> mAlloc;
        size_t mOffset = 0;

    public:
        allocator_type() {
            while (mOffset < 0.5 * blck_size)
                mPool[mOffset++] = mAlloc.allocate(1);
        }

        allocator_type(const allocator_type &) = delete;

        ~allocator_type() {
            while (mOffset > 0)
                mAlloc.deallocate(mPool[--mOffset], 1);
        }

        template <typename... ArgsT>
        inline T *alloc(ArgsT &&... args) {
            T *ptr = nullptr;
            if (mOffset > 0)
                ptr = mPool[--mOffset];
            else
                ptr = mAlloc.allocate(1);
            mAlloc.construct(ptr, std::forward<ArgsT>(args)...);
            return ptr;
        }

        inline void free(T *ptr) {
            mAlloc.destroy(ptr);
            if (mOffset < blck_size)
                mPool[mOffset++] = ptr;
            else
                mAlloc.deallocate(ptr, 1);
        }
    };

    template <typename T, size_t blck_size, template <typename> class allocator_t = allocator>
    class plain_allocator_type final {
        allocator_t<T> mAlloc;

    public:
        plain_allocator_type() = default;

        plain_allocator_type(const plain_allocator_type &) = delete;

        ~plain_allocator_type() = default;

        template <typename... ArgsT>
        inline T *alloc(ArgsT &&... args) {
            T *ptr = mAlloc.allocate(1);
            mAlloc.construct(ptr, forward<ArgsT>(args)...);
            return ptr;
        }

        inline void free(T *ptr) {
            mAlloc.destroy(ptr);
            mAlloc.deallocate(ptr, 1);
        }
    };
}
