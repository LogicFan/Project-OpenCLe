#include <CL/cl.h>

#include <queue>

struct Device{
    cl_device_id device;
    cl_context context;
    cl_command_queue c_queue;
};

void schedule(std::vector<Device> const &processor_list, std::queue<int> &task_queue);