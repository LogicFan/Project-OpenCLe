#include "AMP.hpp"
#include "Scheduler.hpp"
#include "Task.hpp"

#include <atomic>
#include <mutex>
#include <thread>

#include <CL/cl.h>

#include "util/queue/queue.hpp"
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
static unsigned int count_ref{0};

static std::mutex ctor_dtor_lock;

static std::thread scheduler_thread;

static opencle::queue<opencle::Task> task_queue;

void notify(const char *, const void *, size_t, void *) {
    throw std::runtime_error{"OpenCL runtime error"};
}

} // namespace

namespace opencle {

AMP::AMP(Device_Type type) {
    ctor_dtor_lock.lock();

    static std::vector<Device> processor_list{};

    if (count_ref == 0) {
        cl_int status{0};

        // Initialize opencl platforms
        cl_uint platform_num{0};

        status = clGetPlatformIDs(0, NULL, &platform_num);

        if (status != CL_SUCCESS) {
            throw std::runtime_error{"OpenCL initialization error."};
        }
        if (platform_num == 0) {
            throw std::runtime_error{"No OpenCL platform available."};
        }

        std::unique_ptr<cl_platform_id[]> platforms{
            new cl_platform_id[platform_num]};
        status = clGetPlatformIDs(platform_num, platforms.get(), NULL);

        if (status != CL_SUCCESS) {
            throw std::runtime_error{"OpenCL initialization error."};
        }

        // Initialize opencl devices
        cl_device_type device_type{CL_DEVICE_TYPE_DEFAULT};
        switch (type) {
        case Device_Type::ACC:
            device_type = CL_DEVICE_TYPE_ACCELERATOR;
            break;
        case Device_Type::ALL:
            device_type = CL_DEVICE_TYPE_ALL;
            break;
        case Device_Type::CPU:
            device_type = CL_DEVICE_TYPE_CPU;
            break;
        case Device_Type::GPU:
            device_type = CL_DEVICE_TYPE_GPU;
            break;
        default:
            device_type = CL_DEVICE_TYPE_DEFAULT;
            break;
        }

        for (int i = 0; i < platform_num; ++i) {
            cl_uint device_num{0};
            status = clGetDeviceIDs(platforms.get()[i], device_type, 0, NULL,
                                    &device_num);

            if (status != CL_SUCCESS) {
                throw std::runtime_error{"OpenCL initialization error."};
            }
            if (device_num == 0) {
                throw std::runtime_error{"No OpenCL device available."};
            }

            std::unique_ptr<cl_device_id[]> devices{
                new cl_device_id[device_num]};
            status = clGetDeviceIDs(platforms.get()[i], device_type, device_num,
                                    devices.get(), NULL);

            for (int j = 0; j < device_num; ++j) {
                processor_list.push_back({devices.get()[j], NULL, NULL});
            }

            devices.release();
        }
        platforms.release();

        // Initialize context

        for (auto &processor : processor_list) {
            processor.context = clCreateContext(NULL, 1, &processor.device,
                                                notify, NULL, &status);
            if (status != CL_SUCCESS) {
                throw std::runtime_error{"OpenCL initialization error."};
            }
        }

        // Initialize opencl command queues.

        for (auto &processor : processor_list) {
            // Property may be change.
            processor.c_queue = clCreateCommandQueue(
                processor.context, processor.device, 0, &status);
            if (status != CL_SUCCESS) {
                throw std::runtime_error{"OpenCL initialization error."};
            }
        }

        // Initialize scheduler thread
        std::thread scheduler{schedule, std::ref(processor_list),
                              std::ref(task_queue)};
        scheduler_thread = std::move(scheduler);
    }

    ++count_ref;
    ctor_dtor_lock.unlock();
}

AMP::AMP(AMP const &rhs) {}

AMP::AMP(AMP &&rhs) {}

AMP::~AMP() {
    ctor_dtor_lock.lock();

    --count_ref;

    if (count_ref == 0) {
        // push end-of-task Task

        // destruct scheduler_thread
        scheduler_thread.join();

        // destruct task queue
        task_queue.~queue();
    }
    ctor_dtor_lock.unlock();
}

AMP &AMP::operator=(AMP const &rhs) { return *this; }

AMP &AMP::operator=(AMP &&rhs) { return *this; }

} // namespace opencle

#if 0

    Scheduler &operator=(Scheduler const &rhs);
    Scheduler &operator=(Scheduler &&rhs);
    Scheduler &operator[](size_t num);

    Memories exec(Task const &rhs, std::function<void(const Memories&)> func);
    Memories exec(Task &&rhs, std::function<void(const Memories&)> func);

    void print_info();
#endif
