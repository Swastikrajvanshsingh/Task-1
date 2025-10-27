#ifndef TASKSCHEDULER_TASK_QUEUE_HPP
#define TASKSCHEDULER_TASK_QUEUE_HPP

#include "task.hpp"
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace taskscheduler {

class TaskQueue {
public:
    TaskQueue() = default;
    ~TaskQueue() = default;

    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;

    void push(std::unique_ptr<Task> task);
    std::unique_ptr<Task> pop();
    size_t size() const;
    bool empty() const;
    void close();
    bool is_closed() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::unique_ptr<Task>> queue_;
    bool closed_ = false;
};

} // namespace taskscheduler

#endif // TASKSCHEDULER_TASK_QUEUE_HPP
