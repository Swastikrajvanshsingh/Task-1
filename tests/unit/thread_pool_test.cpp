#include <gtest/gtest.h>

#include "taskscheduler/thread_pool.hpp"

using namespace taskscheduler;

TEST(ThreadPoolTest, ConstructsWithThreadCount) {
    ThreadPool pool(4);
    EXPECT_TRUE(true);  // Sanity test - just verify construction works
}
