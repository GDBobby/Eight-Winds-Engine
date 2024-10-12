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

#include <type_traits>
#include <concepts>

namespace EWE{
    struct TransferCallbackReturn {
        std::function<void()> freeCommandBufferCallback{ nullptr };
        std::function<void()> otherCallbacks{ nullptr };
    };

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
#if SEMAPHORE_TRACKING
        std::string name{"null"};
        VkDevice device;
#endif
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
        TransferCallbackReturn transferCallbacks{};
        std::function<void()> inlineCallbacks{ nullptr };
        bool inUse{ false };
        std::vector<SemaphoreData*> waitSemaphores{}; //each wait could potentially be signaled multiple times in a single queue, and then multiple queues
        SemaphoreData* signalSemaphores[Queue::_count] = { nullptr, nullptr, nullptr, nullptr }; //each signal is unique per submit that could wait on it, and right now I'm expecting max 1 wait per queue

        TransferCallbackReturn WaitReturnCallbacks(VkDevice device, uint64_t time);
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
#endif

#define WRAPPING_VULKAN_FUNCTIONS false

#if CALL_TRACING
void EWE_VK_RESULT(VkResult vkResult, const std::source_location& sourceLocation = std::source_location::current());

//if having difficulty with template errors related to this function, define the vulkan function by itself before using this function to ensure its correct
template<typename F, typename... Args>
struct EWE_VK {
    EWE_VK(F&& f, Args&&... args, const std::source_location& sourceLocation = std::source_location::current()) {
#if WRAPPING_VULKAN_FUNCTIONS
        //call a preliminary function
#endif
        if constexpr (std::is_void_v<decltype(std::forward<F>(f)(std::forward<Args>(args)...))>) {
            //std::bind(std::forward<F>(f), std::forward<Args>(args)...)(); //std bind is constexpr, might be worth using
            std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        }
        else {
            VkResult vkResult = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            EWE_VK_RESULT(vkResult, sourceLocation);

        }
#if WRAPPING_VULKAN_FUNCTIONS
        //call a following function
#endif
    }
};
template<typename F, typename... Args>
EWE_VK(F&& f, Args&&...) -> EWE_VK<F, Args...>;

#else
//if having difficulty with template errors related to this function, define the vulkan function by itself before using this function to ensure its correct
void EWE_VK_RESULT(VkResult vkResult) {
#if DEBUGGING_DEVICE_LOST                                                                                        
    if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
    else
#else
    if (vkResult != VK_SUCCESS) {
        printf("VK_ERROR : %d \n", vkResult);
        std::ofstream logFile{};
        logFile.open(GPU_LOG_FILE, std::ios::app);
        assert(logFile.is_open() && "Failed to open log file");
        logFile << "VK_ERROR : VkResult(" << vkResult << ")\n";
        logFile.close();
        assert(vkResult == VK_SUCCESS && "VK_ERROR");
    }
#endif
}

template<typename F, typename... Args>
void EWE_VK(F&& f, Args&&... args) {
#if WRAPPING_VULKAN_FUNCTIONS
    //call a preliminary function
#endif
    if constexpr (std::is_same_v<std::invoke_result<F(Args...)>, void>) {
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        //f(args);
    }
    else {
        //VkResult vkResult = std::forward<F>(f)(std::forward<Args>(args)...);
        VkResult vkResult = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);

        if (vkResult != VK_SUCCESS) {
#if DEBUGGING_DEVICE_LOST                                                                                        
            if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
            else
#else
            if (vkResult != VK_SUCCESS) {
                printf("VK_ERROR : %d \n", vkResult);
                std::ofstream logFile{};
                logFile.open(GPU_LOG_FILE, std::ios::app);
                assert(logFile.is_open() && "Failed to open log file");
                logFile << "VK_ERROR : VkResult(" << vkResult << ")\n";
                logFile.close();
                assert(vkResult == VK_SUCCESS && "VK_ERROR");
            }
#endif
        }
    }
#if WRAPPING_VULKAN_FUNCTIONS
    //call a following function
#endif
    }
#endif