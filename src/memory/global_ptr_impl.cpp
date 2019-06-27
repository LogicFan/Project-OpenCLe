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

#if 0

global_ptr_impl::global_ptr_impl(global_ptr_impl &&rhs)
    : valid_{rhs.valid_}, size_{rhs.size_}, host_ptr_{rhs.host_ptr_}, deleter_{rhs.deleter_}, device_ptr_{rhs.device_ptr_},
      on_device_{rhs.on_device_}
{
    logger("global_ptr_impl(move), " << &rhs << " -> " << this);
    rhs.host_ptr_ = nullptr;
    rhs.deleter_ = nullptr;
    rhs.device_ptr_ = nullptr;
    rhs.on_device_ = nullptr;
    return;
}

global_ptr_impl::~global_ptr_impl()
{
    logger("~global_ptr_impl, destory " << this);
    if (host_ptr_)
    {
        deleter_(host_ptr_);
        logger("Release host-side memory " << host_ptr_ << "!");
        host_ptr_ = nullptr;
        deleter_ = nullptr;
    }

    if (device_ptr_)
    {
        clReleaseMemObject(device_ptr_);
        logger("Release device-side memory " << device_ptr_ << "!");
        device_ptr_ = nullptr;
        on_device_ = nullptr;
    }

    size_ = 0;
    valid_ = false;
    return;
}

global_ptr_impl &global_ptr_impl::operator=(global_ptr_impl &&rhs)
{
    logger("operator=(move), " << &rhs << " -> " << this);
    ~global_ptr_impl();
    new (this) global_ptr_impl(std::move(rhs));
    return *this;
}

void *global_ptr_impl::get()
{
    logger("get");
    if (valid_)
    {
        if (host_ptr_ && device_ptr_)
        {
            cl_int status;
            status = clEnqueueReadBuffer(on_device_->cmd_queue_, device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Synchronize memory " << device_ptr_ << " on " << *on_device_ << " to host-side!");
        }
        else if (host_ptr_)
        {
        }
        else if (device_ptr_)
        {
            host_ptr_ = new char[size_];
            deleter_ = [](void const *ptr) { delete[] static_cast<char const *>(ptr); };
            cl_int status;
            status = clEnqueueReadBuffer(on_device_->cmd_queue_, device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
            if (status != CL_SUCCESS)
            {
                throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
            }
            logger("Synchronize memory " << device_ptr_ << " on " << *on_device_ << " to host-side!");
        }
        logger("Get host-side memory " << host_ptr_ << "!");
        return host_ptr_;
    }
    else
    {
        logger("Get nullptr!");
        return nullptr;
    }
    return nullptr;
}

void const *global_ptr_impl::get() const
{
    logger("get const");
    if (valid_ && host_ptr_)
    {
        logger("Get host-side memory " << host_ptr_ << "!");
        return host_ptr_;
    }
    else
    {
        logger("Get nullptr!");
        return nullptr;
    }
    return nullptr;
}

void *global_ptr_impl::release()
{
    logger("release");
    void *ptr = get();
    host_ptr_ = nullptr;
    deleter_ = nullptr;
    ~global_ptr_impl();
    global_ptr_impl();
    logger("Get host-side memory " << ptr << " and release " << this << "!");
    return ptr;
}

global_ptr_impl global_ptr_impl::clone()
{
    logger("clone");
    if (valid_)
    {
        if (host_ptr_)
        {
            get();
            char *new_ptr = new char[size_];
            memcpy(new_ptr, host_ptr_, size_);

            std::function<void(void const *)> new_deleter = [](void const *ptr) { delete[] static_cast<char const *>(ptr); };

            return global_ptr_impl(new_ptr, size_, new_deleter);
        }
        else
        {
            return global_ptr_impl(size_);
        }
    }
    else
    {
        return global_ptr_impl();
    }
    return {};
}

global_ptr_impl global_ptr_impl::clone() const
{
    logger("clone const");
    if (valid_)
    {
        if (host_ptr_)
        {
            char *new_ptr = new char[size_];
            memcpy(new_ptr, host_ptr_, size_);

            std::function<void(void const *)> new_deleter = [](void const *ptr) { delete[] static_cast<char const *>(ptr); };

            return global_ptr_impl(new_ptr, size_, new_deleter);
        }
        else
        {
            global_ptr_impl(size_);
        }
    }
    else
    {
        return global_ptr_impl();
    }
    return {};
}

global_ptr_impl::operator bool() const
{
    logger("operator bool");
    return valid_ && host_ptr_;
}

size_t global_ptr_impl::size() const
{
    logger("size");
    return size_;
}

cl_mem global_ptr_impl::to_device(device const &dev)
{
    logger("to_device (device) const " << dev << "!");
    global_ptr_impl::to_device(dev.get());
}

cl_mem global_ptr_impl::to_device(device_impl const *dev)
{
    logger("to_device (device_impl*) " << dev << "!");
    cl_int status;
    if (valid_)
    {
        if (host_ptr_)
        {
            if (on_device_ == dev)
            {
                status = clEnqueueWriteBuffer(dev->cmd_queue_, device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
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
                device_ptr_ = clCreateBuffer(on_device_->context_, CL_MEM_READ_WRITE, size_, NULL, &status);
                if (status != CL_SUCCESS)
                {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");

                status = clEnqueueWriteBuffer(on_device_->cmd_queue_, device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
                if (status != CL_SUCCESS)
                {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Synchronize memory " << host_ptr_ << " to device " << *on_device_ << "!");
            }
            else
            {
                on_device_ = dev;
                device_ptr_ = clCreateBuffer(on_device_->context_, CL_MEM_READ_WRITE, size_, NULL, &status);
                if (status != CL_SUCCESS)
                {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");

                status = clEnqueueWriteBuffer(on_device_->cmd_queue_, device_ptr_, CL_TRUE, 0, size_, host_ptr_, 0, NULL, NULL);
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
                device_ptr_ = clCreateBuffer(on_device_->context_, CL_MEM_READ_WRITE, size_, NULL, &status);
                if (status != CL_SUCCESS)
                {
                    throw std::runtime_error{"OpenCL runtime error: Cannot create memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
            }
            else
            {
                on_device_ = dev;
                device_ptr_ = clCreateBuffer(on_device_->context_, CL_MEM_READ_WRITE, size_, NULL, &status);
                if (status != CL_SUCCESS)
                {
                    throw std::runtime_error{"OpenCL runtime error: Cannot create memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
            }
        }
        return device_ptr_;
    }
    else
    {
        return nullptr;
    }
    return nullptr;
}

cl_mem global_ptr_impl::to_device(device const &dev) const
{
    logger("to_device (device) const " << dev << "!");
    global_ptr_impl::to_device(dev.get());
}

cl_mem global_ptr_impl::to_device(device_impl const *dev) const
{
    logger("to_device (device_impl*) const " << dev << "!");
    cl_int status;
    if (valid_)
    {
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
                    clCreateBuffer(on_device_->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size_, host_ptr_, &status);
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
                    clCreateBuffer(on_device_->context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size_, host_ptr_, &status);
                if (status != CL_SUCCESS)
                {
                    throw std::runtime_error{"OpenCL runtime error: Cannot read memory buffer!"};
                }
                logger("Create memory " << device_ptr_ << " on device " << *on_device_ << "!");
            }
        }
        else
        {
            throw std::runtime_error{"Non-initialize const memory!"};
        }
        return device_ptr_;
    }
    else
    {
        return nullptr;
    }
}

#endif

} // namespace opencle