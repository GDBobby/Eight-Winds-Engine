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

        RenderSyncData(VkDevice device) : device{ device } {
            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.pNext = nullptr;
            fenceInfo.flags = 0;

            VkSemaphoreCreateInfo semInfo{};
            semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semInfo.pNext = nullptr;
            for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                EWE_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &inFlight[i]));
                EWE_VK_ASSERT(vkCreateSemaphore(device, &semInfo, nullptr, &imageAvailable[i]));
                EWE_VK_ASSERT(vkCreateSemaphore(device, &semInfo, nullptr, &renderFinished[i]));
            }
        }
        ~RenderSyncData(){
            for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroyFence(device, inFlight[i], nullptr);
                vkDestroySemaphore(device, imageAvailable[i], nullptr);
                vkDestroySemaphore(device, renderFinished[i], nullptr);
            }
        }
        void AddWaitSemaphore(SemaphoreData* semaphore) {
            waitMutex.lock();
            semaphore->BeginWaiting();
            waitSemaphores.push_back(semaphore);
            waitMutex.unlock();
        }
        void AddSignalSemaphore(SemaphoreData* semaphore) {
            signalMutex.lock();
            signalSemaphores.push_back(semaphore);
            signalMutex.unlock();
        }
        std::vector<VkSemaphore> GetWaitData(uint8_t frameIndex) {
            waitMutex.lock();
            for (auto& waitSem : previousWaits[frameIndex]) {
                waitSem->FinishWaiting();
            }
            previousWaits[frameIndex].clear();
            previousWaits[frameIndex] = waitSemaphores;
            waitSemaphores.clear();

            std::vector<VkSemaphore> ret{};
            ret.reserve(previousWaits[frameIndex].size() + 1);
            for (auto& waitSem : previousWaits[frameIndex]) {
                ret.push_back(waitSem->semaphore);
            }
            ret.push_back(imageAvailable[frameIndex]);
            waitMutex.unlock();
            return ret;
        }
        std::vector<VkSemaphore> GetSignalData(uint8_t frameIndex) {
            signalMutex.lock();
            for (auto& sigSem : previousSignals[frameIndex]) {
                sigSem->FinishSignaling();
            }
            previousSignals[frameIndex].clear();
            previousSignals[frameIndex] = signalSemaphores;
            signalSemaphores.clear();

            std::vector<VkSemaphore> ret{};
            ret.reserve(previousSignals[frameIndex].size() + 1);
            for (auto& sigSem : previousSignals[frameIndex]) {
                ret.push_back(sigSem->semaphore);
            }
            ret.push_back(renderFinished[frameIndex]);
            signalMutex.unlock();
            return ret;
        }
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
        void SemaphoreBeginSignaling(VkSemaphore semaphore){
            GetSemaphoreData(semaphore).BeginSignaling();
        }
        
        void CheckFences();

        SemaphoreData* GetSemaphore();
        FenceData& GetFence();
        FenceData& GetFenceSignal(Queue::Enum queue);
        
    };
} //namespace EWE