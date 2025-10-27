#include "taskscheduler/task.hpp"

namespace taskscheduler {

Task::Task(Callable callable) : callable_(std::move(callable)) {}

void Task::execute() {
    if (callable_) {
        callable_();
    }
}

} // namespace taskscheduler
