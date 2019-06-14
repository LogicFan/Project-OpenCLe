#include "global_ptr_impl.hpp"
#include "../device/device_impl.hpp"
#include <assert.h>
#include <memory.h>
#include <stdexcept>
#include <stdlib.h>

namespace opencle {
global_ptr_impl::global_ptr_impl()
    : valid_{false}, size_{0}, host_ptr_{nullptr}, deleter_{nullptr}, device_ptr_{nullptr}, on_device_{nullptr} {
    logger("Default constructor, create " << this << "!");
    return;
}

global_ptr_impl::global_ptr_impl(size_t size)
    : valid_{true}, size_{size}, host_ptr_{nullptr}, deleter_{nullptr}, device_ptr_{nullptr}, on_device_{nullptr} {
    logger("Size constructor, " << this << "!");
    return;
}

global_ptr_impl::global_ptr_impl(void *ptr, size_t size, std::function<void(void const *)> deleter)
    : valid_{true}, size_{size}, host_ptr_{ptr}, deleter_{deleter}, device_ptr_{nullptr}, on_device_{nullptr} {
    logger("Host-side pointer constructor, " << this << "!");
    return;
}

global_ptr_impl::global_ptr_impl(global_ptr_impl &&rhs)
    : valid_{rhs.valid_}, size_{rhs.size_}, host_ptr_{rhs.host_ptr_}, deleter_{rhs.deleter_}, device_ptr_{rhs.device_ptr_},
      on_device_{rhs.on_device_} {
    logger("Move constructor, from " << &rhs << " to " << this << "!");
    return;
}

global_ptr_impl::~global_ptr_impl() {
    if (host_ptr_) {
        deleter_(host_ptr_);
        logger("Release host-side memory " << host_ptr_ << "!");
        host_ptr_ = nullptr;
        deleter_ = nullptr;
    }

    if (device_ptr_) {
        clReleaseMemObject(device_ptr_);
        logger("Release device-side memory " << device_ptr_ << "!");
        device_ptr_ = nullptr;
        on_device_ = nullptr;
    }

    size_ = 0;
    valid_ = false;

    logger("Destructor, " << this << "!");
    return;
}

global_ptr_impl &global_ptr_impl::operator=(global_ptr_impl &&rhs) {
    ~global_ptr_impl();
    new (this) global_ptr_impl(std::move(rhs));
    logger("Move assignment operator, from " << &rhs << " to " << this << "!");
    return *this;
}

void *global_ptr_impl::get() {
    if (valid_) {
        if (host_ptr_ && device_ptr_) {
            cl_int status;
            status = clEnqueueReadBuffer(on_device_->cmd_queue_, device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
            if (status != CL_SUCCESS) {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Synchronize memory " << device_ptr_ << " on " << *on_device_ << " to host-side!");
        } else if (host_ptr_) {
        } else if (device_ptr_) {
            host_ptr_ = new char[size_];
            deleter_ = [](void const *ptr) { delete[] static_cast<char const *>(ptr); };
            cl_int status;
            status = clEnqueueReadBuffer(on_device_->cmd_queue_, device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
            if (status != CL_SUCCESS) {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Synchronize memory " << device_ptr_ << " on " << *on_device_ << " to host-side!");
        }
        logger("Get host-side memory " << host_ptr_ << "!");
        return host_ptr_;
    } else {
        logger("Get nullptr!");
        return nullptr;
    }
    return nullptr;
}

void const *global_ptr_impl::get() const {
    if (valid_ && host_ptr_) {
        logger("Get host-side memory " << host_ptr_ << "!");
        return host_ptr_;
    } else {
        logger("Get nullptr!");
        return nullptr;
    }
    return nullptr;
}

void *global_ptr_impl::release() {
    void *ptr = get();
    host_ptr_ = nullptr;
    deleter_ = nullptr;
    ~global_ptr_impl();
    global_ptr_impl();
    logger("Get host-side memory " << ptr << " and release " << this << "!");
    return ptr;
}

global_ptr_impl global_ptr_impl::clone() {
    logger("Clone from " << this << "!");
    if (valid_) {
        if (host_ptr_) {
            get();
            char *new_ptr = new char[size_];
            memcpy(new_ptr, host_ptr_, size_);

            std::function<void(void const *)> new_deleter = [](void const *ptr) { delete[] static_cast<char const *>(ptr); };

            return global_ptr_impl(new_ptr, size_, new_deleter);
        } else {
            return global_ptr_impl(size_);
        }
    } else {
        return global_ptr_impl();
    }
    return {};
}

global_ptr_impl global_ptr_impl::clone() const {
    logger("Clone from " << this << "!");
    if (valid_) {
        if (host_ptr_) {
            char *new_ptr = new char[size_];
            memcpy(new_ptr, host_ptr_, size_);

            std::function<void(void const *)> new_deleter = [](void const *ptr) { delete[] static_cast<char const *>(ptr); };

            return global_ptr_impl(new_ptr, size_, new_deleter);
        } else {
            global_ptr_impl(size_);
        }
    } else {
        return global_ptr_impl();
    }
    return {};
}

global_ptr_impl::operator bool() const { return valid_ && host_ptr_; }

size_t global_ptr_impl::size() const { return size_; }

cl_mem global_ptr_impl::to_device(device const &dev) {
    logger("Move memory to device " << dev.get() << "!");
    cl_int status;
    if (valid_) {
        if (host_ptr_) {
            if (on_device_ == dev.get()) {
                status = clEnqueueWriteBuffer(dev->cmd_queue_, device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
                if (status != CL_SUCCESS) {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Synchronize memory " << host_ptr_ << " to device " << *on_device_ << "!");
            } else if (on_device_) {
                get();
                clReleaseMemObject(device_ptr_);
                logger("Release memory " << device_ptr_ << " on device " << *on_device_ << "!");

                on_device_ = dev.get();
                device_ptr_ = clCreateBuffer(on_device_->context_, CL_MEM_READ_WRITE, size_, NULL, &status);
                if (status != CL_SUCCESS) {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");

                status =
                    clEnqueueWriteBuffer(on_device_->cmd_queue_, device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
                if (status != CL_SUCCESS) {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Synchronize memory " << host_ptr_ << " to device " << *on_device_ << "!");
            } else {
                on_device_ = dev.get();
                device_ptr_ = clCreateBuffer(on_device_->context_, CL_MEM_READ_WRITE, size_, NULL, &status);
                if (status != CL_SUCCESS) {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");

                status =
                    clEnqueueWriteBuffer(on_device_->cmd_queue_, device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
                if (status != CL_SUCCESS) {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Synchronize memory " << host_ptr_ << " to device " << *on_device_ << "!");
            }
        } else {
            if (on_device_ == dev.get()) {
            } else if (on_device_) {
                clReleaseMemObject(device_ptr_);
                logger("Release memory " << device_ptr_ << " on device " << *on_device_ << "!");

                on_device_ = dev.get();
                device_ptr_ = clCreateBuffer(on_device_->context_, CL_MEM_READ_WRITE, size_, NULL, &status);
                if (status != CL_SUCCESS) {
                    throw std::runtime_error{"OpenCL runtime error: Cannot create memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
            } else {
                on_device_ = dev.get();
                device_ptr_ = clCreateBuffer(on_device_->context_, CL_MEM_READ_WRITE, size_, NULL, &status);
                if (status != CL_SUCCESS) {
                    throw std::runtime_error{"OpenCL runtime error: Cannot create memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
            }
        }
        return device_ptr_;
    } else {
        return nullptr;
    }
    return nullptr;
}

cl_mem global_ptr_impl::to_device(device const &dev) const {
    logger("Move memory to device " << dev.get() << "!");
    cl_int status;
    if (valid_) {
        if (host_ptr_) {
            if (on_device_ == dev.get()) {
            } else if (on_device_) {
                clReleaseMemObject(device_ptr_);
                logger("Release memory " << device_ptr_ << " on device " << *on_device_ << "!");

                on_device_ = dev.get();
                device_ptr_ = clCreateBuffer(on_device_->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size_, host_ptr_, &status);
                if (status != CL_SUCCESS) {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
            } else {
                on_device_ = dev.get();
                device_ptr_ = clCreateBuffer(on_device_->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size_, host_ptr_, &status);
                if (status != CL_SUCCESS) {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
            }
        } else {
            throw std::runtime_error{"Non-initialize const memory!"};
        }
        return device_ptr_;
    } else {
        return nullptr;
    }
}

} // namespace opencle