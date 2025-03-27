#pragma once
#include "EWEngine/Data/KeyValueContainer.h"

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
        KeyValueContainer<std::thread::id, std::queue<std::function<void()>>, true> threadSpecificTasks;
        static thread_local std::queue<std::function<void()>>* localThreadTasks;
        static thread_local int myThreadIndex;

        std::vector<std::mutex> threadMutexesBase;
        KeyValueContainer<std::thread::id, std::mutex*, true> threadSpecificMutex;

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
        enum TaskType {
            Task_None = 0,
            Task_ThreadLocal,
            Task_Generic,
        };

        template<typename F, typename... Args>
        static void EnqueueForEachThread(F&& f, Args&&... args) {
            assert(singleton != nullptr);
            const std::thread::id thisThreadID = std::this_thread::get_id();

            for (auto& thread : singleton->threads) {
                assert(thisThreadID != thread.get_id());
                singleton->threadSpecificTasks.GetValue(thread.get_id()).push(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            }
            for (auto& thread : singleton->threads) {
                thread.join();
            }
        }
        template<typename Value>
        static auto GetKVContainerWithThreadIDKeys() {
            KeyValueContainer<std::thread::id, Value, true> ret{};
            ret.reserve(singleton->threads.size());

            for (auto& threadID : singleton->threads) {
                ret.Add(threadID.get_id());
            }
            return ret;
        }

        template<typename F, typename... Args>
        void GiveTaskToAThread(std::thread::id id, F&& func, Args&&... args) {
            auto task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
        }
        void GiveTaskToAThread(std::thread::id id, std::function<void()> task);

        static void Construct();
        static void Deconstruct();

        static void EnqueueVoidFunction(std::function<void()> task);

        template<typename F, typename... Args, typename ReturnType = std::invoke_result<F, Args...>::type>
        static auto Enqueue(F&& f, Args&&... args) {

            if constexpr (std::is_void_v<ReturnType>) {
                auto task = std::make_shared<std::packaged_task<void()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

                std::future<ReturnType> res = task->get_future();
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
            else {
                auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
                EnqueueVoidFunction(task);
            }
        }

        template<typename F, typename... Args>
        static void EnqueueVoid(F&& f, Args&&... args) {
            auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            EnqueueVoidFunction(task);
        }

        static void WaitForCompletion();
        static bool CheckEmpty();

        static std::vector<TaskType> const& GetThreadTasks() {
            return singleton->threadTasks;
        }
        private:
            std::vector<TaskType> threadTasks;
    };
}