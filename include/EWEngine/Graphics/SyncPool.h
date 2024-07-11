#pragma once

#include "EWEngine/Global_Macros.h"

#include "EWEngine/Graphics/VulkanHeader.h"

#include <type_traits>
#include <functional>
#include <cassert>
#include <thread>


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

    struct SemaphoreData{
        VkSemaphore semaphore{VK_NULL_HANDLE};
        bool waiting{false};
        bool signaling{false};

        bool Idle() const{
            return !(waiting || signaling);
        }
        void FinishSignaling(){
#ifdef _DEBUG
            assert(signaling == true && "finishing a signal that wasn't signaled");
#endif
            signaling = false;
        }
        void FinishWaiting(){
#ifdef _DEBUG
            assert(waiting == true && "finished waiting when not waiting");
#endif
            waiting = false;
        }
        void BeginWaiting(){
#ifdef _DEBUG
            assert(waiting == false && "attempting to begin wait while waiting");
#endif
            waiting = true;
        }
        void BeginSignaling(){
#ifdef _DEBUG
            assert(signaling == false && "attempting to signal while signaled");
#endif
            signaling = true;
        }
    };
    struct FenceData{
        VkFence fence{VK_NULL_HANDLE};
        std::function<void()> callback{nullptr}; //i think this is a function pointer, if I can't set it to null, I need to make it a pointer
        bool inUse{false};
        SemaphoreData* waitSemaphore{nullptr};
        SemaphoreData* signalSemaphore{nullptr};

        SemaphoreData* Reset(VkDevice device);
        void PrepareSubmitInfo(VkSubmitInfo& submitInfo);
    };
    struct RenderSyncData {
    private:
        std::mutex waitMutex{};
        std::mutex signalMutex{};
    public:
        VkFence inFlight[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        VkSemaphore imageAvailable[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        VkSemaphore renderFinished[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        std::vector<VkSemaphore> waitSemaphores{};
        std::vector<VkSemaphore> signalSemaphores{};

        RenderSyncData(VkDevice device) {
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
        void AddWaitSemaphore(VkSemaphore semaphore) {
            waitMutex.lock();
            waitSemaphores.push_back(semaphore);
            waitMutex.unlock();
        }
        void AddSignalSemaphore(VkSemaphore semaphore) {
            signalMutex.lock();
            signalSemaphores.push_back(semaphore);
            signalMutex.unlock();
        }
        std::vector<VkSemaphore> GetWaitData(uint8_t frameIndex) {
            waitMutex.lock();
            std::vector<VkSemaphore> ret{waitSemaphores};
            waitSemaphores.clear();
            waitMutex.unlock();
            ret.push_back(imageAvailable[frameIndex]);
            return ret;
        }
        std::vector<VkSemaphore> GetSignalData(uint8_t frameIndex) {
            signalMutex.lock();
            std::vector<VkSemaphore> ret{ signalSemaphores };
            signalSemaphores.clear();
            signalMutex.unlock();
            ret.push_back(imageAvailable[frameIndex]);
            return ret;
        }
    };


    //this is explicitly for VkFence, Semaphore, and Event (i don't know yet if event will fit here)
    class SyncPool{

        FenceData* fences;
        SemaphoreData* semaphores;
        SemaphoreData* transitionSemaphores;

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

        SemaphoreData& GetSemaphore();
        FenceData& GetFence(Queue::Enum queue);
        FenceData& GetFenceSignal(Queue::Enum queue);
        
    };
} //namespace EWE