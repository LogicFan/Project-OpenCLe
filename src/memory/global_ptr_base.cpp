#include "global_ptr_base.hpp"
#include "global_ptr_impl.hpp"

namespace opencle
{
global_ptr_base::global_ptr_base(std::unique_ptr<global_ptr_impl> &&impl)
    : impl_{std::move(impl)}
{
}

global_ptr_base::global_ptr_base(size_t size, bool read_only = false)
    : impl_{std::make_unique<global_ptr_impl>(size, read_only)}
{
}

global_ptr_base::global_ptr_base(void *ptr, size_t size, Deleter deleter, bool read_only = false)
    : impl_{std::make_unique<global_ptr_impl>(ptr, size, deleter, read_only)}
{
}

global_ptr_base::global_ptr_base(global_ptr_base &&rhs)
    : impl_{std::move(rhs.impl_)}
{
}

global_ptr_base &global_ptr_base::operator=(global_ptr_base &&rhs)
{
    impl_ = std::move(rhs.impl_);
}

void *global_ptr_base::impl_get()
{
    return impl_->get();
}

void *global_ptr_base::impl_release()
{
    return impl_->release();
}

global_ptr_base global_ptr_base::clone() const
{
    return global_ptr_base{impl_->clone()};
}

bool global_ptr_base::impl_bool() const
{
    return impl_->operator bool();
}

size_t global_ptr_base::impl_size() const
{
    return impl_->size();
}

bool global_ptr_base::impl_is_allocated() const
{
    return impl_->is_allocated();
}

} // namespace opencle