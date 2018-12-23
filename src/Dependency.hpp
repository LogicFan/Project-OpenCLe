#ifndef DEPENDENCY_HPP
#define DEPENDENCY_HPP

#include "OpenCLe.hpp"
#include <memory>
#include <vector>

class OpenCLe::Task;

class OpenCLe::Dependency final {
  private:
    std::vector<std::shared_ptr<Task *>> task_id_list_;
};
#endif