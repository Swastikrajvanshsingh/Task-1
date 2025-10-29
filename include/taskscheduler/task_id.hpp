#ifndef TASKSCHEDULER_TASK_ID_HPP
#define TASKSCHEDULER_TASK_ID_HPP

#include <cstdint>

namespace taskscheduler {

using TaskId = uint64_t;

constexpr TaskId INVALID_TASK_ID = 0;

} // namespace taskscheduler

#endif // TASKSCHEDULER_TASK_ID_HPP
