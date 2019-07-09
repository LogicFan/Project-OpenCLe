#include <memory>
#include <functional>

namespace opencle
{
class global_ptr_impl;

class global_ptr_base
{
private:
    using Deleter = std::function<void(void const *)>;

    std::unique_ptr<global_ptr_impl> impl_;

    global_ptr_base(std::unique_ptr<global_ptr_impl> &&impl);

protected:
    global_ptr_base(size_t size, bool read_only = false);
    global_ptr_base(void *ptr, size_t size, Deleter deleter, bool read_only = false);
    global_ptr_base(global_ptr_base &&rhs);
    virtual ~global_ptr_base();

    global_ptr_base &operator=(global_ptr_base &&rhs);

    void *impl_get();
    void *impl_release();

    global_ptr_base clone() const;

    bool impl_bool() const;
    size_t impl_size() const;
    bool impl_is_allocated() const;
};
} // namespace opencle