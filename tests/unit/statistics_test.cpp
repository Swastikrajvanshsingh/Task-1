#include <gtest/gtest.h>

#include "taskscheduler/thread_pool.hpp"
#include <chrono>
#include <thread>

using namespace taskscheduler;

// F2P Test: Statistics tracks completed tasks
TEST(StatisticsTest, Issue11_TracksCompletedTasks) {
    ThreadPool pool(2);
    pool.start();

    int counter = 0;
    for (int i = 0; i < 5; ++i) {
        pool.submit([&counter]() { counter++; });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pool.stop();

    auto stats = pool.get_statistics();
    EXPECT_EQ(stats.completed_tasks, 5);
    EXPECT_EQ(counter, 5);
}

// F2P Test: Statistics tracks execution times
TEST(StatisticsTest, Issue11_TracksExecutionTimes) {
    ThreadPool pool(1);
    pool.start();

    pool.submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
    pool.submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    });
    pool.submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pool.stop();

    auto stats = pool.get_statistics();
    EXPECT_EQ(stats.completed_tasks, 3);
    EXPECT_GT(stats.min_execution_time_ms, 8.0);
    EXPECT_LT(stats.min_execution_time_ms, 12.0);
    EXPECT_GT(stats.max_execution_time_ms, 18.0);
    EXPECT_LT(stats.max_execution_time_ms, 25.0);
    EXPECT_GT(stats.avg_execution_time_ms, 10.0);
    EXPECT_LT(stats.avg_execution_time_ms, 20.0);
}

// F2P Test: Statistics tracks queue depth
TEST(StatisticsTest, Issue11_TracksQueueDepth) {
    ThreadPool pool(1);
    pool.start();

    for (int i = 0; i < 10; ++i) {
        pool.submit([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    auto stats = pool.get_statistics();
    EXPECT_GT(stats.queue_depth, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    pool.stop();

    stats = pool.get_statistics();
    EXPECT_EQ(stats.queue_depth, 0);
}

// P2P Test: Reset statistics works correctly
TEST(StatisticsTest, Issue11_ResetWorks) {
    ThreadPool pool(2);
    pool.start();

    for (int i = 0; i < 5; ++i) {
        pool.submit([]() {});
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto stats = pool.get_statistics();
    EXPECT_EQ(stats.completed_tasks, 5);

    pool.reset_statistics();

    stats = pool.get_statistics();
    EXPECT_EQ(stats.completed_tasks, 0);
    EXPECT_EQ(stats.min_execution_time_ms, 0.0);
    EXPECT_EQ(stats.max_execution_time_ms, 0.0);
    EXPECT_EQ(stats.avg_execution_time_ms, 0.0);

    pool.stop();
}

// P2P Test: Thread pool continues to work with statistics enabled
TEST(StatisticsTest, Issue11_DoesNotAffectExecution) {
    ThreadPool pool(4);
    pool.start();

    std::atomic<int> counter{0};
    for (int i = 0; i < 20; ++i) {
        pool.submit([&counter]() { counter++; });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pool.stop();

    EXPECT_EQ(counter, 20);

    auto stats = pool.get_statistics();
    EXPECT_EQ(stats.completed_tasks, 20);
}
