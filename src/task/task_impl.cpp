#define NDEBUG

#include "task_impl.hpp"

#include <utility>

#include "../util/logger/logger.hpp"
#include "../device/device_impl.hpp"
#include "../memory/global_ptr_impl.hpp"

namespace opencle
{
task_impl::task_impl(std::string const &source, std::string const &kernel_name)
    : valid_{1}, source_{source}, kernel_name_{kernel_name},
      program_{nullptr}, kernel_{nullptr}, on_device_{nullptr}
{
    logger("task_impl(std::string const &), create " << this);
}

task_impl::~task_impl()
{
    logger("~task_impl()");
    clReleaseKernel(kernel_);
    clReleaseProgram(program_);
}

task_impl::operator bool()
{
    return valid_ == 7;
}

void task_impl::compile(device_impl *dev_impl)
{
    on_device_ = dev_impl;

    cl_int status;
    char const *source = source_.c_str();
    program_ = clCreateProgramWithSource(on_device_->get_context(), 1, &source, NULL, &status);
    if (status != CL_SUCCESS)
    {
        valid_ = 0;
        throw std::runtime_error{"OpenCL runtime error: Cannot create program"};
    }

    cl_device_id dev_id = on_device_->get_device_id();

    status = clBuildProgram(program_, 1, &dev_id, NULL, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        valid_ = 0;
        throw std::runtime_error{"OpenCL runtime error: Cannot build program"};
    }

    kernel_ = clCreateKernel(program_, kernel_name_.c_str(), &status);
    if (status != CL_SUCCESS)
    {
        valid_ = 0;
        throw std::runtime_error{"OpenCL runtime error: Cannot create kernel of " + kernel_name_};
    }

    valid_ = valid_ | 2;
}

void task_impl::set_args(Args &&args)
{
    if (valid_ & 3 == 3)
    {
        cl_int status;
        size_t i = 0;
        for (auto const &arg : args)
        {
            status = clSetKernelArg(kernel_, i, arg.first, arg.second);
            ++i;
            if (status != CL_SUCCESS)
            {
                valid_ = 0;
                throw std::runtime_error{"OpenCL runtime error: Cannot set argument"};
            }
        }
        valid_ = valid_ | 4;
    }
    else
    {
        throw std::runtime_error{"Task need to be compiled first"};
    }
}

bool task_impl::is_valid_parallel_size(size_t dim, size_t global_size[], size_t local_size[])
{
    for (size_t i = 0; i < dim; ++i)
    {
        if (global_size[i] % local_size[i] != 0)
        {
            return false;
        }
    }
    return true;
}

int task_impl::get_compute_unit_usage(size_t dim, size_t global_size[], size_t local_size[])
{
    int cu_usage = 1;
    for (size_t i = 0; i < dim; ++i)
    {
        cu_usage = cu_usage * (global_size[i] / local_size[i]);
    }
    return cu_usage;
}

void task_impl::exec(size_t dim, size_t global_size[], size_t local_size[])
{
    if (valid_ & 7 == 7)
    {
        if (is_valid_parallel_size(dim, global_size, local_size))
        {
            cl_int status;
            cl_event event;

            size_t compute_unit_usage = get_compute_unit_usage(dim, global_size, local_size);

            on_device_->compute_unit_usage_increment(compute_unit_usage);

            clEnqueueNDRangeKernel(on_device_->get_command_queue(), kernel_, dim, NULL, global_size, local_size, 0, NULL, &event);
            if (status != CL_SUCCESS)
            {
                valid_ = 0;
                throw std::runtime_error{"OpenCL runtime error: Cannot execute kernel"};
            }

            status = clWaitForEvents(1, &event);
            if (status != CL_SUCCESS)
            {
                valid_ = 0;
                throw std::runtime_error{"OpenCL runtime error: Cannot execute kernel"};
            }

            on_device_->compute_unit_usage_increment(-compute_unit_usage);
        }
        else
        {
            throw std::runtime_error{"Global size needs to be multiple of local size."};
        }
    }
    else
    {
        throw std::runtime_error{"Task need to be compiled and arguments need to be set."};
    }
}

} // namespace opencle