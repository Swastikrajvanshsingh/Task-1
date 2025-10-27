#include "taskscheduler/thread_pool.hpp"

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
    if (!running_) {
        start();
    }
    task_queue_.push(std::move(task));
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

void ThreadPool::worker_loop() {
    while (running_ || !task_queue_.is_closed()) {
        auto task = task_queue_.pop();
        if (task) {
            task->execute();
        }

        if (task_queue_.is_closed() && task_queue_.empty()) {
            break;
        }
    }
}

} // namespace taskscheduler
