#pragma once

#include "../util/core_def.hpp"

#include <CL/cl.h>
#include <list>
#include <memory>
#include <atomic>

namespace opencle {
class device final {
  private:
    cl_device_id device_;
    cl_context context_;
    cl_command_queue cmd_queue_;

    size_t cu_total;

    mutable std::atomic<bool> valid_;
    mutable std::atomic<size_t> cu_used;

    device(cl_device_id const &dev, cl_context const &con,
           cl_command_queue const &cmd);
    device(cl_device_id const &dev);

  public:
    device(device const &rhs) = delete;
    device(device &&rhs);

    ~device();

    device &operator=(device const &rhs) = delete;
    device &operator=(device &&rhs);
    bool operator<(device const &rhs) const;
    operator bool();

    static std::list<std::unique_ptr<const device>> get_device_list();

    // void exec(task &&task);
};
} // namespace opencle