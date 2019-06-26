#include <CL/cl.h>
#include <cassert>
#include <stdexcept>
#include <stdlib.h>
#include <string>

#include "../util/logger/logger.hpp"
#include "../device/device_impl.hpp"
#include "global_ptr.hpp"

// OpenCL C code
const char *programSource =
    "__kernel \n"
    "void vecadd(__global int *A, __global int *B, __global int *C) \n"
    "{ \n"
    "   int idx = get_global_id(0); \n"
    "   C[idx] = A[idx] + B[idx]; \n"
    "} \n";

namespace opencle_test {
    void basic_test();
}

int main(int argc, char *argv[]) {
    opencle_test::basic_test();
    opencle_test::test();
    std::cout << "========== global_ptr_impl test pass ==========" << std::endl;
}

using namespace opencle;

namespace opencle_test {

using namespace opencle;

void test() {

}

// void test() {
//     constexpr int element_num = 16;

//     // allocate space and initialize for input and output host data
//     std::vector<int> input_1_vec;
//     std::vector<int> input_2_vec;
//     std::vector<int> expect_vec;
//     for(int i = 0; i < element_num; ++i) {
//         input_1_vec.push_back(i);
//         input_2_vec.push_back(4 * i);
//         expect_vec.push_back(5 * i);
//     }

//     global_ptr<const int[]> input_1{input_1_vec};
//     global_ptr<const int[]> input_2{input_2_vec};
//     global_ptr<const int[]> expect{expect_vec};
//     global_ptr<int[]> output(element_num);

//     cl_int status;

//     /** Test for get_device_list() method and constructor */
//     std::vector<device> device_list =
//         opencle::device_impl::get_device_list();

//     for (auto const &dev : device_list) {
//         logger(dev << std::endl);
//     }

//     device &dev = device_list[0];

//     cl_device_id device = dev->device_;
//     cl_context context = dev->context_;
//     cl_command_queue cmd_queue = dev->cmd_queue_;

//     cl_mem input_1_buf = dev->synchronize(input_1);
//     cl_mem input_2_buf = dev->synchronize(input_2);
//     cl_mem output_buf = dev->synchronize(output);

//     // initialize kernel
//     cl_program program =
//         clCreateProgramWithSource(context, 1, &programSource, NULL, &status);
//     if (status != CL_SUCCESS) {
//         throw std::runtime_error{
//             "OpenCL runtime error: Cannot create OpenCL program"};
//     }
//     status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
//     if (status != CL_SUCCESS) {
//         throw std::runtime_error{
//             "OpenCL runtime error: Cannot compile OpenCL program"};
//     }
//     cl_kernel kernel = clCreateKernel(program, "vecadd", &status);
//     if (status != CL_SUCCESS) {
//         throw std::runtime_error{"OpenCL runtime error: Cannot create kernel"};
//     }

//     // set arguments
//     status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_1_buf);
//     if (status != CL_SUCCESS) {
//         throw std::runtime_error{"OpenCL runtime error: Cannot set argument"};
//     }
//     status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_2_buf);
//     if (status != CL_SUCCESS) {
//         throw std::runtime_error{"OpenCL runtime error: Cannot set argument"};
//     }
//     status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_buf);
//     if (status != CL_SUCCESS) {
//         throw std::runtime_error{"OpenCL runtime error: Cannot set argument"};
//     }

//     // enqueue kernel
//     size_t index_space_size[1], work_group_size[1];
//     index_space_size[0] = element_num;
//     work_group_size[0] = 4;

//     status =
//         clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, index_space_size,
//                                work_group_size, 0, NULL, NULL);
//     if (status != CL_SUCCESS) {
//         throw std::runtime_error{"OpenCL runtime error: Cannot enqueue kernel"};
//     }

//     logger("The output array is ");
//     for (int i = 0; i < element_num; ++i) {
//         logger(output[i]);
//         assert(expect[i] == output[i]);
//     }

//     // free resources
//     clReleaseKernel(kernel);
//     clReleaseProgram(program);
// }

void basic_test() {
    // test for ctor
    global_ptr<const int[]> g1;
    std::cout << g1.size() << std::endl;

    global_ptr<const int[]> g2(10);
    std::cout << g2.size() << std::endl;

    std::cout << "g3: ";
    std::unique_ptr<int[]> u3 = std::make_unique<int[]>(20);
    for (int i = 0; i < 20; ++i) {
        u3[i] = i * 2;
    }
    global_ptr<int[]> g3{std::move(u3), 20};
    for (int i = 0; i < 20; ++i) {
        std::cout << g3[i] << ", ";
        assert(g3[i] == i * 2);
    }
    std::cout << std::endl;
    std::cout << g3.size() << std::endl;

    std::cout << "g4: ";
    int *p4 = new int[20];
    for (int i = 0; i < 20; ++i) {
        p4[i] = i * 3;
    }
    global_ptr<const int[]> g4(p4, 20);
    delete p4;
    for (int i = 0; i < 20; ++i) {
        std::cout << g4[i] << ", ";
        assert(g4[i] == i * 3);
    }
    std::cout << std::endl;
    std::cout << g4.size() << std::endl;

    // std::cout << "g5: ";
    // std::vector<const int> v5 = {1, 2, 3, 4, 5};
    // global_ptr<const int[]> g5{v5};
    // for (int i = 0; i < 5; ++i) {
    //     std::cout << g5[i] << ", ";
    //     assert(g5[i] == i + 1);
    // }
    // std::cout << std::endl;
    // std::cout << g5.size() << std::endl;

    std::cout << "g6: ";
    global_ptr<const int[]> g6{std::move(g4)};
    for (int i = 0; i < 20; ++i) {
        std::cout << g6[i] << ", ";
        assert(g6[i] == i * 3);
    }
    std::cout << std::endl;
    std::cout << g6.size() << std::endl;

    std::cout << "g7: ";
    global_ptr<const int[]> g7{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 10; ++i) {
        std::cout << g7[i] << ", ";
        assert(g7[i] == i);
    }
    std::cout << std::endl;
    std::cout << g7.size() << std::endl;

    try {
        g7.allocate();
        assert(true);
        std::cout << "Error" << std::endl;
    } catch (...) {

    }

    std::cout << "g8: ";
    global_ptr<int[]> g8(20);
    int *p8 = g8.allocate();
    for (int i = 0; i < 20; ++i) {
        p8[i] = i;
    }
    for (int i = 0; i < 20; ++i) {
        std::cout << g8[i] << ", ";
        assert(g8[i] == i);
    }
    std::cout << std::endl;
    std::cout << g8.size() << std::endl;

    // // test for clone

    // std::cout << "g9: ";
    // global_ptr<int[]> g9 = g4.clone();
    // for (int i = 0; i < 20; ++i) {
    //     std::cout << "(" << g9[i] << ", ";
    //     std::cout << g4[i] << "), ";
    //     assert(g9[i] == g4[i]);
    // }
    // std::cout << std::endl;

    // g9[2] = 70;
    // assert(g9[2] == 70);
    // assert(g4[2] = 6);

    // // test for allocate

    // std::cout << "g10: ";
    // global_ptr<int[]> g10(5);
    // int *p10 = g10.allocate();
    // for (int i = 0; i < 10; ++i) {
    //     p10[i] = i * 7;
    // }
    // for (int i = 0; i < 10; ++i) {
    //     std::cout << g10[i] << std::endl;
    //     assert(g10[i] == i * 7);
    // }
}
}
