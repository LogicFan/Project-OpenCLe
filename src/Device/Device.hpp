#ifndef DEVICE_DEVICE_HPP
#define DEVICE_DEVICE_HPP

#include <CL/cl.h>

namespace opencle{
class Kernal;
class Device final
{
  private:
    bool valid_;
    cl_device_id device_id_;

  public:
    Device(cl_device_id device_id) : valid_(true), device_id_(device_id)
    {
        valid_ = true;
        // create context
        // enable command_queue
        // find calculation unit number
#ifndef NDEBUG
        size_t size;
        clGetDeviceInfo(device_id_, CL_DEVICE_NAME, 0, NULL, &size);
        char *val = new char[size];
        clGetDeviceInfo(device_id_, CL_DEVICE_NAME, size, val, NULL);
        std::cout << "Fetching Device : " << val << std::endl;
#endif
    }
    
    void compile(Kernal const &k); // compile and allocate memory
    void push(Kernal const &k); // exectuable program

};
}

#endif