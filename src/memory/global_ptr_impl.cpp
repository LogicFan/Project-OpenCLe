#include <memory.h>
#include <stdexcept>
#include <stdlib.h>

#include "../device/device_impl.hpp"
#include "../util/logger/logger.hpp"
#include "global_ptr_impl.hpp"

namespace opencle
{
global_ptr_impl::global_ptr_impl(size_t size, bool read_only)
    : valid_{true}, size_{size}, read_only_{read_only}, host_ptr_{nullptr}, deleter_{nullptr}, device_ptr_{nullptr},
      on_device_{nullptr}
{
    logger("global_ptr_impl(size_t, bool), create " << this);
    if (size == 0)
    {
        throw std::runtime_error{"size cannot be 0!"};
    }
    return;
}

global_ptr_impl::global_ptr_impl(void *ptr, size_t size, Deleter deleter, bool read_only)
    : valid_{true}, size_{size}, read_only_{read_only}, host_ptr_{ptr}, deleter_{deleter}, device_ptr_{nullptr}, on_device_{
                                                                                                                     nullptr}
{
    logger("global_ptr_impl(void *, size_t, Deleter, bool), create" << this);
    if (size == 0)
    {
        throw std::runtime_error{"size cannot be 0!"};
    }
    else if (ptr == nullptr)
    {
        throw std::runtime_error{"host pointer cannot be nullptr"};
    }
    return;
}

// global_ptr_impl::global_ptr_impl(global_ptr_impl &&rhs)
//     : valid_{rhs.valid_}, size_{rhs.size_}, read_only_{rhs.read_only_}, host_ptr_{rhs.host_ptr_}, deleter_{rhs.deleter_},
//       device_ptr_{rhs.device_ptr_}, on_device_{rhs.on_device_}
// {
//     logger("global_ptr_impl(global_ptr_impl &&), create " << this << " from " << &rhs);
//     rhs.host_ptr_ = nullptr;
//     rhs.deleter_ = nullptr;
//     rhs.device_ptr_ = nullptr;
//     rhs.on_device_ = nullptr;
//     return;
// }

global_ptr_impl::~global_ptr_impl()
{
    logger("~global_ptr_impl, destory " << this);
    if (host_ptr_ && deleter_)
    {
        deleter_(host_ptr_);
        logger("Release host pointer " << host_ptr_);
    }

    if (device_ptr_)
    {
        clReleaseMemObject(device_ptr_);
        logger("Release device pointer " << device_ptr_);
    }
}

// global_ptr_impl &global_ptr_impl::operator=(global_ptr_impl &&rhs)
// {
//     logger("operator=(global_ptr_impl &&), " << &rhs << " -> " << this);
//     this->~global_ptr_impl();
//     new (this) global_ptr_impl(std::move(rhs));
//     return *this;
// }

void *global_ptr_impl::get_read_write() const
{
    logger("get_read_write() const");
    cl_int status;
    if (host_ptr_ && device_ptr_)
    {
        status = clEnqueueReadBuffer(on_device_->get_command_queue(), device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
        if (status != CL_SUCCESS)
        {
            valid_ = false;
            throw std::runtime_error{"OpenCL runtime error: Cannot read memory "
                                     "buffer!"};
        }
        logger("Synchronize memory " << device_ptr_ << " on " << *on_device_ << " to " << host_ptr_ << " on host");
    }
    return host_ptr_;
}

void *global_ptr_impl::get_read_write()
{
    logger("get_read_write()");
    cl_int status;
    if (host_ptr_ && device_ptr_)
    {
        status = clEnqueueReadBuffer(on_device_->get_command_queue(), device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
        if (status != CL_SUCCESS)
        {
            valid_ = false;
            throw std::runtime_error{"OpenCL runtime error: Cannot read memory "
                                     "buffer!"};
        }
        logger("Synchronize memory " << device_ptr_ << " on " << *on_device_ << " to " << host_ptr_ << " on host");
    }
    else if (device_ptr_)
    {
        host_ptr_ = new char[size_];
        deleter_ = [](void const *ptr) { delete[] static_cast<char const *>(ptr); };
        logger("Allocate memory " << host_ptr_ << " on host");
        status = clEnqueueReadBuffer(on_device_->get_command_queue(), device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
        if (status != CL_SUCCESS)
        {
            valid_ = false;
            throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
        }
        logger("Synchronize memory " << device_ptr_ << " on " << *on_device_ << " to " << host_ptr_ << " on host");
    }
    return host_ptr_;
}

void *global_ptr_impl::get_read_only() const
{
    logger("get_read_only() const");
    return host_ptr_;
}

void *global_ptr_impl::get() const
{
    logger("get() const");
    if (!valid_)
    {
        throw std::runtime_error{"Getting address of an invalid global_ptr"};
    }
    else if (read_only_)
    {
        return get_read_only();
    }
    else
    {
        return get_read_write();
    }
    return nullptr;
}

void *global_ptr_impl::get()
{
    logger("get() const");
    if (!valid_)
    {
        throw std::runtime_error{"Getting address of an invalid global_ptr"};
    }
    else if (read_only_)
    {
        return get_read_only();
    }
    else
    {
        return get_read_write();
    }
    return nullptr;
}

void *global_ptr_impl::release()
{
    logger("release()");
    if (!valid_)
    {
        throw std::runtime_error{"Getting address of an invalid global_ptr"};
    }
    void *temp = get();
    host_ptr_ = nullptr;
    deleter_ = nullptr;
    this->~global_ptr_impl();
    valid_ = false;
    return temp;
}

std::unique_ptr<global_ptr_impl> global_ptr_impl::clone() const
{
    logger("clone() const");
    if (!valid_)
    {
        throw std::runtime_error{"Cannot clone an invalid global_ptr"};
    }

    cl_int status;

    void *new_ptr = new char[size_];
    Deleter new_deleter = [](void const *ptr) { delete[] static_cast<char const *>(ptr); };
    logger("Allocate memory " << new_ptr << " on host");

    if (on_device_ && device_ptr_)
    {
        status = clEnqueueReadBuffer(on_device_->get_command_queue(), device_ptr_, CL_TRUE, 0, size_, new_ptr, 0, NULL, NULL);
        if (status != CL_SUCCESS)
        {
            valid_ = false;
            throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
        }
        logger("Synchronize memory " << device_ptr_ << " on " << *on_device_ << " to " << new_ptr << " on host");

        return std::make_unique<global_ptr_impl>(new_ptr, size_, new_deleter);
    }
    else if (host_ptr_)
    {
        memcpy(new_ptr, host_ptr_, size_);
        logger("Copy memory from " << host_ptr_ << " to " << new_ptr);

        return std::make_unique<global_ptr_impl>(new_ptr, size_, new_deleter);
    }
    else
    {
        return std::make_unique<global_ptr_impl>(size_);
    }
    return {};
}

global_ptr_impl::operator bool() const
{
    logger("operator bool()");
    return host_ptr_;
}

size_t global_ptr_impl::size() const
{
    logger("size() const");
    return size_;
}

bool global_ptr_impl::is_allocated() const
{
    logger("is_allocated() const");
    return host_ptr_;
}

cl_mem global_ptr_impl::to_device_read_write(device_impl const *dev)
{
    logger("to_device_read_write(device_impl const *)");
    cl_int status;
    if (host_ptr_)
    {
        if (on_device_ == dev)
        {
            status = clEnqueueWriteBuffer(dev->get_command_queue(), device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Synchronize memory " << host_ptr_ << " to device " << *on_device_ << "!");
        }
        else if (on_device_)
        {
            get();
            clReleaseMemObject(device_ptr_);
            logger("Release memory " << device_ptr_ << " on device " << *on_device_ << "!");

            on_device_ = dev;
            device_ptr_ = clCreateBuffer(on_device_->get_context(), CL_MEM_READ_WRITE, size_, NULL, &status);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");

            status =
                clEnqueueWriteBuffer(on_device_->get_command_queue(), device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Synchronize memory " << host_ptr_ << " to device " << *on_device_ << "!");
        }
        else
        {
            on_device_ = dev;
            device_ptr_ = clCreateBuffer(on_device_->get_context(), CL_MEM_READ_WRITE, size_, NULL, &status);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");

            status =
                clEnqueueWriteBuffer(on_device_->get_command_queue(), device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Synchronize memory " << host_ptr_ << " to device " << *on_device_ << "!");
        }
    }
    else
    {
        if (on_device_ == dev)
        {
        }
        else if (on_device_)
        {
            clReleaseMemObject(device_ptr_);
            logger("Release memory " << device_ptr_ << " on device " << *on_device_ << "!");

            on_device_ = dev;
            device_ptr_ = clCreateBuffer(on_device_->get_context(), CL_MEM_READ_WRITE, size_, NULL, &status);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot create memory buffer!"};
            }
            logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
        }
        else
        {
            on_device_ = dev;
            device_ptr_ = clCreateBuffer(on_device_->get_context(), CL_MEM_READ_WRITE, size_, NULL, &status);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot create memory buffer!"};
            }
            logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
        }
    }
    return device_ptr_;
}

cl_mem global_ptr_impl::to_device_read_only(device_impl const *dev)
{
    logger("to_device_read_only(device_impl const *)");
    cl_int status;
    if (host_ptr_)
    {
        if (on_device_ == dev)
        {
        }
        else if (on_device_)
        {
            clReleaseMemObject(device_ptr_);
            logger("Release memory " << device_ptr_ << " on device " << *on_device_ << "!");

            on_device_ = dev;
            device_ptr_ =
                clCreateBuffer(on_device_->get_context(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size_, host_ptr_, &status);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
        }
        else
        {
            on_device_ = dev;
            device_ptr_ =
                clCreateBuffer(on_device_->get_context(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size_, host_ptr_, &status);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
        }
    }
    else
    {
        throw std::runtime_error{"Non-initialize read_only memory!"};
    }
    return device_ptr_;
}

cl_mem global_ptr_impl::to_device(device_impl const *dev)
{
    if (!valid_)
    {
        throw std::runtime_error{"Move invalid global_ptr to device"};
    }
    else if (read_only_)
    {
        return to_device_read_only(dev);
    }
    else
    {
        return to_device_read_write(dev);
    }
    return nullptr;
}

} // namespace opencle