#include "global_ptr_impl.hpp"
#include "../device/device.hpp"
#include <stdexcept>
#include <stdlib.h>

namespace opencle {
global_ptr_impl::global_ptr_impl(size_t size)
    : host_ptr_{nullptr}, size_{size}, deleter_{}, device_ptr_{nullptr},
      on_device_{nullptr}, write_only_{true} {
    return;
}

global_ptr_impl::global_ptr_impl(void *ptr, size_t size,
                                 std::function<void(void *)> deleter)
    : host_ptr_{ptr}, size_{size}, deleter_{deleter}, device_ptr_{nullptr},
      on_device_{nullptr}, write_only_{false} {
    return;
}

global_ptr_impl::global_ptr_impl(global_ptr_impl &&rhs)
    : host_ptr_{rhs.host_ptr_}, size_{rhs.size_}, deleter_{rhs.deleter_},
      device_ptr_{rhs.device_ptr_}, on_device_{rhs.on_device_},
      write_only_{rhs.write_only_} {
    rhs.host_ptr_ = nullptr;
    rhs.device_ptr_ = nullptr;
    rhs.on_device_ = nullptr;
}

global_ptr_impl::~global_ptr_impl() {
    if (!this->host_ptr_) {
        this->deleter_(this->host_ptr_);
        this->host_ptr_ = nullptr;
    }

    if (!this->device_ptr_) {
        clReleaseMemObject(this->device_ptr_);
        this->device_ptr_ = nullptr;
        this->on_device_ = nullptr;
    }
}

global_ptr_impl &global_ptr_impl::operator=(global_ptr_impl &&rhs) {
    this->~global_ptr_impl();
    new (this) global_ptr_impl(std::move(rhs));
    return *this;
}

void *global_ptr_impl::get() {
    this->write_only_ = false;

    if (!this->host_ptr_) {
        // If the memory is on the host-side;
        return this->host_ptr_;
    } else if (!this->device_ptr_) {
        // If the memory is on the device-side;

        // Allocate memory on host-side;
        this->host_ptr_ = new char[size_];
        this->deleter_ = [](void *ptr) { delete[] ptr; };

        // Enqueue memory to host-side;
        cl_int status = clEnqueueReadBuffer(
            this->on_device_->c_queue_, this->device_ptr_, CL_TRUE, 0,
            this->size_, this->host_ptr_, 0, NULL, NULL);

        // Clean device-side memory
        clReleaseMemObject(this->device_ptr_);
        this->device_ptr_ = nullptr;
        this->on_device_ = nullptr;

        if (status == CL_SUCCESS) {
            return host_ptr_;
        } else {
            this->on_device_->valid_ = false;
            throw std::runtime_error{
                "OpenCL runtime error: Can not enqueue memory buffer!"};
        }
    } else {
        this->host_ptr_ = new char[size_];
        this->deleter_ = [](void *ptr) { delete[] ptr; };

        return host_ptr_;
    }
}

void *global_ptr_impl::get() const { return this->host_ptr_; }

void *global_ptr_impl::release() {
    void *ret = this->get();
    this->host_ptr_ = nullptr;
    this->device_ptr_ = nullptr;
    this->on_device_ = nullptr;

    return ret;
}

global_ptr_impl::operator bool() const {
    return this->host_ptr_ || this->device_ptr_;
}

cl_mem global_ptr_impl::to_device(device const &dev) {
    if (&dev == this->on_device_) {
        // If the memory is on the device-side dev
        return device_ptr_;
    } else if (write_only_) {
        // If the memory is write only
        this->on_device_ = &dev;

        cl_int status;
        this->device_ptr_ =
            clCreateBuffer(this->on_device_->context_, CL_MEM_WRITE_ONLY,
                           this->size_, NULL, &status);

        if (status != CL_SUCCESS) {
            this->on_device_->valid_ = false;
            throw std::runtime_error{
                "OpenCL runtime error: Can not create memory buffer!"};
        }
    } else {
        // If the memory is on the other device

        // Enqueue data to host-side
        this->get();

        // Enqueue data to dev
        this->on_device_ = &dev;
        cl_int status;
        this->device_ptr_ =
            clCreateBuffer(this->on_device_->context_, CL_MEM_READ_WRITE,
                           this->size_, NULL, &status);

        clEnqueueWriteBuffer(this->on_device_->c_queue_, this->device_ptr_,
                             CL_TRUE, 0, this->size_, this->host_ptr_, 0, NULL,
                             NULL);

        if (status != CL_SUCCESS) {
            this->on_device_->valid_ = false;
            throw std::runtime_error{
                "OpenCL runtime error: Can not enqueue memory buffer!"};
        }

        // Clean host-side;
        this->deleter_(this->host_ptr_);
        this->host_ptr_ = nullptr;
    }
}

cl_mem global_ptr_impl::to_device(device const &dev) const {
    if (&dev == this->on_device_) {
        // If the memory is on the device-side dev
        return device_ptr_;
    } else if (!write_only_) {
        // Otherwise there must be a copy on the host-side
        this->on_device_ = &dev;
        cl_int status;
        this->device_ptr_ =
            clCreateBuffer(this->on_device_->context_, CL_MEM_READ_ONLY,
                           this->size_, NULL, &status);

        clEnqueueWriteBuffer(this->on_device_->c_queue_, this->device_ptr_,
                             CL_TRUE, 0, this->size_, this->host_ptr_, 0, NULL,
                             NULL);

        if (status != CL_SUCCESS) {
            this->on_device_->valid_ = false;
            throw std::runtime_error{
                "OpenCL runtime error: Can not enqueue memory buffer!"};
        }
    } else {
        throw std::runtime_error{"OpenCL runtime error: A memory buffer can "
                                 "not be both write only and read only."};
    }
}

}; // namespace opencle

#if 0

global_ptr_impl &operator=(global_ptr_impl const &rhs) = delete;
global_ptr_impl &operator=(global_ptr_impl &&rhs);

void *get() const;
operator bool();

/** For device **/
cl_mem to_device(cl_device_id device);

#endif