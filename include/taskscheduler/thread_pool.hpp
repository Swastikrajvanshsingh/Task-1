#ifndef TASKSCHEDULER_THREAD_POOL_HPP
#define TASKSCHEDULER_THREAD_POOL_HPP

#include "task_queue.hpp"

namespace taskscheduler {

/**
 * Placeholder for thread pool implementation.
 * Will be implemented in future PRs.
 */
class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void submit(std::unique_ptr<Task> task);
    size_t pending_tasks() const;
    bool is_running() const;

private:
    TaskQueue task_queue_;
};

} // namespace taskscheduler

#endif // TASKSCHEDULER_THREAD_POOL_HPP
