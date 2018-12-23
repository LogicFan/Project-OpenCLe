#ifndef TASK_TASK_HPP
#define TASK_TASK_HPP

#include "../Dependency.hpp"
#include "../OpenCLe.hpp"
#include <memory>
#include <vector>

class OpenCLe::Task {
  private:
    std::shared_ptr<Task *> task_id_;
    Dependency dependency_list_;

  protected:
    bool valid_bit_;
    std::vector<Memory> memory_list_;

  public:
    Task(std::vector<Memory>, Dependency);
    Task(Task const &other) = delete;
    Task(Task &&other) = delete;
    virtual ~Task() = 0;

    std::shared_ptr<Task *> getTaskId();

    friend class OpenCLe;
};

inline OpenCLe::Task::Task(std::vector<Memory> const memory_list,
                           Dependency dependency_list)
    : task_id_{new Task *{this}}, dependency_list_{dependency_list},
      valid_bit_{false}, memory_list_{memory_list} {}

inline OpenCLe::Task::~Task() noexcept { *task_id_ = nullptr; }

inline std::shared_ptr<OpenCLe::Task *> OpenCLe::Task::getTaskId() noexcept {
    return task_id_;
}

#endif