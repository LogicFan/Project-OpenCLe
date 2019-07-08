#include <CL/cl.h>
#include <cassert>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "../device/device_impl.hpp"
#include "../memory/global_ptr_impl.hpp"
#include "../task/task_impl.hpp"
#include "../util/core_def.hpp"

// OpenCL C code
std::string programSource = "__kernel \n"
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
    int *expect = new int[element_num];

    for (int i = 0; i < element_num; ++i)
    {
        input_1[i] = i;
        input_2[i] = 4 * i;
        expect[i] = 5 * i;
    }

    std::function<void(void const *)> deleter = [](void const *p) { delete[] static_cast<int const *>(p); };

    opencle::global_ptr_impl input_1_gp_temp{static_cast<void *>(input_1), element_num * sizeof(int), deleter, true};
    opencle::global_ptr_impl input_2_gp_temp{static_cast<void *>(input_2), element_num * sizeof(int), deleter, true};
    opencle::global_ptr_impl output_gp{element_num * sizeof(int), false};

    opencle::global_ptr_impl input_1_gp{std::move(input_1_gp_temp)};
    opencle::global_ptr_impl input_2_gp{input_2_gp_temp.clone()};

    assert(output_gp.size() == element_num * sizeof(int));
    assert(output_gp.is_allocated() == false);
    assert(input_2_gp.is_allocated() == true);

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
    cl_mem input_1_buf = input_1_gp.to_device(&dev_impl);
    cl_mem input_2_buf = input_2_gp.to_device(&dev_impl);
    cl_mem output_buf = output_gp.to_device(&dev_impl);

    opencle::task_impl vec_add_task{programSource, "vecadd"};

    vec_add_task.compile(&dev_impl);

    std::vector<std::pair<size_t, void *>> args{
        std::pair<size_t, void *>{sizeof(cl_mem), static_cast<void *>(&input_1_buf)}, 
        std::pair<size_t, void *>{sizeof(cl_mem), static_cast<void *>(&input_2_buf)}, 
        std::pair<size_t, void *>{sizeof(cl_mem), static_cast<void *>(&output_buf)}};

    vec_add_task.set_args(std::move(args));

    // enqueue kernel
    size_t index_space_size[1];
    size_t work_group_size[1];
    index_space_size[0] = element_num;
    work_group_size[0] = 4;

    vec_add_task.exec(1, index_space_size, work_group_size);

    int *output = reinterpret_cast<int *>(output_gp.release());

    for (int i = 0; i < element_num; ++i)
    {
        std::cout << output[i] << ", ";
        assert(expect[i] == output[i]);
    }
    std::cout << std::endl;

    // free resources

    delete[] expect;
}
} // namespace opencle_test

int main(int argc, char *argv[])
{
    opencle_test::test();
    std::cout << "========== task_impl test pass ==========" << std::endl;
    return 0;
}