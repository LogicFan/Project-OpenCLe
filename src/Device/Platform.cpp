#include "Platform.hpp"
#include "Device.hpp"
#include "../coredef.hpp"
#include <memory>

namespace opencle
{

Platform::Platform(cl_platform_id platform_id) : valid_(true), platform_id_(platform_id)
{

#ifndef NDEBUG
    size_t size;
    clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, 0, NULL, &size);
    char *val = new char[size];
    clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, size, val, NULL);
    std::cout << "Fetching platform : " << val << std::endl;
#endif
}

list<Device> Platform::getDeviceList()
{
    size_t infoSize;
    cl_int status = CL_SUCCESS;
    status = clGetPlatformInfo(platform_id_, CL_PLATFORM_VENDOR, 0, NULL, &infoSize);

    if (status != CL_SUCCESS)
    {
        valid_ = false;
        return {};
    }

    char *info = new char[infoSize];

    status = clGetPlatformInfo(platform_id_, CL_PLATFORM_VENDOR, infoSize, info, NULL);

    if (status != CL_SUCCESS)
    {
        valid_ = false;
        return {};
    }

    if (strcmp(info, Vendor::Nvidia) == 0)
    {
        return getDeviceListDefault();
    }
    else if (strcmp(info, Vendor::AMD) == 0)
    {
        return getDeviceListDefault();
    }
    else // Default //
    {
        return getDeviceListDefault();
    }

    return {};
}

list<Device> Platform::getDeviceListDefault()
{
    cl_uint deviceNum;
    cl_int status = CL_SUCCESS;

    status = clGetDeviceIDs(platform_id_, CL_DEVICE_TYPE_ALL, 0, NULL, &deviceNum);
    if (status != CL_SUCCESS)
    {
        return {};
    }

    auto devices = std::make_unique<cl_device_id[]>(deviceNum);

    status = clGetDeviceIDs(platform_id_, CL_DEVICE_TYPE_ALL, deviceNum, devices.get(), NULL);
    if (status != CL_SUCCESS)
    {
        return {};
    }

    list<Device> deviceList;

    for(unsigned int i = 0; i < deviceNum; ++i) {
        deviceList.push_back(Device(devices[i]));
    }

    return deviceList;
}

vector<Platform> getPlatformList()
{
    cl_uint platformNum;
    cl_int status = CL_SUCCESS;

    status = clGetPlatformIDs(0, NULL, &platformNum);
    if (status != CL_SUCCESS)
    {
        return {};
    }

    auto platforms = std::make_unique<cl_platform_id[]>(platformNum);

    status = clGetPlatformIDs(platformNum, platforms.get(), NULL);
    if (status != CL_SUCCESS)
    {
        return {};
    }

    vector<Platform> platformList;

    for (unsigned int i = 0; i < platformNum; ++i)
    {
        platformList.push_back(Platform(platforms[i]));
    }

    return platformList;
}

} // namespace opencle
