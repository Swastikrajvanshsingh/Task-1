#ifndef TASKSCHEDULER_DEPENDENCY_TRACKER_HPP
#define TASKSCHEDULER_DEPENDENCY_TRACKER_HPP

#include "task.hpp"
#include "task_id.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <mutex>

namespace taskscheduler {

class DependencyTracker {
public:
    DependencyTracker() = default;
    ~DependencyTracker() = default;

    DependencyTracker(const DependencyTracker&) = delete;
    DependencyTracker& operator=(const DependencyTracker&) = delete;

    TaskId assign_id(std::unique_ptr<Task>& task);
    void add_task(std::unique_ptr<Task> task);
    std::vector<std::unique_ptr<Task>> get_ready_tasks();
    void mark_completed(TaskId task_id);
    bool has_pending_tasks() const;
    bool cancel_task(TaskId id);

private:
    mutable std::mutex mutex_;
    TaskId next_id_{1};

    std::unordered_map<TaskId, std::unique_ptr<Task>> pending_tasks_;
    std::unordered_map<TaskId, std::unordered_set<TaskId>> dependents_;
    std::unordered_map<TaskId, size_t> remaining_dependencies_;
};

} // namespace taskscheduler

#endif // TASKSCHEDULER_DEPENDENCY_TRACKER_HPP
