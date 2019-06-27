#pragma once

#include <memory>
#include <vector>

namespace opencle
{
class device;
class device_impl;

enum class device_type
{
    CPU,
    GPU,
    ACCELERATOR,
    DEFAULT,
    ALL
};

class device final
{
private:
    std::unique_ptr<device_impl> impl_;
    static std::vector<device> device_list_;

    device(std::unique_ptr<device_impl> &&dev_impl);

public:
    static const std::vector<device> get_device_list(device_type type);

    device(device const &rhs);
    device(device &&rhs);
    ~device();

    device &operator=(device const &rhs);
    device &operator=(device &&rhs);

    // exec(task);
};
} // namespace opencle