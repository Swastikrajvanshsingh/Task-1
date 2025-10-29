#include "taskscheduler/task.hpp"

namespace taskscheduler {

Task::Task(Callable callable, Priority priority)
    : callable_(std::move(callable)), priority_(priority) {}

Task::Task(Callable callable, Priority priority, const std::vector<TaskId>& dependencies)
    : callable_(std::move(callable)), priority_(priority), dependencies_(dependencies) {}

void Task::execute() {
    if (callable_) {
        callable_();
    }
}

Priority Task::priority() const {
    return priority_;
}

TaskId Task::id() const {
    return id_;
}

const std::vector<TaskId>& Task::dependencies() const {
    return dependencies_;
}

void Task::set_id(TaskId id) {
    id_ = id;
}

} // namespace taskscheduler
