#ifndef OPENCLE_HPP
#define OPENCLE_HPP

#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <string>

class OpenCLe final {
  public:
    class Task;
    class Memory;
    class Code;
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