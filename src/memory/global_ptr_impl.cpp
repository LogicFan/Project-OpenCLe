#include "global_ptr_impl.hpp"
#include "../device/device_impl.hpp"
#include <assert.h>
#include <memory.h>
#include <stdexcept>
#include <stdlib.h>

namespace opencle {
global_ptr_impl::global_ptr_impl()
    : valid_{false}, size_{0}, host_ptr_{nullptr}, deleter_{nullptr},
      device_ptr_{nullptr}, on_device_{nullptr} {
    logger("Default constructor, " << this << std::endl);
    return;
}

global_ptr_impl::global_ptr_impl(size_t size)
    : valid_{true}, size_{0}, host_ptr_{nullptr}, deleter_{nullptr},
      device_ptr_{nullptr}, on_device_{nullptr} {
    logger("Size constructor, " << this << std::endl);
    return;
}

global_ptr_impl::global_ptr_impl(void *ptr, size_t size,
                                 std::function<void(void const *)> deleter)
    : valid_{true}, size_{size}, host_ptr_{ptr}, deleter_{deleter},
      device_ptr_{nullptr}, on_device_{nullptr} {
    logger("Host pointer constructor, " << this << std::endl);
    return;
}

global_ptr_impl::global_ptr_impl(global_ptr_impl &&rhs)
    : valid_{rhs.valid_}, size_{rhs.size_}, host_ptr_{rhs.host_ptr_},
      deleter_{rhs.deleter_}, device_ptr_{rhs.device_ptr_},
      on_device_{rhs.on_device_} {
    logger("Move constructor, from " << &rhs << " to " << this << std::endl);
    return;
}

global_ptr_impl::~global_ptr_impl() {
    if (host_ptr_) {
        deleter_(host_ptr_);
        host_ptr_ = nullptr;
        deleter_ = nullptr;
    }

    if (device_ptr_) {
        clReleaseMemObject(device_ptr_);
        device_ptr_ = nullptr;
        on_device_ = nullptr;
    }

    size_ = 0;
    valid_ = false;

    logger("Destructor, " << this << std::endl);
    return;
}

global_ptr_impl &global_ptr_impl::operator=(global_ptr_impl &&rhs) {
    ~global_ptr_impl();
    new (this) global_ptr_impl(std::move(rhs));
    logger("Move assignment operator, from " << &rhs << " to " << this);
    return;
}

void *global_ptr_impl::get() {
    logger("Get memory of " << this << std::endl);
    if (valid_) {
        if (device_ptr_) {
            logger("Synchronize with device " << on_device_->get()
                                              << std::endl);
            cl_int status;
            status = clEnqueueReadBuffer(on_device_->get()->cmd_queue_,
                                         device_ptr_, CL_TRUE, 0, size_,
                                         host_ptr_, 0, NULL, NULL);

            if (status != CL_SUCCESS) {
                throw std::runtime_error{
                    "OpenCL runtime error: Cannot read memory buffer"};
            }
        }

        logger("Return pointer " << device_ptr_ << std::endl);

        return device_ptr_;
    } else {
        logger("Return nullptr pointer" << std::endl);
        return nullptr;
    }
    return nullptr;
}

void const *global_ptr_impl::get() const {
    logger("Get memory of " << this << std::endl);
    if (valid_) {
        logger("Return pointer " << device_ptr_ << std::endl);
        return device_ptr_;
    } else {
        logger("Return nullptr pointer" << std::endl);
        return nullptr;
    }
    return nullptr;
}

void *global_ptr_impl::release() {
    logger("Release memory of " << this << std::endl);
    void *ptr = get();
    host_ptr_ = nullptr;
    deleter_ = nullptr;
    ~global_ptr_impl();
    global_ptr_impl();
    return ptr;
}

global_ptr_impl global_ptr_impl::clone() {
    logger("Clone from memory " << this << std::endl);
    if (valid_) {
        get();
        char *new_ptr = new char[size_];
        memcpy(new_ptr, host_ptr_, size_);

        std::function<void(void const *)> new_deleter = [](void const *ptr) {
            delete[] ptr;
        };

        return global_ptr_impl(new_ptr, size_, new_deleter);
    } else {
        return global_ptr_impl();
    }
}

global_ptr_impl global_ptr_impl::clone() const {
    logger("Clone from memory " << this << std::endl);
    if (valid_) {
        get();
        char *new_ptr = new char[size_];
        memcpy(new_ptr, host_ptr_, size_);

        std::function<void(void const *)> new_deleter = [](void const *ptr) {
            delete[] ptr;
        };

        return global_ptr_impl(new_ptr, size_, new_deleter);
    } else {
        return global_ptr_impl();
    }
}

global_ptr_impl::operator bool() const { return valid_ && host_ptr_; }

size_t global_ptr_impl::size() const { return size(); }

// cl_mem global_ptr_impl::to_device(device const &dev) {
//     if(valid_) {

//     } else {
//         return nullptr;
//     }
// }
} // namespace opencle