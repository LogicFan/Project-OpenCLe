#pragma once

#include <CL/cl.h>
#include <functional>

namespace opencle {
class device;

class global_ptr_impl final {
  private:
    size_t size_;

    /** host-side memory */
    void *host_ptr_;
    std::function<void(void *)> deleter_;

    /** device-side memory */
    mutable cl_mem device_ptr_;
    mutable device const *on_device_;

    /** a memory is write only if write_only_ is true; a memory is read only if
     * it is const */
    bool write_only_;

  public:

    /** write only memory constructor */
    global_ptr_impl(size_t size);

    /** general memory constructor */
    global_ptr_impl(void *ptr, size_t size,
                    std::function<void(void *)> deleter);

    global_ptr_impl(global_ptr_impl const &rhs) = delete;

    /** move constructor */
    global_ptr_impl(global_ptr_impl &&rhs);

    /** destructor */
    ~global_ptr_impl();

    global_ptr_impl &operator=(global_ptr_impl const &rhs) = delete;

    /** move assignment operator */
    global_ptr_impl &operator=(global_ptr_impl &&rhs);


    /** enqueue memory to host-side and return
     * the address. This will cause write_only_ to
     * be false. */
    void *get();
    void const *get() const;

    /** similar to get, but also move ownership */
    void *release();

    /** return a deep copy */
    global_ptr_impl clone() const;

    /** return true if memory is not empty. */
    operator bool() const;

    /** return the size */
    size_t size();

    /** For device **/

    /** Enqueue memory to device-side dev **/
    cl_mem to_device(device const &dev);
    cl_mem to_device(device const &dev) const;
};
} // namespace opencle