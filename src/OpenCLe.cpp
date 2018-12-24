#include "OpenCLe.hpp"

static size_t reference_count_ = 0;

OpenCLe::OpenCLe() {
    if(reference_count_ == 0) {
        // fetch platform and devices
        // crate a consumer thread
    }
    reference_count_ = reference_count_ + 1;
}

OpenCLe::OpenCLe(OpenCLe const &other) {
    reference_count_ = reference_count_ + 1;
}

OpenCLe::OpenCLe(OpenCLe &&other) {
    reference_count_ = reference_count_ + 1;
}

OpenCLe::~OpenCLe() {
    reference_count_ = reference_count_ - 1;
    if(reference_count_ == 0) {
        // wait for consumer thread finish
        // exit consumer thread
        // clear platform and devices
    }
}

OpenCLe &OpenCLe::operator=(OpenCLe const &other) {}
OpenCLe &OpenCLe::operator=(OpenCLe &&other) {}

void OpenCLe::push(Task &&t) {
    // push task into task_waiting_list
}