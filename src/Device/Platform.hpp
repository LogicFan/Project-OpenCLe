#ifndef DEVICE_PLATFORM_HPP
#define DEVICE_PLATFORM_HPP

#include <vector>
#include <CL/cl.h>
#include <assert.h>
#include <iostream>
#include "../coredef.hpp"

using std::vector;

namespace opencle
{

// class Device;

class Platform final
{
  private:
    cl_platform_id platform_id_;

  public:
    Platform(cl_platform_id platform_id);

    // vector<Device> getDeviceList();
};

vector<Platform> getPlatformList();

} // namespace opencle

#endif