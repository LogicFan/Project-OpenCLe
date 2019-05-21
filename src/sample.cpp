/** This is a sample code of OpenCL code, used
 * for test the configuration of OpenCL */

#include <CL/cl.h>
#include <cassert>
#include <stdexcept>
#include <stdlib.h>
#include <string>

#include "util/logger/logger.hpp"

// OpenCL C code
const std::string programSource{
    "__kernel \n"
    "void vecadd(__global int *A, __global int *B, __global int *C) \n"
    "{ \n"
    "   int idx = get_global_id(0); \n"
    "   C[idx] = A[idx] + B[idx]; \n"
    "} \n"};

int main(int argc, char *argv[]) {
    constexpr int element_num = 16;

    // allocate space and initialize for input and output host data
    int *input_1 = new int[element_num];
    int *input_2 = new int[element_num];
    int *output = new int[element_num];
    int *expect = new int[element_num];

    for (int i = 0; i < element_num; ++i) {
        input_1[i] = i;
        input_2[i] = i;
        expect[i] = 2 * i;
    }

    cl_int status;

    // initialize platform
    cl_platform_id platform;
    cl_uint platform_num;

    status = clGetPlatformIDs(0, NULL, &platform_num);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error:: Cannot initialize platform"};
    } else if (platform_num == 0) {
        throw std::out_of_range{"Cannot detect platforms"};
    } else {
        status = clGetPlatformIDs(1, &platform, NULL);
    }

    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error:: Cannot initialize platform"};
    }

    char platform_name[50];
    status =
        clGetPlatformInfo(platform, CL_PLATFORM_NAME, 50, platform_name, NULL);

    logger << "Platform is " << platform_name << std::endl;

    // initialize device
    cl_device_id device;
    cl_uint device_num;

    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &device_num);

    return 0;
}

#if 0

int main()
{
    cl_device_id device;
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num);
    if (status == CL_SUCCESS && num > 0)
    {
        status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
    }
    else
    {
        throw std::runtime_error{""};
    }

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);

    cl_command_queue cmdQueue =
        clCreateCommandQueueWithProperties(context, device, NULL, &status);

    // opencle::device dev;
    // dev.c_queue_ = cmdQueue;
    // dev.context_ = context;
    // dev.device_ = device;
    // dev.valid_ = true;

    // cl_mem bufA =
    //     clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, &status);
    // cl_mem bufB =
    //     clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, &status);
    // cl_mem bufC =
    //     clCreateBuffer(context, CL_MEM_WRITE_ONLY, datasize, NULL, &status);
    //
    // status = clEnqueueWriteBuffer(cmdQueue, bufA, CL_FALSE, 0, datasize, A,
    // 0,
    //                               NULL, NULL);
    // status = clEnqueueWriteBuffer(cmdQueue, bufB, CL_FALSE, 0, datasize, B,
    // 0,
    //                               NULL, NULL);

    // opencle::global_ptr_impl ptrA{A, datasize, free};
    // opencle::global_ptr_impl ptrB{B, datasize, free};
    // opencle::global_ptr_impl ptrC{datasize};

    cl_mem bufA =
        clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, &status);
    cl_mem bufB =
        clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, &status);
    cl_mem bufC =
        clCreateBuffer(context, CL_MEM_WRITE_ONLY, datasize, NULL, &status);

    status = clEnqueueWriteBuffer(cmdQueue, bufA, CL_TRUE, 0, datasize, A, 0,
                                  NULL, NULL);
    status = clEnqueueWriteBuffer(cmdQueue, bufB, CL_TRUE, 0, datasize, B, 0,
                                  NULL, NULL);

    // Create a program with source code
    cl_program program = clCreateProgramWithSource(
        context, 1, (const char **)&programSource, NULL, &status);
    // Build(compile) the program for the device
    status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    // Create the vector addition kernel
    cl_kernel kernel = clCreateKernel(program, "vecadd", &status);
    // Set the kernel arguments

    // std::cout << "A: ";
    // cl_mem bufA = ptrA.to_device(dev);
    // std::cout << "B: ";
    // cl_mem bufB = ptrB.to_device(dev);
    // std::cout << "C: ";
    // cl_mem bufC = ptrC.to_device(dev);

    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufB);
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufC);

    // Define an incde space of work-items for execution
    // A work-group size is not required, but can be used.
    size_t indexSpaceSize[1], workGroupSize[1];
    // There are 'elements' work-items
    indexSpaceSize[0] = elements;
    workGroupSize[0] = 4;
    // Execute the kernel
    status = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, indexSpaceSize,
                                    workGroupSize, 0, NULL, NULL);
    // Read the device output buffer to the host output array
    // std::cout << "C: ";
    // int *C = static_cast<int *>(ptrC.get());

    status = clEnqueueReadBuffer(cmdQueue, bufC, CL_TRUE, 0, datasize, C, 0,
                                 NULL, NULL);

    for (int i = 0; i < elements; ++i)
    {
        std::cout << *(C + i) << std::endl;
    }

    // Free OpenCL resouces
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufA);
    clReleaseMemObject(bufB);
    clReleaseMemObject(bufC);
    clReleaseContext(context);
    // free host resouces
    free(A);
    free(B);
    free(C);

    return 0;
}

#endif