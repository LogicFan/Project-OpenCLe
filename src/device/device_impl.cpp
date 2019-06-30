#include <stdexcept>
#include <type_traits>
#include <utility>

#include "device_impl.hpp"
#include "../util/logger/logger.hpp"

namespace
{
void __pfn_notify(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
    logger("__pfn_notify(const char *, const void *, size_t, void *)");
    throw std::runtime_error{"OpenCL runtime error: Context failure. " + std::string{errinfo}};
}

cl_context __get_context(cl_device_id const &dev_id)
{
    logger("__get_context(cl_device_id const &)");
    cl_int status;
    cl_context temp = clCreateContext(NULL, 1, &dev_id, __pfn_notify, NULL, &status);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize context!"};
    }
    logger("Create context " << temp << " based on device_id " << dev_id);

    return temp;
}

cl_command_queue __get_command_queue(cl_device_id const &dev_id, cl_context const &context)
{
    logger("__get_command_queue(cl_device_id const &, cl_context const &)");
    cl_int status;
    cl_command_queue temp = clCreateCommandQueueWithProperties(context, dev_id, NULL, &status);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize command queue!"};
    }
    logger("Create command queue " << temp << " based on context " << context << " and device_id " << dev_id);

    return temp;
}

size_t __get_compute_unit(cl_device_id const &dev_id)
{
    logger("__get_compute_unit(cl_device_id const &)");
    cl_int status;
    cl_uint cu_num;
    status = clGetDeviceInfo(dev_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &cu_num, NULL);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot get device info!"};
    }
    logger("There are " << cu_num << " number of compute unit on device where device_id is " << dev_id);

    return static_cast<size_t>(cu_num);
}
} // namespace

namespace opencle
{
device_impl::device_impl(cl_device_id const &dev_id)
    : device_{dev_id}, context_{__get_context(device_)},
      cmd_queue_{__get_command_queue(device_, context_)},
      cu_total_{__get_compute_unit(device_)},
      valid_{true}, cu_used_{0}
{
    logger("device_impl(device_id const &), create " << this);
    return;
}

device_impl::device_impl(cl_device_id const &dev_id, cl_context const &context, cl_command_queue const &cmd_q)
    : device_{dev_id}, context_{context}, cmd_queue_{cmd_q},
      cu_total_{__get_compute_unit(dev_id)},
      valid_{true}, cu_used_{0}
{
    logger("device_impl(device_id const &, context const &, command_queue const &), create " << this);
    return;
}

device_impl::~device_impl()
{
    logger("~device_impl(), destory " << this);
    clReleaseCommandQueue(cmd_queue_);
    logger("Release command queue " << cmd_queue_);
    clReleaseContext(context_);
    logger("Release context " << context_);
}

bool device_impl::operator<(device_impl const &rhs) const
{
    logger("operator<(device_impl const &) const, compare " << this << " and " << &rhs);
    return valid_ * (cu_total_ - cu_used_) < rhs.valid_ * (rhs.cu_total_ - rhs.cu_used_);
}

device_impl::operator bool() const
{
    logger("operator bool() const");
    return valid_;
}

cl_device_id device_impl::get_device_id() const
{
    logger("get_device_id() const");
    return device_;
}

cl_context device_impl::get_context() const
{
    logger("get_context() const");
    return context_;
}

cl_command_queue device_impl::get_command_queue() const
{
    logger("get_command_queue() const");
    return cmd_queue_;
}

int device_impl::get_compute_unit_available() const
{
    logger("get_computate_unit_available() const");
    return static_cast<int>(cu_total_) - static_cast<int>(cu_used_);
}

void device_impl::compute_unit_usage_increment(int offset)
{
    logger("computate_unit_usage_increment(int)");
    cu_used_ = cu_used_ + offset;
}

std::ostream &operator<<(std::ostream &out, device_impl const &dev_impl)
{
    cl_int status;

    char device_name[100];
    status = clGetDeviceInfo(dev_impl.device_, CL_DEVICE_NAME, 100, device_name, NULL);

    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot get device info"};
    }

    out << "Device: " << device_name << " (address: " << &dev_impl << ")";
    return out;
}

} // namespace opencle
