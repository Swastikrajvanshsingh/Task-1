#include <gtest/gtest.h>

#include "taskscheduler/task.hpp"

using namespace taskscheduler;

TEST(TaskTest, ExecutesCallable) {
    int counter = 0;
    Task task([&counter]() { counter++; });
    task.execute();
    EXPECT_EQ(counter, 1);
}
