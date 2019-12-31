/**
 * Mozart++ Template Library: Any
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <mozart++/core/base.hpp>
#include <mozart++/core/exception.hpp>
#include <mozart++/core/memory.hpp>
#include <typeindex>

namespace mpp {
    class any;
}

class mpp::any final {
public:
    using typeid_t = std::type_index;
    /**
     * Buffer pool size, too large value may be appropriate for the reverse
     */
    static constexpr size_t default_allocate_buffer_size = 16;
    /**
     * Allocator provider, which uses the Mozart++ allocator by default, can be replaced by a memory pool as needed
     * Tip: the Any of this framework USES the Small Data Optimize technique to significantly reduce the heap load
     * Changing the memory pool may not improve performance much
     *
     * @tparam DataType
     */
    template<typename T>
    using default_allocator_provider = mpp::allocator<T>;
    /**
     * Unified definition
     */
    template<typename T>
    using default_allocator = allocator_type<T, default_allocate_buffer_size, default_allocator_provider>;

private:
    /**
     * Data storage base class
     * The key to using polymorphism for type erasers is to abstract out type-independent interfaces
     * This class is an interface class, or a pure virtual base class
     */
    class stor_base {
    public:
        /**
         * Default constructor, directly using the default version
         */
        stor_base() = default;

        /**
         * Copy the constructor, using the default version directly
         */
        stor_base(const stor_base &) = default;

        /**
         * Destructor, declared as a virtual function and implemented using default
         */
        virtual ~stor_base() = default;

        /**
         * RTTI type function
         *
         * @return std::type_index
         */
        virtual std::type_index type() const noexcept = 0;

        /**
         * Suicide function, freeing up resources
         *
         * @param isEnabled is Small Data Optimize enabled
         */
        virtual void suicide(bool) = 0;

        /**
         * The clone function constructs a clone of the current object at the specified address
         *
         * @param TargetAddress
         */
        virtual void clone(byte_t *) const = 0;

        /**
         * The clone function constructs a clone of the current object and returns it
         *
         * @return pointer of new object
         */
        virtual stor_base *clone() const = 0;
    };

    /**
     * Data store template derived classes
     * Concrete implementation of storing data
     * This class takes advantage of the properties of the template class to automatically generate the desired derived classes
     *
     * @tparam DataType
     */
    template<typename T>
    class stor_impl : public stor_base {
    public:
        /**
         * The actual stored data
         */
        T data;

        /**
         * Static Allocator
         */
        static default_allocator<stor_impl<T>> allocator;

        /**
         * Default constructor, implemented using default
         */
        stor_impl() = default;

        /**
         * Destructor, implemented using default
         */
        virtual ~stor_impl() = default;

        /**
         * Disable the copy constructor
         */
        stor_impl(const stor_impl &) = delete;

        // 自定义构造函数，构造存储的数据
        stor_impl(const T &dat) : data(dat) {}

        // 以下五个函数为实现基类的 virtual 函数
        std::type_index type() const noexcept override {
            return typeid(T);
        }

        void suicide(bool is_static) override {
            if (is_static)
                this->~stor_impl();
            else
                allocator.free(this);
        }

        void clone(byte_t *ptr) const override {
            ::new(ptr) stor_impl<T>(data);
        }

        stor_base *clone() const override {
            return allocator.alloc(data);
        }
        /*
        type_support *extension() const override
        {
            // NOT IMPLEMENTED YET
            return nullptr;
        }
        */
    };
    /*
        实现小对象优化
        减少内存分配瓶颈
    */

    // 存储状态，分别为无数据、未触发优化、已触发优化
    enum class stor_status {
        null,
        ptr,
        data
    };

    // 使用联合实现
    struct stor_union {
        // 触发小对象优化的阈值，需大于 std::alignment_of<stor_base *>::value
        static constexpr unsigned int static_stor_size = 3 * std::alignment_of<stor_base *>::value;
        union {
            // 使用无符号字符数组提供存储数据的原始内存空间
            unsigned char data[static_stor_size];
            // 超出大小阈值的数据即存储在堆上
            stor_base *ptr;
        } impl;
        // 存储状态
        stor_status status = stor_status::null;
    };

    stor_union m_data;

    // 内部方法封装

    // 获取 stor_base 指针方法的封装
    inline stor_base *get_handler() {
        switch (m_data.status) {
            case stor_status::null:
                throw_ex<runtime_error>("Access null any object.");
            case stor_status::data:
                return reinterpret_cast<stor_base *>(m_data.impl.data);
            case stor_status::ptr:
                return m_data.impl.ptr;
        }
    }

    // 常量重载
    inline const stor_base *get_handler() const {
        switch (m_data.status) {
            case stor_status::null:
                throw_ex<runtime_error>("Access null any object.");
            case stor_status::data:
                return reinterpret_cast<const stor_base *>(m_data.impl.data);
            case stor_status::ptr:
                return m_data.impl.ptr;
        }
    }

    // 回收方法的封装
    inline void recycle() {
        if (m_data.status != stor_status::null) {
            get_handler()->suicide(m_data.status == stor_status::data);
            MOZART_LOGEV(m_data.status == stor_status::data ? "Any Small Data Recycled." : "Any Normal Data Recycled.")
        }
    }

    // 存储方法的封装
    template<typename T>
    inline void store(const T &val) {
        if (sizeof(stor_impl<T>) <= stor_union::static_stor_size) {
            ::new(m_data.impl.data) stor_impl<T>(val);
            m_data.status = stor_status::data;
            MOZART_LOGEV("Any SDO Enabled.")
        } else {
            m_data.impl.ptr = stor_impl<T>::allocator.alloc(val);
            m_data.status = stor_status::ptr;
            MOZART_LOGEV("Any SDO Disabled.")
        }
    }

    // 复制方法的封装
    inline void copy(const any &data) {
        if (data.m_data.status != stor_status::null) {
            const stor_base *ptr = data.get_handler();
            if (data.m_data.status == stor_status::ptr) {
                recycle();
                m_data.impl.ptr = ptr->clone();
                MOZART_LOGEV("Any Normal Data Copied.")
            } else {
                ptr->clone(m_data.impl.data);
                MOZART_LOGEV("Any Small Data Copied.")
            }
            m_data.status = data.m_data.status;
        }
    }

