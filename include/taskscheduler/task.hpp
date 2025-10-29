#ifndef TASKSCHEDULER_TASK_HPP
#define TASKSCHEDULER_TASK_HPP

#include "priority.hpp"
#include "task_id.hpp"
#include <functional>
#include <vector>

namespace taskscheduler {

class Task {
public:
    using Callable = std::function<void()>;

    explicit Task(Callable callable, Priority priority = Priority::NORMAL);
    explicit Task(Callable callable, Priority priority, const std::vector<TaskId>& dependencies);
    virtual ~Task() = default;

    void execute();
    Priority priority() const;
    TaskId id() const;
    const std::vector<TaskId>& dependencies() const;
    void set_id(TaskId id);

private:
    Callable callable_;
    Priority priority_;
    TaskId id_{INVALID_TASK_ID};
    std::vector<TaskId> dependencies_;
};

} // namespace taskscheduler

#endif // TASKSCHEDULER_TASK_HPP
