#pragma once

#include <string>
#include <vector>
#include <memory>

namespace opencle {
class task_impl;
class device_impl;

class task_impl final {
private:
    using Args = std::vector<size_t, std::unique_ptr<void>>;

    std::string source_;
    Args args_;
public:
    task_impl(std::string const &source, Args const &args);
    task_impl(task_impl const &rhs) = delete;
    task_impl(task_impl &&rhs) = delete;
    ~task_impl();

    size_t get_computate_unit_usage();

    void compile(device_impl const &dev_impl);
    void run();
};
}