public:
    // 交换函数，这里直接调用标准实现
    inline void swap(any &val) noexcept {
        mpp::swap(m_data, val.m_data);
    }

    // 右值引用重载
    inline void swap(any &&val) noexcept {
        mpp::swap(m_data, val.m_data);
    }

    // 默认构造函数
    any() {}

    // 自定义构造函数，未标记为 explicit 以允许隐式转换
    template<typename T>
    any(const T &val) {
        store(val);
    }

    // 复制构造函数
    any(const any &val) {
        copy(val);
    }

    // 移动构造函数
    any(any &&val) noexcept {
        swap(val);
    }

    // 析构函数
    ~any() {
        recycle();
    }

    // 赋值函数，实际上为重载赋值运算符
    template<typename T>
    inline any &operator=(const T &val) {
        recycle();
        store(val);
        return *this;
    }

    // 自赋值重载
    inline any &operator=(const any &val) {
        if (&val != this)
            copy(val);
        return *this;
    }

    // 右值引用重载
    inline any &operator=(any &&val) noexcept {
        swap(val);
        return *this;
    }

    // 获取存储数据的类型，若为空则返回void
    inline std::type_index data_type() const noexcept {
        if (m_data.status == stor_status::null)
            return typeid(void);
        else
            return get_handler()->type();
    }

    // 提取数据方法封装
    template<typename T>
    inline T &get() {
        stor_base *ptr = get_handler();
        if (ptr->type() != typeid(T))
            throw_ex<runtime_error>("Access wrong type of any.");
        return static_cast<stor_impl<T> *>(ptr)->data;
    }

    // 常量重载
    template<typename T>
    inline const T &get() const {
        const stor_base *ptr = get_handler();
        if (ptr->type() != typeid(T))
            throw_ex<runtime_error>("Access wrong type of any.");
        return static_cast<const stor_impl<T> *>(ptr)->data;
    }
};
