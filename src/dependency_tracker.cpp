#include "taskscheduler/dependency_tracker.hpp"

namespace taskscheduler {

TaskId DependencyTracker::assign_id(std::unique_ptr<Task>& task) {
    std::lock_guard<std::mutex> lock(mutex_);
    TaskId id = next_id_++;
    task->set_id(id);
    return id;
}

void DependencyTracker::add_task(std::unique_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(mutex_);

    TaskId task_id = task->id();
    const auto& deps = task->dependencies();

    if (deps.empty()) {
        return;
    }

    remaining_dependencies_[task_id] = deps.size();

    for (TaskId dep_id : deps) {
        dependents_[dep_id].insert(task_id);
    }

    pending_tasks_[task_id] = std::move(task);
}

std::vector<std::unique_ptr<Task>> DependencyTracker::get_ready_tasks() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::unique_ptr<Task>> ready;

    for (auto it = pending_tasks_.begin(); it != pending_tasks_.end(); ) {
        TaskId task_id = it->first;
        if (remaining_dependencies_[task_id] == 0) {
            ready.push_back(std::move(it->second));
            remaining_dependencies_.erase(task_id);
            it = pending_tasks_.erase(it);
        } else {
            ++it;
        }
    }

    return ready;
}

void DependencyTracker::mark_completed(TaskId task_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = dependents_.find(task_id);
    if (it != dependents_.end()) {
        for (TaskId dependent_id : it->second) {
            auto dep_it = remaining_dependencies_.find(dependent_id);
            if (dep_it != remaining_dependencies_.end()) {
                dep_it->second--;
            }
        }
        dependents_.erase(it);
    }
}

bool DependencyTracker::has_pending_tasks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !pending_tasks_.empty();
}

bool DependencyTracker::cancel_task(TaskId id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = pending_tasks_.find(id);
    if (it != pending_tasks_.end()) {
        it->second->cancel();
        return true;
    }

    return false;
}

} // namespace taskscheduler
