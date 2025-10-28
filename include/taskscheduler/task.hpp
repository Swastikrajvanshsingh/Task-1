#ifndef TASKSCHEDULER_TASK_HPP
#define TASKSCHEDULER_TASK_HPP

#include "priority.hpp"
#include <functional>

namespace taskscheduler {

class Task {
public:
    using Callable = std::function<void()>;

    explicit Task(Callable callable, Priority priority = Priority::NORMAL);
    virtual ~Task() = default;

    void execute();
    Priority priority() const;

private:
    Callable callable_;
    Priority priority_;
};

} // namespace taskscheduler

#endif // TASKSCHEDULER_TASK_HPP
