/** This is a sample code of OpenCL code, used
 * for test the configuration of OpenCL */

#include <CL/cl.h>
#include <cassert>
#include <stdexcept>
#include <stdlib.h>
#include <string>

#include "../util/logger/logger.hpp"
#include "device_impl.hpp"

// OpenCL C code
const char *programSource =
    "__kernel \n"
    "void vecadd(__global int *A, __global int *B, __global int *C) \n"
    "{ \n"
    "   int idx = get_global_id(0); \n"
    "   C[idx] = A[idx] + B[idx]; \n"
    "} \n";

int main(int argc, char *argv[]) { opencle_test::device_impl_test(); }

namespace opencle_test {

void device_impl_test() {
    constexpr int element_num = 16;

    // allocate space and initialize for input and output host data
    int *input_1 = new int[element_num];
    int *input_2 = new int[element_num];
    int *output = new int[element_num];
    int *expect = new int[element_num];

    for (int i = 0; i < element_num; ++i) {
        input_1[i] = i;
        input_2[i] = 4 * i;
        expect[i] = 5 * i;
    }

    cl_int status;

    /** Test for get_device_list() method and constructor */
    std::vector<opencle::device> device_list =
        opencle::device_impl::get_device_list();

    for (auto const &dev : device_list) {
        logger(dev << std::endl);
    }

    cl_device_id device = (device_list[0])->device_;
    cl_context context = (device_list[0])->context_;
    cl_command_queue cmd_queue = (device_list[0])->cmd_queue_;

    // Test for operator bool
    if(*(device_list[0])) {
        logger("Device returns true" << std::endl);
    } else {
        logger("Device returns false" << std::endl);
    }

    // initialize and allocate device side memory
    cl_mem input_1_buf = clCreateBuffer(
        context, CL_MEM_WRITE_ONLY, element_num * sizeof(int), NULL, &status);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot create memory buffer"};
    }
    cl_mem input_2_buf = clCreateBuffer(
        context, CL_MEM_WRITE_ONLY, element_num * sizeof(int), NULL, &status);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot create memory buffer"};
    }
    cl_mem output_buf = clCreateBuffer(
        context, CL_MEM_READ_ONLY, element_num * sizeof(int), NULL, &status);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot create memory buffer"};
    }

    status =
        clEnqueueWriteBuffer(cmd_queue, input_1_buf, CL_TRUE, 0,
                             element_num * sizeof(int), input_1, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot enqueue data"};
    }
    status =
        clEnqueueWriteBuffer(cmd_queue, input_2_buf, CL_TRUE, 0,
                             element_num * sizeof(int), input_2, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot enqueue data"};
    }

    // initialize kernel
    cl_program program =
        clCreateProgramWithSource(context, 1, &programSource, NULL, &status);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot create OpenCL program"};
    }
    status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot compile OpenCL program"};
    }
    cl_kernel kernel = clCreateKernel(program, "vecadd", &status);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot create kernel"};
    }

    // set arguments
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_1_buf);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot set argument"};
    }
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_2_buf);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot set argument"};
    }
    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_buf);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot set argument"};
    }

    // enqueue kernel
    size_t index_space_size[1], work_group_size[1];
    index_space_size[0] = element_num;
    work_group_size[0] = 4;

    status =
        clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, index_space_size,
                               work_group_size, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot enqueue kernel"};
    }

    // read result
    status =
        clEnqueueReadBuffer(cmd_queue, output_buf, CL_TRUE, 0,
                            element_num * sizeof(int), output, 0, NULL, NULL);

    logger("The output array is ");
    for (int i = 0; i < element_num; ++i) {
        logger_continue(output[i] << " ");
        assert(expect[i] == output[i]);
    }
   logger_continue(std::endl);

    // free resources
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(input_1_buf);
    clReleaseMemObject(input_1_buf);
    clReleaseMemObject(output_buf);
    clReleaseCommandQueue(cmd_queue);
    clReleaseContext(context);

    delete[] input_1;
    delete[] input_2;
    delete[] output;
}

} // namespace opencle_test