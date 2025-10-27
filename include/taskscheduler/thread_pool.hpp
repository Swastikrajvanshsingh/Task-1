#ifndef TASKSCHEDULER_THREAD_POOL_HPP
#define TASKSCHEDULER_THREAD_POOL_HPP

#include "task_queue.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <future>
#include <functional>

namespace taskscheduler {

/**
 * Thread pool with fixed worker count.
 * Processes tasks from a shared queue using multiple worker threads.
 */
class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void start();
    void stop();
    void submit(std::unique_ptr<Task> task);

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

    size_t thread_count() const;
    size_t pending_tasks() const;
    bool is_running() const;

private:
    void worker_loop();

    TaskQueue task_queue_;
    std::vector<std::thread> threads_;
    std::atomic<bool> running_{false};
    size_t num_threads_;
};

// Template implementation
template<typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();

    submit(std::make_unique<Task>([task]() { (*task)(); }));

    return result;
}

} // namespace taskscheduler

#endif // TASKSCHEDULER_THREAD_POOL_HPP
