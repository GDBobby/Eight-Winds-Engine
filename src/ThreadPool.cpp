#include "EWEngine/Systems/ThreadPool.h"

#include <cstdint>

#define DEBUGGING_THREADS true

namespace EWE {
    ThreadPool* ThreadPool::singleton{ nullptr };
    void ThreadPool::Construct() {
        assert(singleton == nullptr && "constructing Threadpool twice");
        singleton = new ThreadPool(std::thread::hardware_concurrency() - 2);
        //1 thread reserved for graphics, which is the calling thread
        //1 thread is reserved for transfer executive, i need to come back and see if an executive transfer thread is necessary, or if it could be considered a worker thread
        //might be necessary to reserve a thread for compute, and logic. not sure if they can also be considered worker threads
    }
    void ThreadPool::Deconstruct() {
        delete singleton;
    }

    ThreadPool::ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            threads.emplace_back(
                [this] {
                    while (true) {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock,
                            [this] {
                                return this->stop || !this->tasks.empty();
                            }
                        );
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }
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
}//namespace EWE