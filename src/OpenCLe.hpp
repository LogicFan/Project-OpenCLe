#ifndef OPENCLE
#define OPENCLE

#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <string>

/**
 * class OpenCLe is a singleton object. Underlining
 * implementation is OpenCL. When a task is pushed,
 * OpenCLe will automatic decide when and which devices
 * run the task.
 */

class OpenCLe final {
  public:
    class Task;
    class Data;
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

class OpenCLe::Data final {
    std::unique_ptr<void> buf_;
    size_t size_;

  public:
    Data(std::unique_ptr<void> buf, size_t size);
    Data(Data const &other);
    Data(Data &&other);
    ~Data();

    Data &operator=(Data const &other);
    Data &operator=(Data &&other);
};

class OpenCLe::Code final {
    std::string code_;

  public:
    Code(std::string s);
};

class OpenCLe::Dependency final {
    std::list<Task *> depList_;

  public:
    Dependency(std::list<Task *> l);
};

class OpenCLe::Task final {
  public:
    using Fn = std::function<void(Data)>;
    Task(Data const &d, Code const &c, Dependency const &dp, Fn callBack);
};

std::ostream &operator<<(std::ostream &out, OpenCLe::Data rhs);
std::ostream &operator<<(std::ostream &out, OpenCLe::Code rhs);
std::ostream &operator<<(std::ostream &out, OpenCLe::Dependency rhs);
std::ostream &operator<<(std::ostream &out, OpenCLe::Task rhs);
std::ostream &operator<<(std::ostream &out, OpenCLe);

#endif