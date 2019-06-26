#include <iostream>
#include <stdexcept>
#include <utility>
#include <type_traits>

#define NDEBUG

#include "device_impl.hpp"
#include "../memory/global_ptr.hpp"

namespace {
void pfn_notify(const char *errinfo, const void *private_info, size_t cb, void *user_data) {
    logger("pfn_notify");

    throw std::runtime_error{"OpenCL runtime error: Context failure. " + std::string{errinfo}};
}

cl_context get_context(cl_device_id const &dev) {
    logger("get_context");
    cl_int status;
    cl_context temp = clCreateContext(NULL, 1, &dev, pfn_notify, NULL, &status);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize context!"};
    }
    logger("Create context " << temp << " based on device_id " << dev << "!");

    return temp;
}

cl_command_queue get_command_queue(cl_device_id const &dev, cl_context const &con) {
    logger("get_command_queue");
    cl_int status;
    cl_command_queue temp = clCreateCommandQueueWithProperties(con, dev, NULL, &status);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize command queue!"};
    }
    logger("Create command queue " << temp << " based on context " << con << " and device_id " << dev << "!");

    return temp;
}

size_t get_computate_unit(cl_device_id const &dev) {
    logger("get_computate_unit");
    cl_int status;
    cl_uint cu_num;
    status = clGetDeviceInfo(dev, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &cu_num, NULL);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot get device info!"};
    }
    logger("There are " << cu_num << " number of compute unit on device where device_id is " << dev << "!");

    return static_cast<size_t>(cu_num);
}
} // namespace

namespace opencle {
device_impl::device_impl(cl_device_id const &dev, cl_context const &con, cl_command_queue const &cmd)
    : device_{dev}, context_{con}, cmd_queue_{cmd}, cu_total{get_computate_unit(dev)}, valid_{true}, cu_used{0} {
    logger("device_impl(device_id, context, command_queue), create " << this);
    return;
}

device_impl::device_impl(cl_device_id const &dev)
    : device_{dev}, context_{get_context(device_)},
      cmd_queue_{get_command_queue(device_, context_)}, cu_total{get_computate_unit(dev)}, valid_{true}, cu_used{0} {
    logger("device_impl(device_id), create " << this);
    return;
}

// device_impl::device_impl(device_impl &&rhs)
//     : device_{rhs.device_}, context_{rhs.context_},
//       cmd_queue_{rhs.cmd_queue_}, cu_total{rhs.cu_total},
//       valid_{rhs.valid_.load()}, cu_used{rhs.cu_used.load()} {
//     rhs.device_ = nullptr;
//     rhs.context_ = nullptr;
//     rhs.cmd_queue_ = nullptr;
//     rhs.valid_ = false;
// }

device_impl::~device_impl() {
    logger("~device_impl, destory " << this);
    clReleaseCommandQueue(cmd_queue_);
    logger("Release command queue " << cmd_queue_);
    clReleaseContext(context_);
    logger("Release context " << context_);
}

// device_impl &device_impl::operator=(device_impl &&rhs) {
//     this->~device_impl();
//     device_impl(std::move(rhs));
//     return *this;
// }

bool device_impl::operator<(device_impl const &rhs) const {
    logger("operator<, " << this << " and " << &rhs);
    return !valid_ && cu_total - cu_used < rhs.cu_total - rhs.cu_used;
}

device_impl::operator bool() { 
    logger("operator bool")
    return valid_; 
}

std::vector<device> device_impl::get_device_list() {
    logger("get_device_list");
    std::vector<device> device_list;

    cl_int status;

    cl_uint platform_num;
    status = clGetPlatformIDs(0, NULL, &platform_num);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize platform!"};
    } else if (platform_num == 0) {
        throw std::out_of_range{"No platforms detected!"};
    }

    logger("Detect " << platform_num << " number of platforms!");

    std::unique_ptr<cl_platform_id[]> platforms{new cl_platform_id[platform_num]};
    status = clGetPlatformIDs(platform_num, platforms.get(), NULL);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot initialize platform!"};
    }

    for (int i = 0; i < platform_num; ++i) {
        logger("In platform " << platforms[i] << "!");

        cl_uint device_num;
        status = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &device_num);
        if (status != CL_SUCCESS) {
            throw std::runtime_error{"OpenCL runtime error: Cannot initialize device"};
        } else if (device_num == 0) {
            throw std::out_of_range{"Cannot detect devices"};
        }

        logger("Detect " << device_num << " number of devices!");

        std::unique_ptr<cl_device_id[]> devices{new cl_device_id[device_num]};

        status = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, device_num, devices.get(), NULL);

        if (status != CL_SUCCESS) {
            throw std::runtime_error{"OpenCL runtime error: Cannot initialize device"};
        }

        for (int j = 0; j < device_num; ++j) {
            device_list.emplace_back(new device_impl{devices[j]});
            logger("Initialize device with device_id " << devices[j] << "!");
        }
    }

    return std::move(device_list);
}

std::ostream &operator<<(std::ostream &out, device_impl const &dev) {
    cl_int status;

    char device_name[100];
    status = clGetDeviceInfo(dev.device_, CL_DEVICE_NAME, 100, device_name, NULL);

    if (status != CL_SUCCESS) {
        throw std::runtime_error{"OpenCL runtime error: Cannot get device info"};
    }

    out << "Device: " << device_name << ", " << &dev;
    return out;
}

std::ostream &operator<<(std::ostream &out, device const &dev) { return operator<<(out, *dev); }

} // namespace opencle
