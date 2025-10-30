#include <gtest/gtest.h>

#include "taskscheduler/task_queue.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <thread>

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

// F2P Test: Tasks with earlier deadlines should be scheduled first
TEST(TaskQueueTest, Issue9_DeadlineOrdering) {
    TaskQueue queue;
    std::vector<int> execution_order;

    auto now = std::chrono::steady_clock::now();

    // Task with deadline 300ms from now
    auto task1 = std::make_unique<Task>([&execution_order]() { execution_order.push_back(1); }, Priority::NORMAL);
    task1->set_deadline(now + std::chrono::milliseconds(300));

    // Task with deadline 100ms from now (should execute first)
    auto task2 = std::make_unique<Task>([&execution_order]() { execution_order.push_back(2); }, Priority::NORMAL);
    task2->set_deadline(now + std::chrono::milliseconds(100));

    // Task with deadline 200ms from now
    auto task3 = std::make_unique<Task>([&execution_order]() { execution_order.push_back(3); }, Priority::NORMAL);
    task3->set_deadline(now + std::chrono::milliseconds(200));

    queue.push(std::move(task1));
    queue.push(std::move(task2));
    queue.push(std::move(task3));

    auto t1 = queue.pop();
    auto t2 = queue.pop();
    auto t3 = queue.pop();

    t1->execute();
    t2->execute();
    t3->execute();

    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 2);  // Earliest deadline (100ms)
    EXPECT_EQ(execution_order[1], 3);  // Middle deadline (200ms)
    EXPECT_EQ(execution_order[2], 1);  // Latest deadline (300ms)
}

// F2P Test: Tasks with deadlines should take precedence over those without
TEST(TaskQueueTest, Issue9_DeadlineOverridesNoDeadline) {
    TaskQueue queue;
    std::vector<int> execution_order;

    auto now = std::chrono::steady_clock::now();

    // Task without deadline, HIGH priority
    auto task1 = std::make_unique<Task>([&execution_order]() { execution_order.push_back(1); }, Priority::HIGH);

    // Task with deadline, NORMAL priority (should execute first despite lower priority)
    auto task2 = std::make_unique<Task>([&execution_order]() { execution_order.push_back(2); }, Priority::NORMAL);
    task2->set_deadline(now + std::chrono::milliseconds(100));

    queue.push(std::move(task1));
    queue.push(std::move(task2));

    auto t1 = queue.pop();
    auto t2 = queue.pop();

    t1->execute();
    t2->execute();

    ASSERT_EQ(execution_order.size(), 2);
    EXPECT_EQ(execution_order[0], 2);  // Task with deadline executes first
    EXPECT_EQ(execution_order[1], 1);  // Task without deadline executes second
}

// F2P Test: When deadlines are equal, fall back to priority
TEST(TaskQueueTest, Issue9_EqualDeadlineFallbackToPriority) {
    TaskQueue queue;
    std::vector<int> execution_order;

    auto now = std::chrono::steady_clock::now();
    auto deadline = now + std::chrono::milliseconds(100);

    // Both tasks have same deadline, but different priorities
    auto task1 = std::make_unique<Task>([&execution_order]() { execution_order.push_back(1); }, Priority::NORMAL);
    task1->set_deadline(deadline);

    auto task2 = std::make_unique<Task>([&execution_order]() { execution_order.push_back(2); }, Priority::HIGH);
    task2->set_deadline(deadline);

    queue.push(std::move(task1));
    queue.push(std::move(task2));

    auto t1 = queue.pop();
    auto t2 = queue.pop();

    t1->execute();
    t2->execute();

    ASSERT_EQ(execution_order.size(), 2);
    EXPECT_EQ(execution_order[0], 2);  // HIGH priority executes first
    EXPECT_EQ(execution_order[1], 1);  // NORMAL priority executes second
}

// P2P Test: Priority ordering still works when no deadlines are set
TEST(TaskQueueTest, Issue9_PriorityStillWorksWithoutDeadlines) {
    TaskQueue queue;
    std::vector<int> execution_order;

    queue.push(std::make_unique<Task>([&execution_order]() { execution_order.push_back(1); }, Priority::LOW));
    queue.push(std::make_unique<Task>([&execution_order]() { execution_order.push_back(2); }, Priority::HIGH));
    queue.push(std::make_unique<Task>([&execution_order]() { execution_order.push_back(3); }, Priority::CRITICAL));

    auto t1 = queue.pop();
    auto t2 = queue.pop();
    auto t3 = queue.pop();

    t1->execute();
    t2->execute();
    t3->execute();

    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 3);  // CRITICAL
    EXPECT_EQ(execution_order[1], 2);  // HIGH
    EXPECT_EQ(execution_order[2], 1);  // LOW
}

// P2P Test: Task deadline getters work correctly
TEST(TaskQueueTest, Issue9_DeadlineGettersWork) {
    auto task1 = std::make_unique<Task>([]() {}, Priority::NORMAL);
    EXPECT_FALSE(task1->has_deadline());
    EXPECT_FALSE(task1->deadline().has_value());

    auto now = std::chrono::steady_clock::now();
    auto deadline = now + std::chrono::milliseconds(100);
    task1->set_deadline(deadline);

    EXPECT_TRUE(task1->has_deadline());
    EXPECT_TRUE(task1->deadline().has_value());
    EXPECT_EQ(task1->deadline().value(), deadline);
}
