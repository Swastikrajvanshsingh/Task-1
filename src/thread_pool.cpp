#include "taskscheduler/thread_pool.hpp"
#include <chrono>

namespace taskscheduler {

ThreadPool::ThreadPool(size_t num_threads) : num_threads_(num_threads) {
    threads_.reserve(num_threads_);
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::start() {
    if (running_) {
        return;
    }

    running_ = true;
    for (size_t i = 0; i < num_threads_; ++i) {
        threads_.emplace_back(&ThreadPool::worker_loop, this);
    }
}

void ThreadPool::stop() {
    shutdown_immediate();
}

void ThreadPool::shutdown_graceful() {
    if (!running_) {
        return;
    }

    running_ = false;
    task_queue_.close();

    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    threads_.clear();
}

void ThreadPool::shutdown_immediate() {
    if (!running_) {
        return;
    }

    running_ = false;
    task_queue_.close();

    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    threads_.clear();
}

void ThreadPool::submit(std::unique_ptr<Task> task) {
    if (!running_ && !task_queue_.is_closed()) {
        start();
    }

    if (task_queue_.is_closed()) {
        return;  // Don't accept new tasks after shutdown
    }

    TaskId task_id = dependency_tracker_.assign_id(task);

    if (task->dependencies().empty()) {
        task_queue_.push(std::move(task));
    } else {
        dependency_tracker_.add_task(std::move(task));
        process_ready_tasks();
    }
}

TaskId ThreadPool::submit_with_id(std::unique_ptr<Task> task) {
    if (!running_) {
        start();
    }

    TaskId task_id = dependency_tracker_.assign_id(task);

    if (task->dependencies().empty()) {
        task_queue_.push(std::move(task));
    } else {
        dependency_tracker_.add_task(std::move(task));
        process_ready_tasks();
    }

    return task_id;
}

void ThreadPool::process_ready_tasks() {
    auto ready = dependency_tracker_.get_ready_tasks();
    for (auto& task : ready) {
        task_queue_.push(std::move(task));
    }
}

size_t ThreadPool::thread_count() const {
    return threads_.size();
}

size_t ThreadPool::pending_tasks() const {
    return task_queue_.size();
}

bool ThreadPool::is_running() const {
    return running_;
}

StatisticsSnapshot ThreadPool::get_statistics() const {
    return statistics_.get_snapshot();
}

void ThreadPool::reset_statistics() {
    statistics_.reset();
}

bool ThreadPool::cancel_task(TaskId id) {
    // Try to cancel in the task queue
    bool cancelled_in_queue = task_queue_.cancel_task(id);

    // Try to cancel in the dependency tracker
    bool cancelled_in_tracker = dependency_tracker_.cancel_task(id);

    return cancelled_in_queue || cancelled_in_tracker;
}

void ThreadPool::worker_loop() {
    while (running_ || !task_queue_.is_closed()) {
        statistics_.set_queue_depth(task_queue_.size());
        auto task = task_queue_.pop();
        if (task) {
            statistics_.increment_active_workers();

            auto start_time = std::chrono::high_resolution_clock::now();
            TaskId task_id = task->id();
            task->execute();
            auto end_time = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double, std::milli> duration = end_time - start_time;
            statistics_.record_task_completed(duration.count());
            statistics_.decrement_active_workers();

            dependency_tracker_.mark_completed(task_id);
            process_ready_tasks();
        }

        if (task_queue_.is_closed() && task_queue_.empty()) {
            break;
        }
    }
}

} // namespace taskscheduler
