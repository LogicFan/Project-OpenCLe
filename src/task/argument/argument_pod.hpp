#include <type_traits>

#include "argument_base.hpp"

namespace opencle
{
template <typename T>
class argument_pod;

template <typename T>
class argument_pod<std::enable_if_t<std::is_pod_v<T>, T>> : public argument_base
{
    std::unique_ptr<T> value_;

public:
    argument_pod(T const &value)
        : argument_base{}, value_{value}
    {
        logger("argument_pod(T const &), create " << this);
    }
    argument_pod(argument_pod &&rhs)
        : argument_base{std::move(rhs)}, value_{std::move(rhs.value_)}
    {
        logger("argument_pod(argument_pod &&), create " << this);
    }
    ~argument_pod() override
    {
        logger("~argument_pod(), destory " << this);
    }

    argument_pod &operator=(argument_pod &&rhs)
    {
        logger("operator=(argument &&), " << this << " from " << &rhs);
        argument_base::operator=(std::move(rhs));
        value_ = std::move(rhs.value_);
    }

    void init(device_impl &impl) override
    {
        logger("init(device_impl &)");
    }
    size_t get_size() override
    {
        logger("get_size()");
        return sizeof(T);
    }
    void *get_pointer() override
    {
        logger("get_pointer()");
        return value_.get();
    }
};

} // namespace opencle