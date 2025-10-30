#ifndef TASKSCHEDULER_STATISTICS_HPP
#define TASKSCHEDULER_STATISTICS_HPP

#include <atomic>
#include <mutex>
#include <chrono>
#include <limits>

namespace taskscheduler {

struct StatisticsSnapshot {
    size_t completed_tasks;
    size_t active_workers;
    size_t queue_depth;
    double min_execution_time_ms;
    double max_execution_time_ms;
    double avg_execution_time_ms;
};

class Statistics {
public:
    Statistics();
    ~Statistics() = default;

    void record_task_completed(double execution_time_ms);
    void increment_active_workers();
    void decrement_active_workers();
    void set_queue_depth(size_t depth);

    StatisticsSnapshot get_snapshot() const;
    void reset();

private:
    std::atomic<size_t> completed_tasks_{0};
    std::atomic<size_t> active_workers_{0};
    std::atomic<size_t> queue_depth_{0};

    mutable std::mutex execution_time_mutex_;
    double min_execution_time_ms_{std::numeric_limits<double>::max()};
    double max_execution_time_ms_{0.0};
    double total_execution_time_ms_{0.0};
};

} // namespace taskscheduler

#endif // TASKSCHEDULER_STATISTICS_HPP
