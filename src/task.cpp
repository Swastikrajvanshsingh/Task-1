#include "taskscheduler/task.hpp"

namespace taskscheduler {

Task::Task(Callable callable, Priority priority)
    : callable_(std::move(callable)), priority_(priority) {}

void Task::execute() {
    if (callable_) {
        callable_();
    }
}

Priority Task::priority() const {
    return priority_;
}

} // namespace taskscheduler
