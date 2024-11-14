#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"
#include "EWEngine/Graphics/PipelineBarrier.h"
#include "EWEngine/Data/CommandCallbacks.h"

#include <type_traits>
#include <functional>
#include <cassert>
#include <thread>
#include <mutex>
#include <array>


/*
* std::this_thread::sleep_for(std::chrono::microseconds(1));
    this is a small amount of time, 
    and most likely 
    it'll allow the OS to take over the thread and do other operations on it, 
    then return control of this thead to our program.
    It's not 100% for the OS to take over, but if it does, the thread will potentially sleep for longer (up to multiple milliseconds)

    it is possible to spin the thread until a certain amount of time has passed
    BUT if immediate control of a new fence/semaphore is required,
    it's recommended throw an error if the requested data is not available, and increase pool size as needed
*/

namespace EWE{




    struct FenceData {
        VkFence fence{ VK_NULL_HANDLE };
        CommandCallbacks callbackData{};
        bool inUse{ false };
        std::vector<SemaphoreData*> waitSemaphores{}; //each wait could potentially be signaled multiple times in a single queue, and then multiple queues
        SemaphoreData* signalSemaphores[Queue::_count] = { nullptr, nullptr, nullptr, nullptr }; //each signal is unique per submit that could wait on it, and right now I'm expecting max 1 wait per queue

        CommandCallbacks WaitReturnCallbacks(VkDevice device, uint64_t time);
        void Lock() {
            mut.lock();
        }
        void Unlock() {
            mut.unlock();
        }
    private:
        std::mutex mut{};
    };

    //not thread safe

    //each semaphore and fence could track which queue it's being used in, but I don't think that's the best approach

    struct WaitData {
        std::vector<SemaphoreData*> semaphores{};
        std::vector<VkPipelineStageFlags> waitDstMask{};

        //for submission, the lifetime needs to be controlled here
        //theres probably a better way to control lifetime but i cant think of it right now
        //main concern is repeatedly reallocating memory, which this might avoid depending on how the vector is handled behind the scenes
        //an alternative solution is using an array, with a max size of say 16, and using a counter to keep track of the active count, rather than vectors
        std::vector<VkSemaphore> semaphoreData{}; 
    };


    struct RenderSyncData {
    private:
        std::mutex waitMutex{};
        std::mutex signalMutex{};
        VkDevice device;
        WaitData previousWait[MAX_FRAMES_IN_FLIGHT]{};
        std::vector<SemaphoreData*> previousSignals[MAX_FRAMES_IN_FLIGHT]{};
    public:
        VkFence inFlight[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        VkSemaphore imageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        VkSemaphore renderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        WaitData waitData{};
        std::vector<SemaphoreData*> signalSemaphores{};

        RenderSyncData(VkDevice device);
        ~RenderSyncData();
        void AddWaitSemaphore(SemaphoreData* semaphore, VkPipelineStageFlags waitDstStageMask);
        void AddSignalSemaphore(SemaphoreData* semaphore);
        void SetWaitData(uint8_t frameIndex, VkSubmitInfo& submitInfo);
        std::vector<VkSemaphore> GetSignalData(uint8_t frameIndex);
    };


    //this is explicitly for VkFence, Semaphore, and Event (i don't know yet if event will fit here)
    class SyncPool{
    private:
        const uint8_t size;
        VkDevice device;

        FenceData* fences;
        SemaphoreData* semaphores;
        std::array<CommandBufferData*, Queue::_count> cmdBufs;

        //acquisition mutexes to ensure multiple threads dont acquire a single object
        std::mutex fenceAcqMut{};
        std::mutex semAcqMut{};
        std::mutex cmdBufAcqMut{};

        std::array<VkCommandPool, Queue::_count> cmdPools;


        void HandleCallbacks(std::vector<CommandCallbacks> callbacks);

    public:
        SyncPool(uint8_t size, VkDevice device, std::array<VkCommandPool, Queue::_count>& cmdPools);

        ~SyncPool();

        SemaphoreData& GetSemaphoreData(VkSemaphore semaphore);
        void SemaphoreBeginWaiting(VkSemaphore semaphore){
            GetSemaphoreData(semaphore).BeginWaiting();
        }
        void SemaphoreFinishedWaiting(VkSemaphore semaphore){
            GetSemaphoreData(semaphore).FinishWaiting();  
        }
        void SemaphoreFinishedSignaling(VkSemaphore semaphore){
            GetSemaphoreData(semaphore).FinishSignaling();
        }
#if SEMAPHORE_TRACKING
        void SemaphoreBeginSignaling(VkSemaphore semaphore, const char* name){
            GetSemaphoreData(semaphore).BeginSignaling(name);
        }
#else
        void SemaphoreBeginSignaling(VkSemaphore semaphore) {
            GetSemaphoreData(semaphore).BeginSignaling();
        }
#endif
        
        bool CheckFencesForUsage();
        void CheckFencesForCallbacks();
#if SEMAPHORE_TRACKING
        SemaphoreData* GetSemaphoreForSignaling(const char* signalName);
#else
        SemaphoreData* GetSemaphoreForSignaling();
#endif
        //this locks the fence as well
        FenceData& GetFence();
        CommandBufferData& GetCmdBufSingleTime(Queue::Enum queue);
        void ResetCommandBuffer(CommandBufferData& cmdBuf, Queue::Enum queue);
        void ResetCommandBuffer(CommandBufferData& cmdBuf);
        void ResetCommandBuffers(std::vector<CommandBufferData*>& cmdBufs, Queue::Enum queue);
        void ResetCommandBuffers(std::vector<CommandBufferData*>& cmdBufs);

        static void (*SubmitGraphicsAsync)(CommandBufferData&, std::vector<SemaphoreData*>);
    };
} //namespace EWE