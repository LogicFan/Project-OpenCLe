#define NDEBUG

#pragma once

#include <CL/cl.h>
#include <functional>
#include <memory>

#include "../util/core_def.hpp"

namespace opencle
{

class global_ptr_impl;
class device_impl;

class global_ptr_impl final
{
private:
    using Deleter = std::function<void(void const *)>;

    mutable bool valid_;
    size_t size_;
    bool read_only_;

    void *host_ptr_;
    Deleter deleter_;

    cl_mem device_ptr_;
    device_impl const *on_device_;
    
    void *get_read_write();
    void *get_read_only();

    cl_mem to_device_read_write(device_impl const *dev);
    cl_mem to_device_read_only(device_impl const *dev);

public:
    global_ptr_impl(size_t size, bool read_only = false);
    global_ptr_impl(void *ptr, size_t size, Deleter deleter, bool read_only = false);
    global_ptr_impl(global_ptr_impl const &rhs) = delete;
    global_ptr_impl(global_ptr_impl &&rhs) = delete;
    ~global_ptr_impl();

    global_ptr_impl &operator=(global_ptr_impl const &rhs) = delete;
    global_ptr_impl &operator=(global_ptr_impl &&rhs) = delete;

    void *get();
    void *release();

    std::unique_ptr<global_ptr_impl> clone() const;

    operator bool() const;

    size_t size() const;
    bool is_allocated() const;

    cl_mem to_device(device_impl const *dev);
};
} // namespace opencle