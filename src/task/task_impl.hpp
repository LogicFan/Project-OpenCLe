#pragma once

#include <CL/cl.h>
#include <string>
#include <vector>
#include <memory>

#include "../util/core_def.hpp"

namespace opencle
{
class task_impl;
class device_impl;
class global_ptr_impl;

class task_impl final
{
private:
    using Args = std::vector<std::pair<size_t, void *>>;

    char valid_;

    std::string source_;

    cl_program program_;
    cl_kernel kernel_;
    device_impl *on_device_;

    static int get_compute_unit_usage(size_t dim, size_t global_size[], size_t local_size[]);
    static bool is_valid_parallel_size(size_t dim, size_t global_size[], size_t local_size[]); 

public:
    task_impl(std::string const &source);
    task_impl(task_impl const &rhs) = delete;
    task_impl(task_impl &&rhs) = delete;
    ~task_impl();

    task_impl &operator=(task_impl const &rhs) = delete;
    task_impl &operator=(task_impl &&rhs) = delete;
    operator bool();

    void compile(device_impl *dev_impl, std::string const &func_name);
    void set_args(Args &&args);
    void exec(size_t dim, size_t global_size[], size_t local_size[]);
};
} // namespace opencle
