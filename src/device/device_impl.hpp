#pragma once

#include <CL/cl.h>
#include <atomic>
#include <memory>
#include <vector>

#include "../util/core_def.hpp"
#include "../util/logger/logger.hpp"

namespace opencle_test {
void device_impl_test();
};

namespace opencle {
class device_impl;

using device = std::unique_ptr<device_impl>;

class device_impl final {
    // private:
    cl_device_id device_;
    cl_context context_;
    cl_command_queue cmd_queue_;

    size_t cu_total;

    mutable std::atomic<bool> valid_;
    mutable std::atomic<size_t> cu_used;

    device_impl(cl_device_id const &dev, cl_context const &con,
                cl_command_queue const &cmd);
    device_impl(cl_device_id const &dev);

  public:
    device_impl(device_impl const &rhs) = delete;
    device_impl(device_impl &&rhs) = delete;

    ~device_impl();

    device_impl &operator=(device_impl const &rhs) = delete;
    device_impl &operator=(device_impl &&rhs) = delete;
    bool operator<(device_impl const &rhs) const;
    operator bool();

    static std::vector<device> get_device_list();

    // void exec(task &&task);

    friend std::ostream &operator<<(std::ostream &out, device_impl const &dev);

    friend void ::opencle_test::device_impl_test();
};

std::ostream &operator<<(std::ostream &out, device_impl const &dev);

std::ostream &operator<<(std::ostream &out, device const &dev);
} // namespace opencle