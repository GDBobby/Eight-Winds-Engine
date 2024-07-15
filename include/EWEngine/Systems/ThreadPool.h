#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <cassert>

namespace EWE {
    //i want this to be statically accessible, aka dont want to pass around a reference or pointer
    //id prefer to make it data oriented rather than a class, but that's a little complicated with templates

    class ThreadPool {
    private:
        static ThreadPool* singleton;
        std::vector<std::thread> threads{};
        std::queue<std::function<void()>> tasks{};
        std::mutex queueMutex{};
        std::mutex counterMutex{};
        std::condition_variable condition{};
        std::condition_variable counterCondition{};
        std::size_t numTasksEnqueued{ 0 };
        std::size_t numTasksCompleted{ 0 };
        bool stop{ false };

        static bool WaitCondition();
        explicit ThreadPool(std::size_t numThreads);
        ~ThreadPool();

    public:
        static void Construct();
        static void Deconstruct();

        template<typename F, typename... Args>
        static auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F(Args...)>::type> {
            using return_type = typename std::invoke_result<F(Args...)>::type;
            auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            printf("why is task being returned 1\n");
            return task();
            printf("why is task being returned 2\n");

            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(singleton->queueMutex);
                assert(!singleton->stop && "enqueue on stopped threadpool");
                singleton->tasks.emplace([task]() { (*task)(); });

                // Increment the number of tasks enqueued
                std::unique_lock<std::mutex> counterLock(singleton->counterMutex);
                ++singleton->numTasksEnqueued;
            }
            singleton->condition.notify_one();
            return res;
        }

        static void EnqueueVoidFunction(std::function<void()> task);

        template<typename F, typename... Args>
        static void EnqueueVoid(F&& f, Args&&... args) {
            auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            EnqueueVoidFunction(task);
        }

        static void WaitForCompletion();
    };
}