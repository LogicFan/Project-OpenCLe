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

namespace opencle {
template <typename T, typename X = void> class global_ptr;

template <typename T> class global_ptr<T[], std::enable_if_t<std::is_pod_v<T>>> final {
  private:
    mutable std::unique_ptr<global_ptr_impl> impl_;

    global_ptr *nxt;
    global_ptr *pre;

  public:
    global_ptr() {
        impl_ = std::make_unique<global_ptr_impl>();
        nxt = nullptr;
        pre = nullptr;
        logger("Default constructor, create " << this);
        return;
    }

    global_ptr(size_t size) {
        impl_ = std::make_unique<global_ptr_impl>(size * sizeof(T));
        nxt = nullptr;
        pre = nullptr;
        logger("Size constructor, create " << this);
        return;
    }

    global_ptr(std::unique_ptr<T[]> ptr, size_t size) {
        std::function<void(void const *)> deleter = [del = ptr.get_deleter()](void const *p) {
            del(reinterpret_cast<T *>(const_cast<void *>(p)));
        };

        impl_ = std::make_unique<global_ptr_impl>(ptr.release(), size * sizeof(T), deleter);
        nxt = nullptr;
        pre = nullptr;
        logger("Unique ptr constructor, create " << this);
        return;
    }

    global_ptr(T *ptr, size_t size) {
        T *new_ptr = new T[size];
        std::function<void(void const *)> deleter = [](void const *p) { delete[] static_cast<T const *>(p); };
        memcpy(new_ptr, ptr, size * sizeof(T));

        impl_ = std::make_unique<global_ptr_impl>(new_ptr, size * sizeof(T), deleter);
        nxt = nullptr;
        pre = nullptr;
        logger("Non-ownership ptr constructor, create " << this);
        return;
    }

    global_ptr(std::initializer_list<T> const &il) {
        size_t size = il.size();
        T *new_ptr = new T[size];
        std::function<void(void const *)> deleter = [](void const *p) { delete[] static_cast<T const *>(p); };

        int i = 0;
        for (auto const &e : il) {
            new_ptr[i] = e;
            ++i;
        }

        impl_ = std::make_unique<global_ptr_impl>(new_ptr, size * sizeof(T), deleter);
        nxt = nullptr;
        pre = nullptr;
        logger("Initializer list constructor, create " << this);
        return;
    }

    template <class U> global_ptr(U const &container, std::enable_if_t<std::is_class_v<U>, dummy> = dummy{}) {
        size_t size = container.size();
        T *new_ptr = new T[size];
        std::function<void(void const *)> deleter = [](void const *p) { delete[] static_cast<T const *>(p); };

        int i = 0;
        for (auto const &e : container) {
            new_ptr[i] = e;
            ++i;
        }

        impl_ = std::make_unique<global_ptr_impl>(new_ptr, size * sizeof(T), deleter);
        nxt = nullptr;
        pre = nullptr;
        logger("Container constructor, create " << this);
        return;
    }

    global_ptr(global_ptr const &rhs) = delete;
    global_ptr(global_ptr &&rhs) = default;
    ~global_ptr() = default;

    global_ptr &operator=(global_ptr const &rhs) = delete;
    global_ptr &operator=(global_ptr &&rhs) = default;

    T &operator[](size_t index) {
        T *ptr = static_cast<T *>(impl_->get());
        return ptr[index];
    }

    T &at(size_t index) {
        if (index < impl_->size()) {
            return operator[](index);
        } else {
            throw std::out_of_range{"global_ptr out of range"};
        }
    }

    size_t size() {
        return impl_->size() / sizeof(T);
    }

    T *allocate() {
        if (impl_->size() == 0) {
            throw std::runtime_error{"Unknown size of global_ptr!"};
        } else if (impl_->operator bool()) {
            throw std::runtime_error{"Cannot reallocate memory!"};
        } else {
            size_t size = impl_->size();
            T *new_ptr = new T[size];
            std::function<void(void const *)> deleter = [](void const *p) { delete[] static_cast<T const *>(p); };

            impl_->~global_ptr_impl();
            new (impl_.get()) global_ptr_impl{new_ptr, size, deleter};

            return new_ptr;
        }
        return nullptr;
    }

    T *release() {
        if (impl_) {
            T *ptr = static_cast<T *>(impl_->release());
            impl_.reset();
            return ptr;
        } else {
            return nullptr;
        }
    }

    void reset() { 
        ~global_ptr();
        new (this) global_ptr{};
    }

    void reset(size_t size) {
        ~global_ptr();
        new (this) global_ptr{size};
    }

    void reset(std::unique_ptr<T[]> ptr, size_t size) {
        ~global_ptr();
        new (this) global_ptr{ptr, size};
    }

    void reset(T *ptr, size_t size) {
        ~global_ptr();
        new (this) global_ptr{ptr, size};
    }

    void swap(global_ptr &rhs) { std::swap(impl_, rhs.impl_); }

    global_ptr clone() {
        global_ptr new_global_ptr;
        new_global_ptr.impl_ = std::make_unique<global_ptr_impl>(impl_->clone());
        return new_global_ptr;
    }

    T *get() {
        return static_cast<T *>(impl_->get());
    }
};

} // namespace opencle
