#pragma once

#include <CL/cl.h>
#include <functional>

namespace opencle {
class device;

class global_ptr_impl final {
    private:
    /** Host-side memory management **/
    void *host_ptr_;
    size_t size_;
    std::function<void(void *)> deleter_;

    /** Device-side memory management **/
    mutable cl_mem device_ptr_;
    mutable device const *on_device_;

    /** Indicate if this is a write-only memory.
     * This is true if object is initialized by 
     * global_ptr_impl(size_t) and never called 
     * get() method **/
    bool write_only_;

    /** If the object is const object, then it 
     * is a read_only memory **/

    public:
    /** For global_ptr **/

    /** Create a write only memory **/
    global_ptr_impl(size_t size);

    /** Create a general memory **/
    global_ptr_impl(void *ptr, size_t size, std::function<void(void *)> deleter);
    
    /** Can not be copied **/
    global_ptr_impl(global_ptr_impl const &rhs) = delete;
    
    /** Move constructor **/
    global_ptr_impl(global_ptr_impl &&rhs);
    
    /** Destructor **/
    ~global_ptr_impl();

    /** Can not be copied **/
    global_ptr_impl &operator=(global_ptr_impl const &rhs) = delete;
    
    /** Move assignment operator **/
    global_ptr_impl &operator=(global_ptr_impl &&rhs);
    
    /** Enqueue memory to host-side and then return 
     * the address. This will cause write_only_ to 
     * be false. **/
    void *get();

    /** Return the host-side memory address **/
    void *get() const;

    /** Similar to get, but also move ownership **/
    void *release();

    /** Return true if memory is not empty. **/
    operator bool() const;

    /** For device **/

    /** Enqueue memory to device-side dev **/
    cl_mem to_device(device const &dev);
    cl_mem to_device(device const &dev) const;
};
}