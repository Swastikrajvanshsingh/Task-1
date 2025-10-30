#include "taskscheduler/statistics.hpp"

namespace taskscheduler {

Statistics::Statistics() {}

void Statistics::record_task_completed(double execution_time_ms) {
    completed_tasks_.fetch_add(1, std::memory_order_relaxed);

    std::lock_guard<std::mutex> lock(execution_time_mutex_);
    total_execution_time_ms_ += execution_time_ms;
    if (execution_time_ms < min_execution_time_ms_) {
        min_execution_time_ms_ = execution_time_ms;
    }
    if (execution_time_ms > max_execution_time_ms_) {
        max_execution_time_ms_ = execution_time_ms;
    }
}

void Statistics::increment_active_workers() {
    active_workers_.fetch_add(1, std::memory_order_relaxed);
}

void Statistics::decrement_active_workers() {
    active_workers_.fetch_sub(1, std::memory_order_relaxed);
}

void Statistics::set_queue_depth(size_t depth) {
    queue_depth_.store(depth, std::memory_order_relaxed);
}

StatisticsSnapshot Statistics::get_snapshot() const {
    StatisticsSnapshot snapshot;
    snapshot.completed_tasks = completed_tasks_.load(std::memory_order_relaxed);
    snapshot.active_workers = active_workers_.load(std::memory_order_relaxed);
    snapshot.queue_depth = queue_depth_.load(std::memory_order_relaxed);

    std::lock_guard<std::mutex> lock(execution_time_mutex_);
    snapshot.min_execution_time_ms = (completed_tasks_.load() > 0) ? min_execution_time_ms_ : 0.0;
    snapshot.max_execution_time_ms = max_execution_time_ms_;
    snapshot.avg_execution_time_ms = (completed_tasks_.load() > 0) 
        ? total_execution_time_ms_ / completed_tasks_.load() 
        : 0.0;

    return snapshot;
}

void Statistics::reset() {
    completed_tasks_.store(0, std::memory_order_relaxed);
    active_workers_.store(0, std::memory_order_relaxed);
    queue_depth_.store(0, std::memory_order_relaxed);

    std::lock_guard<std::mutex> lock(execution_time_mutex_);
    min_execution_time_ms_ = std::numeric_limits<double>::max();
    max_execution_time_ms_ = 0.0;
    total_execution_time_ms_ = 0.0;
}

} // namespace taskscheduler
