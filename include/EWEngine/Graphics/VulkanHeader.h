#pragma once

#include "EWEngine/Graphics/Preprocessor.h"
#if USING_VMA
#include "EWEngine/Graphics/vk_mem_alloc.h"
#endif
#include "EWEngine/Data/EngineDataTypes.h"
#if DEBUGGING_DEVICE_LOST
#include "EWEngine/Graphics/VkDebugDeviceLost.h"
#endif
#if DEBUG_NAMING
#include "EWEngine/Graphics/DebugNaming.h"
#endif

#if SEMAPHORE_TRACKING
#include <string>
#endif

#include <mutex>

#include <functional>

namespace EWE{
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, const VkMemoryPropertyFlags properties);

	static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;

	namespace Queue {
		enum Enum : uint32_t {
			graphics,
			present,
			compute,
			transfer,
			_count,
		};
	} //namespace Queue

    struct SemaphoreData {
        VkSemaphore semaphore{ VK_NULL_HANDLE };
        std::string name{"null"};
        bool waiting{ false };
        bool signaling{ false };

        bool Idle() const {
            return !(waiting || signaling);
        }
        void FinishSignaling();
        void FinishWaiting();
        void BeginWaiting();
        void BeginSignaling(const char* name);
    };

    struct FenceData {
        VkFence fence{ VK_NULL_HANDLE };
        std::function<void()> asyncCallbacks{ nullptr }; //i think this is a function pointer, if I can't set it to null, I need to make it a pointer
        std::function<void()> inlineCallbacks{ nullptr };
        bool inUse{ false };
        std::vector<SemaphoreData*> waitSemaphores{}; //each wait could potentially be signaled multiple times in a single queue, and then multiple queues
        SemaphoreData* signalSemaphores[Queue::_count] = { nullptr, nullptr, nullptr, nullptr }; //each signal is unique per submit that could wait on it, and right now I'm expecting max 1 wait per queue

        std::function<void()> Reset(VkDevice device);
        std::function<void()> WaitReturnCallbacks(VkDevice device, uint64_t time);
        void Lock() {
            mut.lock();
        }
        void Unlock() {
            mut.unlock();
        }
    private:
        std::mutex mut{};
    };

    struct StagingBuffer {
        VkBuffer buffer{ VK_NULL_HANDLE };
#if USING_VMA
        VmaAllocation vmaAlloc{};
        StagingBuffer(VkDeviceSize size, VmaAllocator vmaAllocator);
        StagingBuffer(VkDeviceSize size, VmaAllocator vmaAllocator, const void* data);
        void Free(VmaAllocator vmaAllocator);
        void Free(VmaAllocator vmaAllocator) const;
        void Stage(VmaAllocator vmaAllocator, const void* data, uint64_t bufferSize);
#else
        VkDeviceMemory memory{ VK_NULL_HANDLE };
        StagingBuffer(VkDeviceSize size, VkPhysicalDevice physicalDevice, VkDevice device);
        StagingBuffer(VkDeviceSize size, VkPhysicalDevice physicalDevice, VkDevice device, const void* data);
        void Free(VkDevice device);
        void Free(VkDevice device) const;
        void Stage(VkDevice device, const void* data, VkDeviceSize bufferSize);
#endif
    };

} //namespace EWE



#if GPU_LOGGING
#include <fstream>
#define GPU_LOG_FILE "GPULog.log"

#ifndef EWE_VK_RESULT_ASSERT
#if DEBUGGING_DEVICE_LOST
#define EWE_VK_RESULT_ASSERT(result)                                                                                            \
                    if(result == VK_ERROR_DEVICE_LOST){ EWE::VKDEBUG::OnDeviceLost();}                                                  \
                    else if (result != VK_SUCCESS) {                                                                                    \
                        printf("VK_ERROR : %s(%d) : %s - %d \n", __FILE__, __LINE__, __FUNCTION__, result);                             \
                        std::ofstream logFile{};                                                                                        \
                        logFile.open(GPU_LOG_FILE, std::ios::app);                                                                      \
                        assert(logFile.is_open() && "Failed to open log file");                                                         \
                        logFile << "VK_ERROR : " << __FILE__ << '(' << __LINE__ << ") : " << __FUNCTION__ << " : VkResult(" << result << ")\n";   \
                        logFile.close();                                                                                                \
                        assert(result == VK_SUCCESS && "VK_ERROR");                                                                     \
	                }
#else
#define EWE_VK_RESULT_ASSERT(result)                                                                                            \
                    if (result != VK_SUCCESS) {                                                                                         \
                        printf("VK_ERROR : %s(%d) : %s - %d \n", __FILE__, __LINE__, __FUNCTION__, result);                             \
                        std::ofstream logFile{};                                                                                        \
                        logFile.open(GPU_LOG_FILE, std::ios::app);                                                                      \
                        assert(logFile.is_open() && "Failed to open log file");                                                         \
                        logFile << "VK_ERROR : " << __FILE__ << '(' << __LINE__ << ") : " << __FUNCTION__ << " : VkResult(" << result << ")\n";   \
                        logFile.close();                                                                                                \
                        assert(result == VK_SUCCESS && "VK_ERROR");                                                                     \
	                }
#endif
#endif

#ifndef EWE_VK_ASSERT
#define EWE_VK_ASSERT(vkFunc)       \
        {VkResult result = (vkFunc);    \
        EWE_VK_RESULT_ASSERT(result)}
#endif
#else
#ifndef EWE_VK_RESULT_ASSERT
#define EWE_VK_RESULT_ASSERT(result)                                                        \
        if (result != VK_SUCCESS) {                                                             \
            printf("VK_ERROR : %s(%d) : %s - %l\n", __FILE__, __LINE__, __FUNCTION__, result);  \
            assert(result == VK_SUCCESS && "VK_ERROR");                                         \
	    }
#endif

#ifndef EWE_VK_ASSERT
#define EWE_VK_ASSERT(vkFunc)       \
    {VkResult result = (vkFunc);        \
        EWE_VK_RESULT_ASSERT(result)}
#endif
#endif