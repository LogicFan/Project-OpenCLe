#include "Platform.hpp"

namespace opencle
{

Platform::Platform(cl_platform_id platform_id): platform_id_(platform_id)
{

#ifndef NDEBUG
    size_t size;
    clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 0, NULL, &size);
    char *val = new char[size];
    clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, size, val, NULL);
    std::cout << "Fetching platform : " << val << std::endl;
#endif
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

    cl_platform_id *platforms = new cl_platform_id[platformNum];

    status = clGetPlatformIDs(platformNum, platforms, NULL);
    if (status != CL_SUCCESS)
    {
        delete[] platforms;
        return {};
    }

    vector<Platform> platformList;

    for (int i = 0; i < platformNum; ++i)
    {
        platformList.push_back(Platform(platforms[i]));
    }

    delete[] platforms;
    return std::move(platformList);
}

} // namespace opencle
