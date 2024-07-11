#include "EWEngine/Graphics/SyncPool.h"

namespace EWE {
    SemaphoreData* FenceData::Reset(VkDevice device) {
        EWE_VK_ASSERT(vkResetFences(device, 1, &fence));
        if (waitSemaphore != nullptr) {
            waitSemaphore->FinishWaiting();
            waitSemaphore = nullptr;
        }
        if (signalSemaphore != nullptr) {
            signalSemaphore->FinishSignaling();
            if (!signalSemaphore->waiting) {
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
    void FenceData::PrepareSubmitInfo(VkSubmitInfo& submitInfo) {
        submitInfo.pSignalSemaphores = &signalSemaphore->semaphore;
        submitInfo.signalSemaphoreCount = signalSemaphore != nullptr;

        submitInfo.pWaitSemaphores = &waitSemaphore->semaphore;
        submitInfo.waitSemaphoreCount = waitSemaphore != nullptr;
    }



    SyncPool::SyncPool(uint8_t size, VkDevice device) :
        size{ size },
        device{ device },
        fences{ new FenceData[size] },
        semaphores{ new SemaphoreData[size] },
        transitionSemaphores{ new SemaphoreData[size] }
    {
        assert(size <= 64 && "this isn't optimized very well, don't use big size"); //big size probably also isn't necessary

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.flags = 0;
        fenceInfo.pNext = nullptr;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        for (uint8_t i = 0; i < size; i++) {
            EWE_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &fences[i].fence));
        }

        VkSemaphoreCreateInfo semInfo{};
        semInfo.flags = 0;
        semInfo.pNext = nullptr;
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for (uint8_t i = 0; i < size; i++) {
            EWE_VK_ASSERT(vkCreateSemaphore(device, &semInfo, nullptr, &semaphores[i].semaphore));
            EWE_VK_ASSERT(vkCreateSemaphore(device, &semInfo, nullptr, &transitionSemaphores[i].semaphore));
        }
    }
    SyncPool::~SyncPool() {
        delete[] fences;
        delete[] semaphores;
    }
    SemaphoreData& SyncPool::GetSemaphoreData(VkSemaphore semaphore) {
        for (uint8_t i = 0; i < size * 2; i++) {
            if (semaphores[i].semaphore == semaphore) {
                return semaphores[i];
            }
        }
        assert(false && "failed to find SemaphoreData");
        return semaphores[0]; //DO NOT return this, error silencing
        //only way for this to not return an error is if the return type is changed to pointer and nullptr is returned if not found, or std::conditional which im not a fan of
    }

    void SyncPool::CheckFences() {

        std::vector<std::function<void()>> callbacks{};
        for (uint16_t i = 0; i < size; i++) {
            if (fences[i].inUse) {
                VkResult ret = vkWaitForFences(device, 1, &fences[i].fence, true, 0);
                if (ret == VK_SUCCESS) {
                    SemaphoreData* idleSem = fences[i].Reset(device);
                    //idleSem is nullptr if the owned signaler semaphore is also being waited on
                    //i could store the queue but it seems like a bit of a waste, in the following code im checking if it's equal, then setting to nullptr

                    //do an assembly comparison when i have internet again
                }
                else if (ret != VK_TIMEOUT) {
                    EWE_VK_ASSERT(ret);
                }
            }
        }
    }

    SemaphoreData& SyncPool::GetSemaphore() {
        //to wait for one to be free, or consider none available to be an error?
        for (uint16_t i = 0; i < size * 2; i++) {
            if (semaphores[i].Idle()) {
                return semaphores[i];
            }
        }
        assert(false && "no semaphore available");//if waiting for the semaphore instead of crashing is acceptable, do the following
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        return GetSemaphore();
    }
    FenceData& SyncPool::GetFence(Queue::Enum queue) {
        //potential risk of stack overflow if recursive calls get too large
        for (uint8_t i = 0; i < size; i++) {
            if (!fences[i].inUse) {
                return fences[i];
            }
        }
        assert(false && "no available fence when requested"); //if waiting for the fence instead of crashing is acceptable, do the following
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        //there is potential for a stack overflow if catastrophic errors occur, not sure if possible or how to detect before they occur
        return GetFence(queue);
    }
    FenceData& SyncPool::GetFenceSignal(Queue::Enum queue) {
        //potential risk of stack overflow if recursive calls get too large
        for (uint8_t i = 0; i < size; i++) {
            if (!fences[i].inUse) {
                fences[i].inUse = true;
                fences[i].signalSemaphore = &GetSemaphore();
                fences[i].signalSemaphore->BeginSignaling();

                return fences[i];
            }
        }
        assert(false && "no available fence when requested"); //if waiting for the fence instead of crashing is acceptable, do the following
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        //there is potential for a stack overflow if catastrophic errors occur, not sure if possible or how to detect before they occur
        return GetFenceSignal(queue);
    }
}//namespace EWE