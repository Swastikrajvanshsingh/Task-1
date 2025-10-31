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

TEST(ThreadPoolTest, Issue3_SubmitReturnsIntegerFuture) {
    ThreadPool pool(2);

    auto future = pool.submit([]() { return 42; });

    EXPECT_EQ(future.get(), 42);
    pool.stop();
}

TEST(ThreadPoolTest, Issue3_SubmitHandlesMultipleFutures) {
    ThreadPool pool(2);

    auto f1 = pool.submit([]() { return 10; });
    auto f2 = pool.submit([]() { return 20; });
    auto f3 = pool.submit([]() { return 30; });

    EXPECT_EQ(f1.get(), 10);
    EXPECT_EQ(f2.get(), 20);
    EXPECT_EQ(f3.get(), 30);
    pool.stop();
}

TEST(ThreadPoolTest, Issue3_SubmitWithArgumentsAndReturn) {
    ThreadPool pool(2);

    auto add = [](int a, int b) { return a + b; };
    auto future = pool.submit(add, 5, 7);

    EXPECT_EQ(future.get(), 12);
    pool.stop();
}

TEST(ThreadPoolTest, Issue3_SubmitVoidTaskCompatibility) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};

    auto future = pool.submit([&counter]() { counter++; });
    future.wait();

    EXPECT_EQ(counter, 1);
    pool.stop();
}

// F2P Test: Graceful shutdown waits for in-flight tasks
TEST(ThreadPoolTest, Issue13_GracefulShutdownWaitsForTasks) {
    ThreadPool pool(2);
    std::atomic<int> completed{0};
    std::atomic<int> started{0};

    for (int i = 0; i < 4; ++i) {
        pool.submit(std::make_unique<Task>([&completed, &started]() {
            started++;
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            completed++;
        }));
    }

    // Wait for at least 2 tasks to start (2 workers)
    while (started < 2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    pool.shutdown_graceful();

    // All started tasks should complete
    EXPECT_GE(completed, 2);
    EXPECT_LE(completed, 4);
    EXPECT_FALSE(pool.is_running());
}

// F2P Test: Immediate shutdown stops quickly
TEST(ThreadPoolTest, Issue13_ImmediateShutdownStopsQuickly) {
    ThreadPool pool(2);
    std::atomic<int> completed{0};

    for (int i = 0; i < 10; ++i) {
        pool.submit(std::make_unique<Task>([&completed]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            completed++;
        }));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto start = std::chrono::steady_clock::now();
    pool.shutdown_immediate();
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_FALSE(pool.is_running());
    EXPECT_LT(duration.count(), 200);
}

// F2P Test: Cannot submit after shutdown
TEST(ThreadPoolTest, Issue13_NoSubmitAfterShutdown) {
    ThreadPool pool(2);
    pool.start();
    pool.shutdown_graceful();

    std::atomic<int> counter{0};
    pool.submit(std::make_unique<Task>([&counter]() { counter++; }));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(counter, 0);
    EXPECT_FALSE(pool.is_running());
}

// P2P Test: Stop method still works as before
TEST(ThreadPoolTest, Issue13_StopMethodBackwardCompatible) {
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

// P2P Test: Multiple shutdowns are safe
TEST(ThreadPoolTest, Issue13_MultipleShutdownsSafe) {
    ThreadPool pool(2);
    pool.start();

    pool.shutdown_graceful();
    pool.shutdown_graceful();
    pool.shutdown_immediate();

    EXPECT_FALSE(pool.is_running());
}
