#include <gtest/gtest.h>

#include "taskscheduler/task_queue.hpp"

using namespace taskscheduler;

TEST(TaskQueueTest, PushAndPop) {
    TaskQueue queue;
    int counter = 0;
    queue.push(std::make_unique<Task>([&counter]() { counter++; }));

    auto task = queue.pop();
    ASSERT_NE(task, nullptr);
    task->execute();
    EXPECT_EQ(counter, 1);
}

TEST(TaskQueueTest, Size) {
    TaskQueue queue;
    EXPECT_EQ(queue.size(), 0);

    queue.push(std::make_unique<Task>([]() {}));
    EXPECT_EQ(queue.size(), 1);

    queue.push(std::make_unique<Task>([]() {}));
    EXPECT_EQ(queue.size(), 2);
}
