#include "global_ptr_impl.hpp"
#include "../device/device_impl.hpp"
#include <assert.h>
#include <stdexcept>
#include <stdlib.h>

namespace opencle {

global_ptr_impl::global_ptr_impl(size_t size)
    : global_ptr_impl{new char[size], size,
                      [](void const *ptr) { delete[] ptr; }} {
    logger("Calling 1 parameter constructor, construct " << this << std::endl);
    return;
}

global_ptr_impl::global_ptr_impl(void *ptr, size_t size,
                                 std::function<void(void const *)> deleter)
    : size_{size}, host_ptr_{ptr}, deleter_{deleter}, device_ptr_{nullptr},
      on_device_{nullptr} {
    logger("Calling 3 parameter constructor, construct " << this << std::endl);
    return;
}

global_ptr_impl::global_ptr_impl(global_ptr_impl &&rhs)
    : size_{rhs.size_}, host_ptr_{rhs.host_ptr_}, deleter_{rhs.deleter_},
      device_ptr_{rhs.device_ptr_}, on_device_{rhs.on_device_} {
    rhs.host_ptr_ = nullptr;
    rhs.device_ptr_ = nullptr;
    logger("Calling move constructor, from " << &rhs << " to " << this << std::endl);
    return;
}

global_ptr_impl::~global_ptr_impl() {
    logger("Calling destructor, destory " << this << std::endl);
    if (host_ptr_) {
        deleter_(this->host_ptr_);
        logger("Release host memory " << host_ptr_ << std::endl);
    }

    if (device_ptr_) {
        clReleaseMemObject(this->device_ptr_);
        logger("Release device memory " << device_ptr_ << std::endl);
    }
}

global_ptr_impl &global_ptr_impl::operator=(global_ptr_impl &&rhs) {
    logger("Calling moving assign operator, from " << &rhs << " to " << this << std::endl);
    this->~global_ptr_impl();

    new (this) global_ptr_impl(std::move(rhs));
    return *this;
}

// cl_mem global_ptr_impl::to_device(device const &dev) {
//     if (on_device_ == &dev) {
//         return device_ptr_;
//     } else if {
//     }
// }

#if 0

void *global_ptr_impl::get() {
    this->write_only_ = false;

    if (this->host_ptr_) {
        std::cout << "get, case A" << std::endl;
        // If the memory is on the host-side;
        return this->host_ptr_;
    } else if (this->device_ptr_) {
        std::cout << "get, case B" << std::endl;
        // If the memory is on the device-side;

        // Allocate memory on host-side;
        this->host_ptr_ = new char[size_];
        this->deleter_ = [](void *ptr) { delete[] static_cast<char *>(ptr); };

        // Enqueue memory to host-side;
        cl_int status = clEnqueueReadBuffer(
            this->on_device_->c_queue_, this->device_ptr_, CL_TRUE, 0,
            this->size_, this->host_ptr_, 0, NULL, NULL);

        // Clean device-side memory
        clReleaseMemObject(this->device_ptr_);
        this->device_ptr_ = nullptr;
        this->on_device_ = nullptr;

        if (status == CL_SUCCESS) {
            assert(this->host_ptr_);

            return this->host_ptr_;
        } else {
            this->on_device_->valid_ = false;
            throw std::runtime_error{
                "OpenCL runtime error: Can not enqueue memory buffer!"};
        }
    } else {
        std::cout << "get, case C" << std::endl;
        this->host_ptr_ = new char[size_];
        this->deleter_ = [](void *ptr) { delete[] static_cast<char *>(ptr); };

        return this->host_ptr_;
    }
}

void *global_ptr_impl::get() const {

    assert(write_only_ || this->host_ptr_);

    return this->host_ptr_;
}

void *global_ptr_impl::release() {
    void *ret = this->get();

    assert(ret);

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
        std::cout << "to_device, case A" << std::endl;

        assert(device_ptr_);
        // If the memory is on the device-side dev
        return device_ptr_;
    } else if (write_only_) {
        std::cout << "to_device, case B" << std::endl;

        assert(!host_ptr_);
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
        return device_ptr_;
    } else {
        std::cout << "to_device, case C" << std::endl;

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
        return device_ptr_;
    }
    assert(false);
    return nullptr;
}

cl_mem global_ptr_impl::to_device(device const &dev) const {
    if (&dev == this->on_device_) {
        // If the memory is on the device-side dev
        return device_ptr_;
    } else if (!write_only_) {
        assert(!host_ptr_);
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
    assert(false);
    return nullptr;
}

#endif

} // namespace opencle
