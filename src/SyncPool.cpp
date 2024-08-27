#include "EWEngine/Graphics/SyncPool.h"

#include "EWEngine/Systems/ThreadPool.h"

namespace EWE {
    RenderSyncData::RenderSyncData(VkDevice device) : device{ device } {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.pNext = nullptr;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semInfo.pNext = nullptr;
        for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            EWE_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &inFlight[i]));
            EWE_VK_ASSERT(vkCreateSemaphore(device, &semInfo, nullptr, &imageAvailable[i]));
            EWE_VK_ASSERT(vkCreateSemaphore(device, &semInfo, nullptr, &renderFinished[i]));
        }
    }
    RenderSyncData::~RenderSyncData() {
        for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyFence(device, inFlight[i], nullptr);
            vkDestroySemaphore(device, imageAvailable[i], nullptr);
            vkDestroySemaphore(device, renderFinished[i], nullptr);
        }
    }
    void RenderSyncData::AddWaitSemaphore(SemaphoreData* semaphore) {
        waitMutex.lock();
        semaphore->BeginWaiting();
        waitSemaphores.push_back(semaphore);
        waitMutex.unlock();
    }
    void RenderSyncData::AddSignalSemaphore(SemaphoreData* semaphore) {
        signalMutex.lock();
        signalSemaphores.push_back(semaphore);
        signalMutex.unlock();
    }
    std::vector<VkSemaphore> RenderSyncData::GetWaitData(uint8_t frameIndex) {
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
    std::vector<VkSemaphore> RenderSyncData::GetSignalData(uint8_t frameIndex) {
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

    bool SyncPool::CheckFencesForUsage() {
        for (uint16_t i = 0; i < size; i++) {
            if (fences[i].inUse) {
                return true;
            }
        }
        return false;
    }

    void SyncPool::CheckFencesForCallbacks() {

        std::vector<std::function<void()>> callbacks{};
        for (uint16_t i = 0; i < size; i++) {
            if (fences[i].inUse) {
                std::function<void()> cb = fences[i].WaitReturnCallbacks(device, 0);
                if (cb != nullptr) {
                    callbacks.push_back(cb);
                }
            }
        }
        if (callbacks.size() == 1) {
            ThreadPool::EnqueueVoidFunction(callbacks[0]);
        }
        else if (callbacks.size() > 0) {
            ThreadPool::EnqueueVoid([callbacks] {
                    for (auto const& cb : callbacks) {
                        cb();
                    }
                }
            );
        }
    }

    SemaphoreData* SyncPool::GetSemaphore() {
        while (true) {
            for (uint16_t i = 0; i < size * 2; i++) {
                if (semaphores[i].Idle()) {
                    return &semaphores[i];
                }
            }
            assert(false && "no semaphore available, if waiting for a semaphore to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    FenceData& SyncPool::GetFence() {
        while (true) {
            for (uint8_t i = 0; i < size; i++) {
                if (!fences[i].inUse) {
                    fences[i].inUse = true;
                    return fences[i];
                }
            }
            assert(false && "no available fence when requested, if waiting for a fence to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    //FenceData& SyncPool::GetFenceSignal(Queue::Enum queue) {
    //    FenceData& ret = GetFence();
    //    ret.signalSemaphores[queue] = GetSemaphore();
    //    ret.signalSemaphores[queue]->BeginSignaling();
    //    return ret;
    //}
}//namespace EWE

//brb