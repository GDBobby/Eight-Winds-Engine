#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"

#include <type_traits>
#include <functional>
#include <cassert>
#include <thread>
#include <mutex>


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
    //not thread safe

    //each semaphore and fence could track which queue it's being used in, but I don't think that's the best approach


    struct RenderSyncData {
    private:
        std::mutex waitMutex{};
        std::mutex signalMutex{};
        VkDevice device;
        std::vector<SemaphoreData*> previousWaits[MAX_FRAMES_IN_FLIGHT]{};
        std::vector<SemaphoreData*> previousSignals[MAX_FRAMES_IN_FLIGHT]{};
    public:
        VkFence inFlight[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        VkSemaphore imageAvailable[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        VkSemaphore renderFinished[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        std::vector<SemaphoreData*> waitSemaphores{};
        std::vector<SemaphoreData*> signalSemaphores{};

        RenderSyncData(VkDevice device);
        ~RenderSyncData();
        void AddWaitSemaphore(SemaphoreData* semaphore);
        void AddSignalSemaphore(SemaphoreData* semaphore);
        std::vector<VkSemaphore> GetWaitData(uint8_t frameIndex);
        std::vector<VkSemaphore> GetSignalData(uint8_t frameIndex);
    };


    //this is explicitly for VkFence, Semaphore, and Event (i don't know yet if event will fit here)
    class SyncPool{

        FenceData* fences;
        SemaphoreData* semaphores;

        const uint8_t size;
        VkDevice device;

    public:
        SyncPool(uint8_t size, VkDevice device);

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
        void CheckFencesForCallbacks(std::mutex& transferPoolMutex);

        SemaphoreData* GetSemaphore();
        FenceData& GetFence();
        //FenceData& GetFenceSignal(Queue::Enum queue);
        
    };
} //namespace EWE