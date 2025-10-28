#include <gtest/gtest.h>

#include "taskscheduler/task_queue.hpp"
#include <vector>
#include <string>

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

TEST(TaskQueueTest, Issue5_PriorityOrdering) {
    TaskQueue queue;
    std::vector<int> execution_order;

    queue.push(std::make_unique<Task>([&execution_order]() { execution_order.push_back(1); }, Priority::NORMAL));
    queue.push(std::make_unique<Task>([&execution_order]() { execution_order.push_back(2); }, Priority::HIGH));
    queue.push(std::make_unique<Task>([&execution_order]() { execution_order.push_back(3); }, Priority::LOW));
    queue.push(std::make_unique<Task>([&execution_order]() { execution_order.push_back(4); }, Priority::CRITICAL));

    auto t1 = queue.pop();
    auto t2 = queue.pop();
    auto t3 = queue.pop();
    auto t4 = queue.pop();

    t1->execute();
    t2->execute();
    t3->execute();
    t4->execute();

    ASSERT_EQ(execution_order.size(), 4);
    EXPECT_EQ(execution_order[0], 4);  // CRITICAL
    EXPECT_EQ(execution_order[1], 2);  // HIGH
    EXPECT_EQ(execution_order[2], 1);  // NORMAL
    EXPECT_EQ(execution_order[3], 3);  // LOW
}

TEST(TaskQueueTest, Issue5_FIFOWithinSamePriority) {
    TaskQueue queue;
    std::vector<int> execution_order;

    queue.push(std::make_unique<Task>([&execution_order]() { execution_order.push_back(1); }, Priority::HIGH));
    queue.push(std::make_unique<Task>([&execution_order]() { execution_order.push_back(2); }, Priority::HIGH));
    queue.push(std::make_unique<Task>([&execution_order]() { execution_order.push_back(3); }, Priority::HIGH));

    auto t1 = queue.pop();
    auto t2 = queue.pop();
    auto t3 = queue.pop();

    t1->execute();
    t2->execute();
    t3->execute();

    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 1);  // First HIGH
    EXPECT_EQ(execution_order[1], 2);  // Second HIGH
    EXPECT_EQ(execution_order[2], 3);  // Third HIGH
}

TEST(TaskQueueTest, Issue5_DefaultPriorityIsNormal) {
    TaskQueue queue;

    auto task = std::make_unique<Task>([]() {});
    EXPECT_EQ(task->priority(), Priority::NORMAL);
}
