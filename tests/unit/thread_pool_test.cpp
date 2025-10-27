#include <gtest/gtest.h>

#include "taskscheduler/thread_pool.hpp"
#include <atomic>
#include <chrono>
#include <thread>

using namespace taskscheduler;

TEST(ThreadPoolTest, ConstructsWithThreadCount) {
    ThreadPool pool(4);
    EXPECT_TRUE(true);  // Sanity test - just verify construction works
}

TEST(ThreadPoolTest, Issue1_StartsWithCorrectThreadCount) {
    ThreadPool pool(4);
    pool.start();
    EXPECT_EQ(pool.thread_count(), 4);
    EXPECT_TRUE(pool.is_running());
    pool.stop();
}

TEST(ThreadPoolTest, Issue1_ExecutesSubmittedTasks) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};

    for (int i = 0; i < 10; ++i) {
        pool.submit(std::make_unique<Task>([&counter]() { counter++; }));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pool.stop();

    EXPECT_EQ(counter, 10);
}

TEST(ThreadPoolTest, Issue1_StopsGracefully) {
    ThreadPool pool(2);
    std::atomic<int> completed{0};

    for (int i = 0; i < 5; ++i) {
        pool.submit(std::make_unique<Task>([&completed]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            completed++;
        }));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pool.stop();

    EXPECT_EQ(completed, 5);
    EXPECT_FALSE(pool.is_running());
}
