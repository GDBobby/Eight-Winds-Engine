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
        bool waiting{ false };
        bool signaling{ false };

        bool Idle() const {
            return !(waiting || signaling);
        }
        void FinishSignaling() {
#ifdef _DEBUG
            assert(signaling == true && "finishing a signal that wasn't signaled");
#endif
            signaling = false;
        }
        void FinishWaiting() {
#ifdef _DEBUG
            assert(waiting == true && "finished waiting when not waiting");
#endif
            waiting = false;
        }
        void BeginWaiting() {
#ifdef _DEBUG
            assert(waiting == false && "attempting to begin wait while waiting");
#endif
            waiting = true;
        }
        void BeginSignaling() {
#ifdef _DEBUG
            assert(signaling == false && "attempting to signal while signaled");
#endif
            signaling = true;
        }
    };

    struct FenceData {
        VkFence fence{ VK_NULL_HANDLE };
        std::function<void()> callback{ nullptr }; //i think this is a function pointer, if I can't set it to null, I need to make it a pointer
        bool inUse{ false };
        std::vector<SemaphoreData*> waitSemaphores{}; //each wait could potentially be signaled multiple times in a single queue, and then multiple queues
        SemaphoreData* signalSemaphores[Queue::_count] = { nullptr, nullptr, nullptr, nullptr }; //each signal is unique per submit that could wait on it, and right now I'm expecting max 1 wait per queue

        std::function<void()> Reset(VkDevice device);
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
        StagingBuffer(VkDeviceSize size, VkDevice device);
        StagingBuffer(VkDeviceSize size, VkDevice device, const void* data);
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