#pragma once

#include <CL/cl.h>
#include <string>
#include <vector>
#include <memory>

namespace opencle
{
class task_impl;
class device_impl;
class global_ptr_impl;

class task_impl final
{
private:
    using Args = std::vector<size_t, std::unique_ptr<void>>;
    using Mems = std::vector<global_ptr_impl *>;

    bool valid_;

    std::string source_;
    Args args_;
    Mems mems_;
    size_t work_size_;

    cl_program program_;
    cl_kernel kernel_;
    device_impl const *on_device_;

public:
    task_impl(std::string const &source, size_t work_size);
    task_impl(task_impl const &rhs) = delete;
    task_impl(task_impl &&rhs) = delete;
    ~task_impl();

    void setArgs(Args args, Mems mems);

    void compile_and_allocate(device_impl const &dev_impl);
    void exec();
};
} // namespace opencle
