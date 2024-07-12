#include "EWEngine/Graphics/SyncPool.h"

namespace EWE {
    SyncPool::SyncPool(uint8_t size, VkDevice device) :
        size{ size },
        device{ device },
        fences{ new FenceData[size] },
        semaphores{ new SemaphoreData[size * 2] }
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
        for (uint8_t i = 0; i < size * 2; i++) {
            EWE_VK_ASSERT(vkCreateSemaphore(device, &semInfo, nullptr, &semaphores[i].semaphore));
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
                    fences[i].Reset(device);
                }
                else if (ret != VK_TIMEOUT) {
                    EWE_VK_ASSERT(ret);
                }
            }
        }
    }

    SemaphoreData* SyncPool::GetSemaphore() {
        while (true) {
            for (uint16_t i = 0; i < size * 2; i++) {
                if (semaphores[i].Idle()) {
                    return &semaphores[i];
                }
            }
            assert(false && "no semaphore available");//if waiting for the semaphore instead of crashing is acceptable, do the following
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
        return GetSemaphore();
    }
    FenceData& SyncPool::GetFence() {
        //potential risk of stack overflow if recursive calls get too large
        while (true) {
            for (uint8_t i = 0; i < size; i++) {
                if (!fences[i].inUse) {
                    fences[i].inUse = true;
                    return fences[i];
                }
            }
            assert(false && "no available fence when requested"); //if waiting for the fence instead of crashing is acceptable, do the following
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    FenceData& SyncPool::GetFenceSignal(Queue::Enum queue) {
        FenceData& ret = GetFence();
        ret.signalSemaphores[queue] = GetSemaphore();
        ret.signalSemaphores[queue]->BeginSignaling();
        return ret;
    }
}//namespace EWE

//brb