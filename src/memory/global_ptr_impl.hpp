#pragma once

#include <CL/cl.h>
#include <functional>

namespace opencle {
class device;

/** const indicate read_only, otherwise is read_write **/

class global_ptr_impl final {
  private:
    size_t size_;

    /** host-side memory */
    void *host_ptr_;
    std::function<void(void *)> deleter_;

    /** device-side memory */
    mutable cl_mem device_ptr_;
    mutable device const *on_device_;

  public:
    global_ptr_impl(size_t size);

    /* do not copy the data */
    global_ptr_impl(void *ptr, size_t size,
                    std::function<void(void *)> deleter);
    global_ptr_impl(global_ptr_impl const &rhs) = delete;
    global_ptr_impl(global_ptr_impl &&rhs);
    ~global_ptr_impl();

    global_ptr_impl &operator=(global_ptr_impl const &rhs) = delete;

    global_ptr_impl &operator=(global_ptr_impl &&rhs);

    void *get();
    void const *get() const;

    void *release();
    void const *release() const;

    global_ptr_impl clone() const;

    operator bool() const;
    size_t size();

    /** For device **/

    cl_mem to_device(device const &dev);
    cl_mem to_device(device const &dev) const;
};
} // namespace opencle