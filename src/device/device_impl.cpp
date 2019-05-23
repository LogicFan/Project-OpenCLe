#include <iostream>
#include <stdexcept>
#include <utility>

#include "device_impl.hpp"

namespace {
void pfn_notify(const char *errinfo, const void *private_info, size_t cb,
                void *user_data) {
    throw std::runtime_error{"OpenCL runtime error: Context failure. " +
                             std::string{errinfo}};
}

cl_context get_context(cl_device_id const &dev) {
    logger("Creating context based on device " << dev << std::endl);
    cl_int status;
    cl_context temp = clCreateContext(NULL, 1, &dev, pfn_notify, NULL, &status);

    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot initialize context"};
    }

    return temp;
}

cl_command_queue get_command_queue(cl_device_id const &dev,
                                   cl_context const &con) {
    logger("Createing command queue based on device " << dev << " and context "
                                                      << con << std::endl);
    cl_int status;
    cl_command_queue temp =
        clCreateCommandQueueWithProperties(con, dev, NULL, &status);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot initialize command queue"};
    }

    return temp;
}

size_t get_computate_unit(cl_device_id const &dev) {
    logger("Getting the total available compute unit on device " << dev
                                                                 << std::endl);
    cl_int status;
    cl_uint cu_num;
    status = clGetDeviceInfo(dev, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint),
                             &cu_num, NULL);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot get device info"};
    }

    return static_cast<size_t>(cu_num);
}
} // namespace

namespace opencle {
device_impl::device_impl(cl_device_id const &dev, cl_context const &con,
                         cl_command_queue const &cmd)
    : device_{dev}, context_{con}, cmd_queue_{cmd},
      cu_total{get_computate_unit(dev)}, valid_{true}, cu_used{0} {
    logger("Calling 3 parameter constructor, create " << this << std::endl);
    return;
}

device_impl::device_impl(cl_device_id const &dev)
    : device_{dev}, context_{get_context(device_)},
      cmd_queue_{get_command_queue(device_, context_)},
      cu_total{get_computate_unit(dev)}, valid_{true}, cu_used{0} {
    logger("Calling 1 parameter constructor, create " << this << std::endl);
    return;
}

// device_impl::device_impl(device_impl &&rhs)
//     : device_{rhs.device_}, context_{rhs.context_},
//       cmd_queue_{rhs.cmd_queue_}, cu_total{rhs.cu_total},
//       valid_{rhs.valid_.load()}, cu_used{rhs.cu_used.load()} {
//     logger("Calling move constructor, creat " << this << " based on " << &rhs
//                                               << std::endl);
//     rhs.device_ = nullptr;
//     rhs.context_ = nullptr;
//     rhs.cmd_queue_ = nullptr;
//     rhs.valid_ = false;
// }

device_impl::~device_impl() {
    logger("Calling destructor, destory " << this << std::endl);
    clReleaseCommandQueue(cmd_queue_);
    clReleaseContext(context_);
}

// device_impl &device_impl::operator=(device_impl &&rhs) {
//     this->~device_impl();
//     device_impl(std::move(rhs));
//     return *this;
// }

bool device_impl::operator<(device_impl const &rhs) const {
    return !valid_ && cu_total - cu_used < rhs.cu_total - rhs.cu_used;
}

device_impl::operator bool() { return valid_; }

std::vector<device> device_impl::get_device_list() {
    std::vector<device> device_list;

    cl_int status;

    logger("Getting Platform" << std::endl);

    cl_uint platform_num;
    status = clGetPlatformIDs(0, NULL, &platform_num);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot initialize platform"};
    } else if (platform_num == 0) {
        throw std::out_of_range{"Cannot detect platforms"};
    }

    logger("There are " << platform_num << " number of platforms" << std::endl);

    std::unique_ptr<cl_platform_id[]> platforms{
        new cl_platform_id[platform_num]};
    status = clGetPlatformIDs(platform_num, platforms.get(), NULL);
    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot initialize platform"};
    }

    for (int i = 0; i < platform_num; ++i) {
        logger("Using platform " << platforms[i] << " to initialize devices"
                                 << std::endl);

        cl_uint device_num;
        status = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL,
                                &device_num);
        if (status != CL_SUCCESS) {
            throw std::runtime_error{
                "OpenCL runtime error: Cannot initialize device"};
        } else if (device_num == 0) {
            throw std::out_of_range{"Cannot detect devices"};
        }

        logger("There are " << device_num << " number of devices" << std::endl);

        std::unique_ptr<cl_device_id[]> devices{new cl_device_id[device_num]};

        status = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, device_num,
                                devices.get(), NULL);

        if (status != CL_SUCCESS) {
            throw std::runtime_error{
                "OpenCL runtime error: Cannot initialize device"};
        }

        for (int j = 0; j < device_num; ++j) {
            logger("Initializing device " << devices[j] << std::endl);

            device_list.emplace_back(new device_impl{devices[j]});
        }
    }

    return std::move(device_list);
}

std::ostream &operator<<(std::ostream &out, device_impl const &dev) {
    cl_int status;

    char device_name[100];
    status =
        clGetDeviceInfo(dev.device_, CL_DEVICE_NAME, 100, device_name, NULL);

    if (status != CL_SUCCESS) {
        throw std::runtime_error{
            "OpenCL runtime error: Cannot get device info"};
    }

    out << "Device: " << device_name;
    return out;
}

std::ostream &operator<<(std::ostream &out, device const &dev) {
    return operator<<(out, *dev);
}

} // namespace opencle
