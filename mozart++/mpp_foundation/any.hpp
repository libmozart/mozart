/**
 * Mozart++ Template Library: Any
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <mozart++/core>
#include <mozart++/memory>
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
    template <typename T>
    using default_allocator_provider = mpp::allocator<T>;
    /**
     * Unified definition
     */
    template <typename T>
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
    template <typename T>
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

        stor_impl(const T &dat) : data(dat) {}

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
     * Small object optimization
    */

    /**
     * No data/optimization not triggered/optimization triggered
     */
    enum class stor_status {
        null,
        ptr,
        data
    };

    struct stor_union {
        // threshold of small object optimization, which must be greater than
        // std::alignment_of<stor_base *>::value
        static constexpr unsigned int static_stor_size = 3 * std::alignment_of<stor_base *>::value;

        union {
            // raw memory
            unsigned char data[static_stor_size] = {0};
            // or storing on heap
            stor_base *ptr;
        } impl;

        stor_status status = stor_status::null;
    };

    stor_union m_data;


    /**
     * @return Return inner pointer inside stor_base
     */
    inline stor_base *get_handler() {
        switch (m_data.status) {
            case stor_status::null:
                throw_ex<mpp::runtime_error>("Access null any object.");
            case stor_status::data:
                return reinterpret_cast<stor_base *>(m_data.impl.data);
            case stor_status::ptr:
                return m_data.impl.ptr;
        }
    }

    inline const stor_base *get_handler() const {
        switch (m_data.status) {
            case stor_status::null:
                throw_ex<mpp::runtime_error>("Access null any object.");
            case stor_status::data:
                return reinterpret_cast<const stor_base *>(m_data.impl.data);
            case stor_status::ptr:
                return m_data.impl.ptr;
        }
    }

    inline void recycle() {
        if (m_data.status != stor_status::null) {
            get_handler()->suicide(m_data.status == stor_status::data);
            MOZART_LOGEV(m_data.status == stor_status::data ? "Any Small Data Recycled." : "Any Normal Data Recycled.")
        }
    }

    template <typename T>
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
    inline void swap(any &val) noexcept {
        mpp::swap(m_data, val.m_data);
    }

    inline void swap(any &&val) noexcept {
        mpp::swap(m_data, val.m_data);
    }

    any() = default;

    template <typename T>
    /*implicit*/ any(const T &val) {
        store(val);
    }

    any(const any &val) {
        copy(val);
    }

    any(any &&val) noexcept {
        swap(val);
    }

    ~any() {
        recycle();
    }

    template <typename T>
    inline any &operator=(const T &val) {
        recycle();
        store(val);
        return *this;
    }

    inline any &operator=(const any &val) {
        if (&val != this)
            copy(val);
        return *this;
    }

    inline any &operator=(any &&val) noexcept {
        swap(val);
        return *this;
    }

    /**
     * @return Data type inside any, void if this any holds nothing
     */
    inline std::type_index data_type() const noexcept {
        if (m_data.status == stor_status::null)
            return typeid(void);
        else
            return get_handler()->type();
    }

    template <typename T>
    inline T &get() {
        stor_base *ptr = get_handler();
        if (ptr->type() != typeid(T))
            throw_ex<mpp::runtime_error>("Access wrong type of any.");
        return static_cast<stor_impl<T> *>(ptr)->data;
    }

    template <typename T>
    inline const T &get() const {
        const stor_base *ptr = get_handler();
        if (ptr->type() != typeid(T))
            throw_ex<mpp::runtime_error>("Access wrong type of any.");
        return static_cast<const stor_impl<T> *>(ptr)->data;
    }
};
