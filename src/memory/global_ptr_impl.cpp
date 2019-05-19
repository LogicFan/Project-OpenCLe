#include "global_ptr_impl.hpp"

#define GLOBAL_PTR_IMPL_TEST

#ifndef GLOBAL_PTR_IMPL_TEST
#include "../device/device.hpp"

#else
#include <atomic>
#include <iostream>
namespace opencle {
struct device final {
    mutable std::atomic<bool> valid_;

    cl_device_id device_;
    cl_context context_;
    cl_command_queue c_queue_;

    const size_t cu_total_;
    mutable std::atomic<size_t> cu_used_;

    device()
        : valid_{true}, device_{nullptr}, context_{nullptr}, c_queue_{nullptr},
          cu_total_{0}, cu_used_{0} {}
};
} // namespace opencle
#endif

#include <assert.h>
#include <stdexcept>
#include <stdlib.h>

namespace opencle {
global_ptr_impl::global_ptr_impl(size_t size)
    : host_ptr_{nullptr}, size_{size}, deleter_{}, device_ptr_{nullptr},
      on_device_{nullptr}, write_only_{true} {
    return;
}

global_ptr_impl::global_ptr_impl(void *ptr, size_t size,
                                 std::function<void(void *)> deleter)
    : host_ptr_{ptr}, size_{size}, deleter_{deleter}, device_ptr_{nullptr},
      on_device_{nullptr}, write_only_{false} {
    return;
}

global_ptr_impl::global_ptr_impl(global_ptr_impl &&rhs)
    : host_ptr_{rhs.host_ptr_}, size_{rhs.size_}, deleter_{rhs.deleter_},
      device_ptr_{rhs.device_ptr_}, on_device_{rhs.on_device_},
      write_only_{rhs.write_only_} {
    rhs.host_ptr_ = nullptr;
    rhs.device_ptr_ = nullptr;
    rhs.on_device_ = nullptr;
}

global_ptr_impl::~global_ptr_impl() {
    if (this->host_ptr_) {
        this->deleter_(this->host_ptr_);
        this->host_ptr_ = nullptr;
    }

    if (this->device_ptr_) {
        clReleaseMemObject(this->device_ptr_);
        this->device_ptr_ = nullptr;
        this->on_device_ = nullptr;
    }
}

global_ptr_impl &global_ptr_impl::operator=(global_ptr_impl &&rhs) {
    this->~global_ptr_impl();

    assert(!this->device_ptr_);
    assert(!this->host_ptr_);
    assert(!this->on_device_);

    new (this) global_ptr_impl(std::move(rhs));
    return *this;
}

void *global_ptr_impl::get() {
    this->write_only_ = false;

    if (this->host_ptr_) {
        std::cout << "get, case A" << std::endl;
        // If the memory is on the host-side;
        return this->host_ptr_;
    } else if (this->device_ptr_) {
        std::cout << "get, case B" << std::endl;
        // If the memory is on the device-side;

        // Allocate memory on host-side;
        this->host_ptr_ = new char[size_];
        this->deleter_ = [](void *ptr) { delete[] static_cast<char *>(ptr); };

        // Enqueue memory to host-side;
        cl_int status = clEnqueueReadBuffer(
            this->on_device_->c_queue_, this->device_ptr_, CL_TRUE, 0,
            this->size_, this->host_ptr_, 0, NULL, NULL);

        // Clean device-side memory
        clReleaseMemObject(this->device_ptr_);
        this->device_ptr_ = nullptr;
        this->on_device_ = nullptr;

        if (status == CL_SUCCESS) {
            assert(this->host_ptr_);

            return this->host_ptr_;
        } else {
            this->on_device_->valid_ = false;
            throw std::runtime_error{
                "OpenCL runtime error: Can not enqueue memory buffer!"};
        }
    } else {
        std::cout << "get, case C" << std::endl;
        this->host_ptr_ = new char[size_];
        this->deleter_ = [](void *ptr) { delete[] static_cast<char *>(ptr); };

        return this->host_ptr_;
    }
}

void *global_ptr_impl::get() const {

    assert(write_only_ || this->host_ptr_);

    return this->host_ptr_;
}

void *global_ptr_impl::release() {
    void *ret = this->get();

    assert(ret);

    this->host_ptr_ = nullptr;
    this->device_ptr_ = nullptr;
    this->on_device_ = nullptr;

    return ret;
}

global_ptr_impl::operator bool() const {
    return this->host_ptr_ || this->device_ptr_;
}

cl_mem global_ptr_impl::to_device(device const &dev) {
    if (&dev == this->on_device_) {
        std::cout << "to_device, case A" << std::endl;

        assert(device_ptr_);
        // If the memory is on the device-side dev
        return device_ptr_;
    } else if (write_only_) {
        std::cout << "to_device, case B" << std::endl;

        assert(!host_ptr_);
        // If the memory is write only
        this->on_device_ = &dev;

        cl_int status;
        this->device_ptr_ =
            clCreateBuffer(this->on_device_->context_, CL_MEM_WRITE_ONLY,
                           this->size_, NULL, &status);

        if (status != CL_SUCCESS) {
            this->on_device_->valid_ = false;
            throw std::runtime_error{
                "OpenCL runtime error: Can not create memory buffer!"};
        }
        return device_ptr_;
    } else {
        std::cout << "to_device, case C" << std::endl;

        // If the memory is on the other device

        // Enqueue data to host-side
        this->get();

        // Enqueue data to dev
        this->on_device_ = &dev;
        cl_int status;
        this->device_ptr_ =
            clCreateBuffer(this->on_device_->context_, CL_MEM_READ_WRITE,
                           this->size_, NULL, &status);

        clEnqueueWriteBuffer(this->on_device_->c_queue_, this->device_ptr_,
                             CL_TRUE, 0, this->size_, this->host_ptr_, 0, NULL,
                             NULL);

        if (status != CL_SUCCESS) {
            this->on_device_->valid_ = false;
            throw std::runtime_error{
                "OpenCL runtime error: Can not enqueue memory buffer!"};
        }

        // Clean host-side;
        this->deleter_(this->host_ptr_);
        this->host_ptr_ = nullptr;
        return device_ptr_;
    }
    assert(false);
    return nullptr;
}

cl_mem global_ptr_impl::to_device(device const &dev) const {
    if (&dev == this->on_device_) {
        // If the memory is on the device-side dev
        return device_ptr_;
    } else if (!write_only_) {
        assert(!host_ptr_);
        // Otherwise there must be a copy on the host-side
        this->on_device_ = &dev;
        cl_int status;
        this->device_ptr_ =
            clCreateBuffer(this->on_device_->context_, CL_MEM_READ_ONLY,
                           this->size_, NULL, &status);

        clEnqueueWriteBuffer(this->on_device_->c_queue_, this->device_ptr_,
                             CL_TRUE, 0, this->size_, this->host_ptr_, 0, NULL,
                             NULL);

        if (status != CL_SUCCESS) {
            this->on_device_->valid_ = false;
            throw std::runtime_error{
                "OpenCL runtime error: Can not enqueue memory buffer!"};
        }
    } else {
        throw std::runtime_error{"OpenCL runtime error: A memory buffer can "
                                 "not be both write only and read only."};
    }
    assert(false);
    return nullptr;
}

} // namespace opencle

