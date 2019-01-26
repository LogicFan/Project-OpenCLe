#ifndef DEVICE_PLATFORM_HPP
#define DEVICE_PLATFORM_HPP

#include <vector>
#include <list>
#include <CL/cl.h>
#include <assert.h>
#include <iostream>
#include "../coredef.hpp"

using std::list;
using std::vector;

namespace opencle
{

class Device;

class Platform final
{
private:
  bool valid_;
  cl_platform_id platform_id_;

  list<Device> getDeviceListDefault();

public:
  Platform(cl_platform_id const &platform_id);

  list<Device> getDeviceList();
};

vector<Platform> getPlatformList();

} // namespace opencle

#endif