#include <cstddef>

#include "../../util/logger/logger.hpp"

namespace opencle
{
class device_impl;

class argument_base
{
public:
    argument_base()
    {
        logger("argument_base(), create " << this);
    }
    argument_base(argument_base &&rhs)
    {
        logger("argument_base(argument_base &&), create " << this << " from " << &rhs);
    }
    virtual ~argument_base()
    {
        logger("~argument_base(), destory " << this);
    }

    argument_base &operator=(argument_base &&rhs)
    {
        logger("operator=(argument_base &&), " << this << " from " << &rhs);
    }

    virtual void init(device_impl &impl) = 0;
    virtual size_t get_size() = 0;
    virtual void *get_pointer() = 0;
};
} // namespace opencle
