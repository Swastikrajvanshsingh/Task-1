#include "taskscheduler/task_queue.hpp"
#include <queue>
#include <vector>

namespace taskscheduler {

void TaskQueue::push(std::unique_ptr<Task> task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) {
            return;
        }
        queue_.emplace(std::move(task), sequence_counter_++);
    }
    cv_.notify_one();
}

std::unique_ptr<Task> TaskQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty() || closed_; });

    if (queue_.empty()) {
        return nullptr;
    }

    auto task = std::move(queue_.top().task);
    queue_.pop();
    return task;
}

size_t TaskQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

bool TaskQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

void TaskQueue::close() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
    }
    cv_.notify_all();
}

bool TaskQueue::is_closed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return closed_;
}

bool TaskQueue::cancel_task(TaskId id) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Extract all tasks from the priority queue to search for the target
    std::vector<TaskWrapper> temp_tasks;
    bool found = false;

    while (!queue_.empty()) {
        auto wrapper = std::move(const_cast<TaskWrapper&>(queue_.top()));
        queue_.pop();

        if (wrapper.task->id() == id) {
            wrapper.task->cancel();
            found = true;
        }
        temp_tasks.push_back(std::move(wrapper));
    }

    // Restore all tasks back to the queue
    for (auto& wrapper : temp_tasks) {
        queue_.push(std::move(wrapper));
    }

    return found;
}

} // namespace taskscheduler
