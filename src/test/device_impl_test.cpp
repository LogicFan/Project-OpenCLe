#include <CL/cl.h>
#include <cassert>
#include <stdexcept>
#include <stdlib.h>
#include <string>

#include "../util/core_def.hpp"
#include "../util/logger/logger.hpp"
#include "../device/device_impl.hpp"

// OpenCL C code
const char *programSource = "__kernel \n"
                            "void vecadd(__global int *A, __global int *B, __global int *C) \n"
                            "{ \n"
                            "   int idx = get_global_id(0); \n"
                            "   C[idx] = A[idx] + B[idx]; \n"
                            "} \n";

namespace opencle_test
{
void test()
{
    constexpr int element_num = 16;

    // allocate space and initialize for input and output host data
    int *input_1 = new int[element_num];
    int *input_2 = new int[element_num];
    int *output = new int[element_num];
    int *expect = new int[element_num];

    for (int i = 0; i < element_num; ++i)
    {
        input_1[i] = i;
        input_2[i] = 4 * i;
        expect[i] = 5 * i;
    }

    cl_int status;

    // initialize platform
    cl_platform_id platform;
    cl_uint platform_num;

    status = clGetPlatformIDs(0, NULL, &platform_num);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize platform"};
    }
    else if (platform_num == 0)
    {
        throw std::out_of_range{"Cannot detect platforms"};
    }
    else
    {
        status = clGetPlatformIDs(1, &platform, NULL);
    }

    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize platform"};
    }

    char platform_name[50];
    status = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 50, platform_name, NULL);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot get platform info"};
    }

    logger("Platform is " << platform_name);

    // initialize device
    cl_device_id device;
    cl_uint device_num;

    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &device_num);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize device"};
    }
    else if (device_num == 0)
    {
        throw std::out_of_range{"Cannot detect devices"};
    }
    else
    {
        status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
    }

    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize device"};
    }

    char device_name[50];
    status = clGetDeviceInfo(device, CL_DEVICE_NAME, 50, device_name, NULL);

    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot get device info"};
    }

    opencle::device_impl dev_impl{device};

    // initialize and allocate device side memory
    cl_mem input_1_buf = clCreateBuffer(dev_impl.get_context(), CL_MEM_READ_ONLY, element_num * sizeof(int), NULL, &status);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot create memory buffer"};
    }
    cl_mem input_2_buf = clCreateBuffer(dev_impl.get_context(), CL_MEM_READ_ONLY, element_num * sizeof(int), NULL, &status);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot create memory buffer"};
    }
    cl_mem output_buf = clCreateBuffer(dev_impl.get_context(), CL_MEM_WRITE_ONLY, element_num * sizeof(int), NULL, &status);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot create memory buffer"};
    }

    status = clEnqueueWriteBuffer(dev_impl.get_command_queue(), input_1_buf, CL_TRUE, 0, element_num * sizeof(int), input_1, 0, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot enqueue data"};
    }
    status = clEnqueueWriteBuffer(dev_impl.get_command_queue(), input_2_buf, CL_TRUE, 0, element_num * sizeof(int), input_2, 0, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot enqueue data"};
    }

    // initialize kernel
    cl_program program = clCreateProgramWithSource(dev_impl.get_context(), 1, &programSource, NULL, &status);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot create OpenCL program"};
    }
    status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot compile OpenCL program"};
    }
    cl_kernel kernel = clCreateKernel(program, "vecadd", &status);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot create kernel"};
    }

    // set arguments
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_1_buf);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot set argument"};
    }
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_2_buf);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot set argument"};
    }
    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_buf);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot set argument"};
    }

    // enqueue kernel
    size_t index_space_size[1], work_group_size[1];
    index_space_size[0] = element_num;
    work_group_size[0] = 4;

    status = clEnqueueNDRangeKernel(dev_impl.get_command_queue(), kernel, 1, NULL, index_space_size, work_group_size, 0, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        throw std::runtime_error{"OpenCL runtime error: Cannot enqueue kernel"};
    }

    // read result
    status = clEnqueueReadBuffer(dev_impl.get_command_queue(), output_buf, CL_TRUE, 0, element_num * sizeof(int), output, 0, NULL, NULL);

    for (int i = 0; i < element_num; ++i)
    {
        std::cout << output[i] << ", ";
        assert(expect[i] == output[i]);
    }
    std::cout << std::endl;

    size_t cu = dev_impl.get_compute_unit_available();
    std::cout << cu << std::endl;
    dev_impl.compute_unit_usage_increment(5);
    std::cout << dev_impl.get_compute_unit_available() << std::endl;
    assert(dev_impl.get_compute_unit_available() == cu - 5);

    // free resources
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(input_1_buf);
    clReleaseMemObject(input_1_buf);
    clReleaseMemObject(output_buf);

    delete[] input_1;
    delete[] input_2;
    delete[] output;
    delete[] expect;
}
} // namespace opencle_test

int main(int argc, char *argv[])
{
    opencle_test::test();
    std::cout << "========== device_impl test pass ==========" << std::endl;
    return 0;
}