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

        SemaphoreData* Reset(VkDevice device){
            EWE_VK_ASSERT(vkResetFences(device, 1, &fence));
            if(waitSemaphore != nullptr){
                waitSemaphore->FinishWaiting();
                waitSemaphore = nullptr;
            }
            if(signalSemaphore != nullptr){
                signalSemaphore->FinishSignaling();
                if(!signalSemaphore->waiting){
                    inUse = false;
                    SemaphoreData* retData = signalSemaphore;
                    signalSemaphore = nullptr;
                    return signalSemaphore;
                }
                signalSemaphore = nullptr;
            }
            inUse = false;
            return nullptr;
        }
        void PrepareSubmitInfo(VkSubmitInfo& submitInfo){
            submitInfo.pSignalSemaphores = &signalSemaphore->semaphore;
            submitInfo.signalSemaphoreCount = signalSemaphore != nullptr;
            
            submitInfo.pWaitSemaphores = &waitSemaphore->semaphore;
            submitInfo.waitSemaphoreCount = waitSemaphore != nullptr;
        }
    };


    //this is explicitly for VkFence, Semaphore, and Event (i don't know yet if event will fit here)
    class SyncPool{

        FenceData* fences;
        SemaphoreData* semaphores;
        SemaphoreData* lastSignaledSemaphore[Queue::_count];

        const uint8_t size;
        VkDevice device;

        public:
        SyncPool(uint8_t size, VkDevice device) : 
            size{size}, 
            device{device}, 
            fences{new FenceData[size]}, 
            semaphores{new SemaphoreData[size * 2]} 
        {
            assert(size <= 64 && "this isn't optimized very well, don't use big size"); //big size probably also isn't necessary

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.flags = 0;
            fenceInfo.pNext = nullptr;
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            for(uint16_t i = 0; i < size; i++){
                EWE_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &fences[i].fence));
            }

            VkSemaphoreCreateInfo semInfo{};
            semInfo.flags = 0;
            semInfo.pNext = nullptr;
            semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            for(uint16_t i = 0; i < size * 2; i++){
                EWE_VK_ASSERT(vkCreateSemaphore(device, &semInfo, nullptr, &semaphores[i].semaphore));
            }
        }

        ~SyncPool(){
            delete[] fences;
            delete[] semaphores;
        }
        SemaphoreData& GetSemaphoreData(VkSemaphore semaphore){
            for(uint8_t i = 0; i < size * 2; i++){
                if(semaphores[i].semaphore == semaphore){
                    return semaphores[i];
                }
            }
            assert(false && "failed to find SemaphoreData");
            return semaphores[0]; //DO NOT return this, error silencing
            //only way for this to not return an error is if the return type is changed to pointer and nullptr is returned if not found, or std::conditional which im not a fan of
        }
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
        
        void CheckFences(){

            std::vector<std::function<void()>> callbacks{};
            for(uint16_t i = 0; i < size; i++){
                if(fences[i].inUse){
                    VkResult ret = vkWaitForFences(device, 1, &fences[i].fence, true, 0);
                    if(ret == VK_SUCCESS){
                        SemaphoreData* idleSem = fences[i].Reset(device);
                        //idleSem is nullptr if the owned signaler semaphore is also being waited on
                        //i could store the queue but it seems like a bit of a waste, in the following code im checking if it's equal, then setting to nullptr
                        lastSignaledSemaphore[Queue::graphics] = reinterpret_cast<SemaphoreData*>(reinterpret_cast<std::size_t>(lastSignaledSemaphore[Queue::graphics]) * (lastSignaledSemaphore[Queue::graphics] != idleSem));
                        lastSignaledSemaphore[Queue::transfer] = reinterpret_cast<SemaphoreData*>(reinterpret_cast<std::size_t>(lastSignaledSemaphore[Queue::transfer]) * (lastSignaledSemaphore[Queue::transfer] != idleSem));
                        lastSignaledSemaphore[Queue::compute] = reinterpret_cast<SemaphoreData*>(reinterpret_cast<std::size_t>(lastSignaledSemaphore[Queue::compute]) * (lastSignaledSemaphore[Queue::compute] != idleSem));

                        //do an assembly comparison when i have internet again
                    }
                    else if(ret != VK_TIMEOUT){
                        EWE_VK_ASSERT(ret);
                    }
                }
            }
        }

        SemaphoreData& GetSemaphore(){
            //to wait for one to be free, or consider none available to be an error?
            for(uint16_t i = 0; i < size * 2; i++){
                if(semaphores[i].Idle()){
                    return semaphores[i];
                }
            }
            assert(false && "no semaphore available");//if waiting for the semaphore instead of crashing is acceptable, do the following
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            return GetSemaphore();
        }
        FenceData& GetFence(Queue::Enum queue){
            //potential risk of stack overflow if recursive calls get too large
            for(uint8_t i = 0; i < size; i++){
                if(!fences[i].inUse){
                    if(lastSignaledSemaphore[queue] != nullptr){
                        fences[i].waitSemaphore = lastSignaledSemaphore[queue];
                        lastSignaledSemaphore[queue]->BeginWaiting();
                    }
                    return fences[i];
                }
            }
            assert(false && "no available fence when requested"); //if waiting for the fence instead of crashing is acceptable, do the following
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            //there is potential for a stack overflow if catastrophic errors occur, not sure if possible or how to detect before they occur
            return GetFence(queue);
        }
        FenceData& GetFenceSignal(Queue::Enum queue){
            //potential risk of stack overflow if recursive calls get too large
            for(uint8_t i = 0; i < size; i++){
                if(!fences[i].inUse){
                    fences[i].inUse = true;
                    fences[i].signalSemaphore = &GetSemaphore();
                    fences[i].signalSemaphore->BeginSignaling();
                    if(lastSignaledSemaphore[queue] != nullptr){
                        fences[i].waitSemaphore = lastSignaledSemaphore[queue];
                        lastSignaledSemaphore[queue]->waiting = true;
                    }
                    lastSignaledSemaphore[queue] = fences[i].signalSemaphore;
                    return fences[i];
                }
            }
            assert(false && "no available fence when requested"); //if waiting for the fence instead of crashing is acceptable, do the following
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            //there is potential for a stack overflow if catastrophic errors occur, not sure if possible or how to detect before they occur
            return GetFenceSignal(queue);
        }
        
    };
} //namespace EWE