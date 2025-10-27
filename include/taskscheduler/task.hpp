#ifndef TASKSCHEDULER_TASK_HPP
#define TASKSCHEDULER_TASK_HPP

#include <functional>

namespace taskscheduler {

class Task {
public:
    using Callable = std::function<void()>;

    explicit Task(Callable callable);
    virtual ~Task() = default;

    void execute();

private:
    Callable callable_;
};

} // namespace taskscheduler

#endif // TASKSCHEDULER_TASK_HPP
