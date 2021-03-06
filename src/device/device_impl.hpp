#pragma once

#include <CL/cl.h>
#include <atomic>
#include <memory>
#include <vector>

#include "../util/core_def.hpp"

namespace opencle
{
class device_impl;

class device_impl final
{
private:
    cl_device_id device_;
    cl_context context_;
    cl_command_queue cmd_queue_;

    size_t cu_total_;

    mutable std::atomic<bool> valid_;
    std::atomic<size_t> cu_used_;

public:
    device_impl(cl_device_id const &dev_id);
    device_impl(cl_device_id const &dev_id, cl_context const &context, cl_command_queue const &cmd_q);
    device_impl(device_impl const &rhs) = delete;
    device_impl(device_impl &&rhs) = delete;
    ~device_impl();

    device_impl &operator=(device_impl const &rhs) = delete;
    device_impl &operator=(device_impl &&rhs) = delete;
    bool operator<(device_impl const &rhs) const;
    operator bool() const;

    cl_device_id get_device_id() const;
    cl_context get_context() const;
    cl_command_queue get_command_queue() const;
    int get_compute_unit_available() const; 

    void compute_unit_usage_increment(int offset);

    friend std::ostream &operator<<(std::ostream &out, device_impl const &dev_impl);
};
} // namespace opencle