#ifdef GLOBAL_PTR_IMPL_TEST

#include <CL/cl.h>
#include <stdlib.h>

const char *programSource =
    "__kernel \n"
    "void vecadd(__global int *A, __global int *B, __global int *C) \n"
    "{ \n"
    " int idx = get_global_id(0); \n"
    " C[idx] = A[idx] + B[idx]; \n"
    "} \n";
int main() {
    constexpr int elements = 16;
    size_t datasize = sizeof(int) * elements;

    // Allocate space for input/output host data
    int *A = (int *)malloc(datasize); // Input array
    int *B = (int *)malloc(datasize); // Input array
    int *C = (int *)malloc(datasize); // Output array

    // Initialize the input data
    for (int i = 0; i < elements; i++) {
        A[i] = i;
        B[i] = i;
    }

    cl_int status;

    cl_platform_id platform;
    cl_uint num;
    status = clGetPlatformIDs(0, NULL, &num);
    if (status == CL_SUCCESS && num > 0) {
        status = clGetPlatformIDs(1, &platform, NULL);
    } else {
        throw std::runtime_error{""};
    }

    cl_device_id device;
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num);
    if (status == CL_SUCCESS && num > 0) {
        status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
    } else {
        throw std::runtime_error{""};
    }

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);

    cl_command_queue cmdQueue =
        clCreateCommandQueue(context, device, NULL, &status);

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

    for (int i = 0; i < elements; ++i) {
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