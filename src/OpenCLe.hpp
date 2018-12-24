#ifndef OPENCLE_HPP
#define OPENCLE_HPP

#include <functional>
#include <iostream>
#include <queue>
#include <memory>
#include <string>

class OpenCLe final {
  public:
    class Task;
    class MemOps;
    class Kernel;
    class Function;

  private:
    class Device;
    class Platform;

    static size_t reference_count_;

    static std::vector<Platform> const platform_list_;
    static std::vector<Device> const device_list_;

    static std::queue<std::unique_ptr<Task>> task_waiting_list_;

  public:
    class Memory;
    class Dependency;

    OpenCLe();
    OpenCLe(OpenCLe const &other);
    OpenCLe(OpenCLe &&other);
    ~OpenCLe();

    OpenCLe &operator=(OpenCLe const &other);
    OpenCLe &operator=(OpenCLe &&other);

    void push(Task &&t);
};

#endif