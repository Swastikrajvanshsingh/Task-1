#include <gtest/gtest.h>

#include "taskscheduler/dependency_tracker.hpp"
#include <atomic>

using namespace taskscheduler;

TEST(DependencyTrackerTest, Issue7_TaskWithoutDependenciesReady) {
    DependencyTracker tracker;
    std::atomic<int> counter{0};

    auto task = std::make_unique<Task>([&counter]() { counter++; });
    tracker.assign_id(task);

    EXPECT_TRUE(task->dependencies().empty());
}

TEST(DependencyTrackerTest, Issue7_TaskExecutesAfterDependency) {
    DependencyTracker tracker;
    std::vector<int> execution_order;

    auto task1 = std::make_unique<Task>([&execution_order]() { execution_order.push_back(1); });
    TaskId id1 = tracker.assign_id(task1);

    auto task2 = std::make_unique<Task>([&execution_order]() { execution_order.push_back(2); }, Priority::NORMAL, std::vector<TaskId>{id1});
    tracker.assign_id(task2);

    tracker.add_task(std::move(task2));

    auto ready = tracker.get_ready_tasks();
    EXPECT_EQ(ready.size(), 0);

    task1->execute();
    tracker.mark_completed(id1);

    ready = tracker.get_ready_tasks();
    ASSERT_EQ(ready.size(), 1);
    ready[0]->execute();

    ASSERT_EQ(execution_order.size(), 2);
    EXPECT_EQ(execution_order[0], 1);
    EXPECT_EQ(execution_order[1], 2);
}

TEST(DependencyTrackerTest, Issue7_MultipleDependenciesResolved) {
    DependencyTracker tracker;
    std::atomic<int> counter{0};

    auto task1 = std::make_unique<Task>([&counter]() { counter++; });
    TaskId id1 = tracker.assign_id(task1);

    auto task2 = std::make_unique<Task>([&counter]() { counter++; });
    TaskId id2 = tracker.assign_id(task2);

    auto task3 = std::make_unique<Task>([&counter]() { counter++; }, Priority::NORMAL, std::vector<TaskId>{id1, id2});
    tracker.assign_id(task3);
    tracker.add_task(std::move(task3));

    tracker.mark_completed(id1);
    auto ready = tracker.get_ready_tasks();
    EXPECT_EQ(ready.size(), 0);

    tracker.mark_completed(id2);
    ready = tracker.get_ready_tasks();
    EXPECT_EQ(ready.size(), 1);
}
