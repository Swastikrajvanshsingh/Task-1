#ifndef TASKSCHEDULER_PRIORITY_HPP
#define TASKSCHEDULER_PRIORITY_HPP

namespace taskscheduler {

enum class Priority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

} // namespace taskscheduler

#endif // TASKSCHEDULER_PRIORITY_HPP
