#include "EWEngine/Systems/ThreadPool.h"

#include <cstdint>

#define DEBUGGING_THREADS true

namespace EWE {
    ThreadPool* ThreadPool::singleton{ nullptr };

    thread_local std::queue<std::function<void()>>* ThreadPool::localThreadTasks{};

    void ThreadPool::Construct() {
        assert(singleton == nullptr && "constructing Threadpool twice");
        singleton = new ThreadPool(std::thread::hardware_concurrency() - 1);
    }
    void ThreadPool::Deconstruct() {
        delete singleton;
    }

    ThreadPool::ThreadPool(std::size_t numThreads) : threadSpecificTasks{}, threadMutexesBase{numThreads}, threadSpecificMutex{} {

        threadSpecificTasks.reserve(numThreads);
        threadSpecificMutex.reserve(numThreads);

        for (std::size_t i = 0; i < numThreads; ++i) {

            threads.emplace_back(
                [this, localThreadMutex = &threadMutexesBase[i]] {
                    const std::thread::id myThreadID = std::this_thread::get_id();

                    threadSpecificTasks.Lock();
                    localThreadTasks = &threadSpecificTasks.Add(myThreadID);
                    threadSpecificTasks.Unlock();

                    threadSpecificMutex.Add(myThreadID, localThreadMutex);

                    while (true) {
                        while(localThreadTasks->size() != 0){
                            localThreadMutex->lock();
                            auto func = localThreadTasks->front();
                            localThreadTasks->pop();
                            localThreadMutex->unlock();
                            func();
                        }

                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock,
                            [this] {
                                return this->stop || !this->tasks.empty() || (this->localThreadTasks->size() != 0);
                            }
                        );
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }
                        if (tasks.size() == 0) {
                            lock.unlock();
                            assert(localThreadTasks->size() != 0);
                            while (localThreadTasks->size() != 0) {
                                localThreadMutex->lock();
                                auto func = localThreadTasks->front();
                                localThreadTasks->pop();
                                localThreadMutex->unlock();
                                func();
                            }
                        }
                        else {
                            auto task = std::move(this->tasks.front());
                            this->tasks.pop();
                            lock.unlock();
#if DEBUGGING_THREADS
                            printf("thread[%u] doing a task\n", std::this_thread::get_id());
#endif

                            task();
#if DEBUGGING_THREADS
                            printf("thread[%u] finished task\n", std::this_thread::get_id());
#endif

                            // Increment the number of tasks completed
                            std::unique_lock<std::mutex> counterLock(this->counterMutex);
                            this->numTasksCompleted++;

#if DEBUGGING_THREADS
                            printf("task completed:enqueued - %zu:%zu\n", this->numTasksCompleted, this->numTasksEnqueued);
#endif
                            // Notify any waiting threads if all tasks have completed
                            if (this->numTasksCompleted == this->numTasksEnqueued) {
                                this->counterCondition.notify_all();
                            }
                        }
                    }
                }
            );
        }
    }
    ThreadPool::~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& thread : threads) {
            thread.join();
        }
    }

    bool ThreadPool::WaitCondition() {
        std::unique_lock<std::mutex> counterLock(singleton->counterMutex);
#if DEBUGGING_THREADS
        printf("waiting for completion of thread pool - %zu:%zu \n", singleton->numTasksCompleted, singleton->numTasksEnqueued);
#endif
        if (singleton->numTasksCompleted == singleton->numTasksEnqueued) {
            singleton->numTasksCompleted = 0;
            singleton->numTasksEnqueued = 0;
            return true;
        }
        return false;
    }

    void ThreadPool::WaitForCompletion() {
#if EWE_DEBUG
        std::thread::id thisThreadID = std::this_thread::get_id();
        for (auto& eachThread : threads) {
            assert(eachThread.get_id() != thisThreadID);
        }
#endif

        std::unique_lock<std::mutex> counterLock(singleton->counterMutex);
        singleton->counterCondition.wait(counterLock,
            [&] {
#if DEBUGGING_THREADS
                printf("waiting for completion of thread pool - %zu:%zu \n", singleton->numTasksCompleted, singleton->numTasksEnqueued);
#endif
                if (singleton->numTasksCompleted == singleton->numTasksEnqueued) {
                    singleton->numTasksCompleted = 0;
                    singleton->numTasksEnqueued = 0;
                    return true;
                }
                return false;
            }

        );
    }
    bool ThreadPool::CheckEmpty() {
        return WaitCondition();
    }

    void ThreadPool::EnqueueVoidFunction(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(singleton->queueMutex);
            assert(!singleton->stop && "enqueue on stopped threadpool");
            singleton->tasks.emplace(task);

            // Increment the number of tasks enqueued
            std::unique_lock<std::mutex> counterLock(singleton->counterMutex);
            ++singleton->numTasksEnqueued;
        }
        singleton->condition.notify_one();
    }

    void ThreadPool::GiveTaskToAThread(std::thread::id id, std::function<void()> task) {
        std::mutex* threadMut = threadSpecificMutex.GetValue(id);
        threadMut->lock();
        threadSpecificTasks.Lock();
        threadSpecificTasks.GetValue(id).push(task);
        threadSpecificTasks.Unlock();
        threadMut->unlock();
    }
}//namespace EWE