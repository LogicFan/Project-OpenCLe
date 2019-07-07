#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <mutex>

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

    static std::atomic<bool> is_device_list_created_;
    static std::vector<device> device_list_;
    static std::mutex device_list_mutex_;

    device(std::unique_ptr<device_impl> &&dev_impl);

public:
    static void create_device_list(device_type type = device_type::DEFAULT);
    static device const &get_top_device();
    static void sort_device_list();

    device(device const &rhs) = delete;
    device(device &&rhs);
    ~device();

    device &operator=(device const &rhs) = delete;
    device &operator=(device &&rhs);
    bool operator<(device const &rhs) const;

    int get_compute_unit_available() const;

    // for test purpose
    std::unique_ptr<device_impl> const &get_device_impl() const {
        return impl_;
    }

    // exec(task);
};
} // namespace opencle