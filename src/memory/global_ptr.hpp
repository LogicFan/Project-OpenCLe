#pragma once

#include "../util/core_def.hpp"
#include "../util/logger/logger.hpp"
#include "global_ptr_impl.hpp"
#include <initializer_list>
#include <iostream>
#include <memory.h>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace {
class dummy {};
} // namespace

namespace opencle_test {
    void test();
}

namespace opencle {

template <typename T> struct resolve_global_ptr {
    using type = typename std::conditional_t<std::is_const_v<T>, const global_ptr_impl, global_ptr_impl>;
};

template <typename T>
using global_ptr_t = typename resolve_global_ptr<T>::type;

template <typename T, typename X = void> class global_ptr;

template <typename T> class global_ptr<T[], std::enable_if_t<std::is_pod_v<T>>> final {
private:

    using U = typename std::remove_const<T>::type;
    std::unique_ptr<global_ptr_t<T>> impl_;

    global_ptr *nxt;
    global_ptr *pre;

public:
    global_ptr() {
        logger("global_ptr, create " << this);
        impl_ = std::make_unique<global_ptr_t<T>>();
        nxt = nullptr;
        pre = nullptr;
        return;
    }

    global_ptr(size_t size) {
        logger("global_ptr(size_t), create " << this);
        impl_ = std::make_unique<global_ptr_t<T>>(size * sizeof(T));
        nxt = nullptr;
        pre = nullptr;
        return;
    }

    global_ptr(std::unique_ptr<T[]> ptr, size_t size) {
        logger("global_ptr(unique_ptr, size_t), create " << this);
        std::function<void(void const *)> deleter = [del = ptr.get_deleter()](void const * p) {
            del(reinterpret_cast<T *>(const_cast<void *>(p)));
        };

        impl_ = std::make_unique<global_ptr_t<T>>(ptr.release(), size * sizeof(T), deleter);
        nxt = nullptr;
        pre = nullptr;
        return;
    }

    global_ptr(T *ptr, size_t size) {
        logger("global_ptr(T*, size_t), create " << this);
        U *new_ptr = new U[size];
        std::function<void(void const *)> deleter = [](void const * p) { delete[] static_cast<T const *>(p); };
        memcpy(new_ptr, ptr, size * sizeof(T));

        impl_ = std::make_unique<global_ptr_t<T>>(new_ptr, size * sizeof(T), deleter);
        nxt = nullptr;
        pre = nullptr;
        return;
    }

    global_ptr(std::initializer_list<T> const &il) {
        logger("global_ptr(initializer_list), create " << this);
        size_t size = il.size();
        U *new_ptr = new U[size];
        std::function<void(void const *)> deleter = [](void const * p) { delete[] static_cast<T const *>(p); };

        int i = 0;
        for (auto const &e : il) {
            new_ptr[i] = e;
            ++i;
        }

        impl_ = std::make_unique<global_ptr_t<T>>(new_ptr, size * sizeof(T), deleter);
        nxt = nullptr;
        pre = nullptr;
        return;
    }

    // template <class C> global_ptr(C const &container, std::enable_if_t<std::is_class_v<C>, dummy> = dummy{}) {
    //     size_t size = container.size();
    //     U *new_ptr = new U[size];
    //     std::function<void(void const *)> deleter = [](void const * p) { delete[] static_cast<T const *>(p); };

    //     int i = 0;
    //     for (auto const &e : container) {
    //         new_ptr[i] = e;
    //         ++i;
    //     }

    //     impl_ = std::make_unique<global_ptr_t<T>>(new_ptr, size * sizeof(T), deleter);
    //     nxt = nullptr;
    //     pre = nullptr;
    //     logger("Container constructor, create " << this);
    //     return;
    // }

    global_ptr(global_ptr const &rhs) = delete;
    global_ptr(global_ptr &&rhs) = default;
    ~global_ptr() = default;

    global_ptr &operator=(global_ptr const &rhs) = delete;
    global_ptr &operator=(global_ptr &&rhs) = default;

    T &operator[](size_t index) {
        logger("operator[" << index << "]");
        T *ptr = static_cast<T *>(impl_->get());
        return ptr[index];
    }

    T &at(size_t index) {
        logger("at(" << index << ")");
        if (index < impl_->size()) {
            return operator[](index);
        } else {
            throw std::out_of_range{"global_ptr out of range"};
        }
    }

    size_t size() {
        logger("size");
        return impl_->size() / sizeof(T);
    }

    template <typename X = T, typename = std::enable_if_t<!std::is_const_v<X>>>
    T *allocate() {
        logger("allocate");
        if (impl_->size() == 0) {
            throw std::runtime_error{"Unknown size of global_ptr!"};
        } else if (impl_->operator bool()) {
            throw std::runtime_error{"Cannot reallocate memory!"};
        } else {
            size_t size = impl_->size();
            T *new_ptr = new T[size];
            std::function<void(void const *)> deleter = [](void const * p) { delete[] static_cast<T const *>(p); };

            impl_->~global_ptr_impl();
            new (impl_.get()) global_ptr_impl{new_ptr, size, deleter};

            return new_ptr;
        }
        return nullptr;
    }

    T *release() {
        logger("release");
        T *ptr = static_cast<T *>(impl_->release());
        ~global_ptr();
        new (this) global_ptr{};
    }

    template <typename ...Args>
    void reset(Args &&...args) {
        logger("reset");
        ~global_ptr();
        new (this) global_ptr{std::forward<Args>(args)...};
    }

    global_ptr clone() {
        logger("clone");
        global_ptr new_global_ptr;
        if(impl_) {
            new_global_ptr.impl_ = std::make_unique<global_ptr_t<T>>(impl_->clone());  
        } 
        return new_global_ptr;
    }

    T *get() {
        logger("get");
        return static_cast<T *>(impl_->get());
    }

    friend class device_impl;
    
    friend void ::opencle_test::test();
};

} // namespace opencle
