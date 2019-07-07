#include "device.hpp"

#include <CL/cl.h>
#include <algorithm>

#include "../util/logger/logger.hpp"
#include "device_impl.hpp"

namespace
{
cl_device_type get_cl_device_type(opencle::device_type type)
{
    switch (type)
    {
    case opencle::device_type::CPU:
        return CL_DEVICE_TYPE_CPU;
    case opencle::device_type::GPU:
        return CL_DEVICE_TYPE_GPU;
    case opencle::device_type::ACCELERATOR:
        return CL_DEVICE_TYPE_ACCELERATOR;
    case opencle::device_type::DEFAULT:
        return CL_DEVICE_TYPE_DEFAULT;
    case opencle::device_type::ALL:
        return CL_DEVICE_TYPE_ALL;
    default:
        throw std::out_of_range{"unknown opencle device type"};
    }
}
} // namespace

namespace opencle
{
std::atomic<bool> device::is_device_list_created_ = false;
std::vector<device> device::device_list_ = std::vector<device>{};
std::mutex device::device_list_mutex_ = std::mutex{};

device::device(std::unique_ptr<device_impl> &&dev_impl)
    : impl_{std::move(dev_impl)}
{
    logger("device(std::unique_ptr<device_impl> &&), create " << this);
}

void device::create_device_list(device_type type)
{
    logger("create_device_list(device_type)");

    if (is_device_list_created_)
    {
        throw std::runtime_error{"device_list_ has been created, it should not be created twice"};
    }

    device_list_mutex_.lock();

    cl_int status;

    // get platforms
    logger("getting platforms");

    cl_uint platform_num;
    status = clGetPlatformIDs(0, NULL, &platform_num);

    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize platform"};
    }
    else if (platform_num == 0)
    {
        is_device_list_created_ = true;
        return;
    }

    std::unique_ptr<cl_platform_id[]> platforms{new cl_platform_id[platform_num]};
    status = clGetPlatformIDs(platform_num, platforms.get(), NULL);

    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize platform"};
    }

    for (size_t i = 0; i < platform_num; ++i)
    {
        logger("getting devices on platform " << i);

        cl_uint device_num;
        status = clGetDeviceIDs(platforms[i], get_cl_device_type(type), 0, NULL, &device_num);
        if (status != CL_SUCCESS)
        {
            throw std::runtime_error{"OpenCL runtime error: Cannot initialize device"};
        }
        else if (device_num == 0)
        {
            continue;
        }

        std::unique_ptr<cl_device_id[]> devices{new cl_device_id[device_num]};
        status = clGetDeviceIDs(platforms[i], get_cl_device_type(type), device_num, devices.get(), NULL);

        for (size_t j = 0; j < device_num; ++j)
        {
            device_list_.push_back(device{std::make_unique<device_impl>(devices[i])});
        }
    }

    std::sort(device_list_.begin(), device_list_.end());

    device_list_mutex_.unlock();

    is_device_list_created_ = true;
}

device const &device::get_top_device()
{
    return *device_list_.begin();
}

void device::sort_device_list()
{
    device_list_mutex_.lock();
    std::sort(device_list_.begin(), device_list_.end());
    device_list_mutex_.unlock();
}

device::device(device &&rhs) : impl_{std::move(rhs.impl_)}
{
    logger("device(device &&rhs), create " << this << " from " << &rhs);
}

device::~device()
{
    logger("~device(), destory " << this);
}

device &device::operator=(device &&rhs)
{
    this->~device();
    new (this) device{std::move(rhs)};
    return *this;
}

bool device::operator<(device const &rhs) const
{
    return impl_->operator<(*(rhs.impl_));
}

int device::get_compute_unit_available() const
{
    return impl_->get_compute_unit_available();
}

} // namespace opencle