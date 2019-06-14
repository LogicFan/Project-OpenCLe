#include "../util/logger/logger.hpp"
#include "global_ptr_impl.hpp"
#include <iostream>
#include <memory.h>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <initializer_list>

namespace {
class dummy {};
} // namespace

namespace opencle {
template <typename T, typename X = void> class global_ptr;

template <typename T> class global_ptr<T[], std::enable_if_t<std::is_pod_v<T>>> final {
  private:
    std::unique_ptr<global_ptr_impl> ptr_;

  public:
    global_ptr() {
        logger("Default constructor, create " << this);
        return;
    }

    global_ptr(size_t size) : ptr_{std::make_unique<global_ptr_impl>(size * sizeof(T))} {
        ptr_ = std::make_unique<global_ptr_impl>(size * sizeof(T));
        logger("Size constructor, create " << this);
        return;
    }

    global_ptr(std::unique_ptr<T[]> ptr, size_t size) {
        std::function<void(void const *)> deleter = [del = ptr.get_deleter()](void const *p) {
            del(reinterpret_cast<T *>(const_cast<void *>(p)));
        };
        ptr_ = std::make_unique<global_ptr_impl>(ptr.release(), size * sizeof(T), deleter);
        logger("Unique ptr constructor, create " << this);
        return;
    }

    global_ptr(T *ptr, size_t size) {
        T *new_ptr = new T[size];
        std::function<void(void const *)> deleter = [](void const *p) { delete[] static_cast<T const *>(p); };
        memcpy(new_ptr, ptr, size * sizeof(T));

        ptr_ = std::make_unique<global_ptr_impl>(new_ptr, size * sizeof(T), deleter);
        logger("Non-ownership ptr constructor, create " << this);
        return;
    }
    
    global_ptr(std::initializer_list<T> const &il) {
        size_t size = il.size();
        T *new_ptr = new T[size];
        std::function<void(void const *)> deleter = [](void const *p) { delete[] static_cast<T const *>(p); };
        
        int i = 0;
        for(auto const &e: il) {
            new_ptr[i] = e;
            ++i;
        }

        ptr_ = std::make_unique<global_ptr_impl>(new_ptr, size * sizeof(T), deleter);
        logger("Initializer list constructor, create " << this);
        return;
    }

    template <class U> global_ptr(U const &container, std::enable_if_t<std::is_class_v<U>, dummy> = dummy{}) {
        size_t size = container.size();
        T *new_ptr = new T[size];
        std::function<void(void const *)> deleter = [](void const *p) { delete[] static_cast<T const *>(p); };
        
        int i = 0;
        for(auto const &e: container) {
            new_ptr[i] = e;
            ++i;
        }

        ptr_ = std::make_unique<global_ptr_impl>(new_ptr, size * sizeof(T), deleter);
        logger("Container constructor, create " << this);
        return;
    }

    global_ptr(global_ptr const &rhs) = delete;
    global_ptr(global_ptr &&rhs) = default;
    ~global_ptr() = default;

    global_ptr &operator=(global_ptr const &rhs) = delete;
    global_ptr &operator=(global_ptr &&rhs) = default;
    
    T &operator[](size_t index) {
        T *ptr = static_cast<T *>(ptr_->get());
        return ptr[index];
    }
    
    T &at(size_t index) {
        if (index < ptr_->size()) {
            return operator[](index);
        } else {
            throw std::out_of_range{"global_ptr out of range"};
        }
    }

    // void allocate();

    // T *release();

    // void reset();
    // void reset(size_t size);
    // void reset(std::unique_ptr<T[]> ptr, size_t size);
    // void reset(T *ptr, size_t size);
    // template <typename U> void reset(U &&container);

    // void swap(global_ptr &rhs);

    // T *get();
};

} // namespace opencle
