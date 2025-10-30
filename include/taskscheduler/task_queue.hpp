#ifndef TASKSCHEDULER_TASK_QUEUE_HPP
#define TASKSCHEDULER_TASK_QUEUE_HPP

#include "task.hpp"
#include <queue>
#include <vector>
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
    struct TaskWrapper {
        mutable std::unique_ptr<Task> task;
        size_t sequence;

        TaskWrapper(std::unique_ptr<Task> t, size_t seq)
            : task(std::move(t)), sequence(seq) {}

        TaskWrapper(TaskWrapper&& other) noexcept
            : task(std::move(other.task)), sequence(other.sequence) {}

        TaskWrapper& operator=(TaskWrapper&& other) noexcept {
            task = std::move(other.task);
            sequence = other.sequence;
            return *this;
        }

        bool operator<(const TaskWrapper& other) const {
            // First, consider deadlines
            bool this_has_deadline = task->has_deadline();
            bool other_has_deadline = other.task->has_deadline();

            // If both have deadlines, prioritize the earlier deadline
            if (this_has_deadline && other_has_deadline) {
                auto this_deadline = task->deadline().value();
                auto other_deadline = other.task->deadline().value();
                if (this_deadline != other_deadline) {
                    return this_deadline > other_deadline;  // Earlier deadline has higher priority
                }
            }

            // If only one has a deadline, it gets priority
            if (this_has_deadline != other_has_deadline) {
                return !this_has_deadline;  // Task with deadline wins
            }

            // Fall back to priority comparison
            if (task->priority() == other.task->priority()) {
                return sequence > other.sequence;  // FIFO for same priority
            }
            return task->priority() < other.task->priority();  // Higher priority first
        }
    };

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::priority_queue<TaskWrapper> queue_;
    size_t sequence_counter_ = 0;
    bool closed_ = false;
};

} // namespace taskscheduler

#endif // TASKSCHEDULER_TASK_QUEUE_HPP
