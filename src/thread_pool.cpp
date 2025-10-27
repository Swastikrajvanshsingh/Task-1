#include "taskscheduler/thread_pool.hpp"

namespace taskscheduler {

ThreadPool::ThreadPool(size_t num_threads) {
    // Placeholder implementation
    (void)num_threads;
}

ThreadPool::~ThreadPool() {
    // Placeholder implementation
}

void ThreadPool::submit(std::unique_ptr<Task> task) {
    task_queue_.push(std::move(task));
}

size_t ThreadPool::pending_tasks() const {
    return task_queue_.size();
}

bool ThreadPool::is_running() const {
    return false;
}

} // namespace taskscheduler
