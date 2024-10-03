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
            EWE_VK(vkCreateFence, device, &fenceInfo, nullptr, &inFlight[i]);
            EWE_VK(vkCreateSemaphore, device, &semInfo, nullptr, &imageAvailableSemaphore[i]);
            EWE_VK(vkCreateSemaphore, device, &semInfo, nullptr, &renderFinishedSemaphore[i]);
#if DEBUG_NAMING
            std::string objName{};
            objName = "in flight " + std::to_string(i);
            DebugNaming::SetObjectName(device, inFlight[i], VK_OBJECT_TYPE_FENCE, objName.c_str());
            objName = "image available " + std::to_string(i);
            DebugNaming::SetObjectName(device, imageAvailableSemaphore[i], VK_OBJECT_TYPE_SEMAPHORE, objName.c_str());
            objName = "render finished " + std::to_string(i);
            DebugNaming::SetObjectName(device, renderFinishedSemaphore[i], VK_OBJECT_TYPE_SEMAPHORE, objName.c_str());
#endif
        }
    }
    RenderSyncData::~RenderSyncData() {
        for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            EWE_VK(vkDestroyFence, device, inFlight[i], nullptr);
            EWE_VK(vkDestroySemaphore, device, imageAvailableSemaphore[i], nullptr);
            EWE_VK(vkDestroySemaphore, device, renderFinishedSemaphore[i], nullptr);
        }
    }
    void RenderSyncData::AddWaitSemaphore(SemaphoreData* semaphore, VkPipelineStageFlags waitDstStageMask) {
        waitMutex.lock();
        semaphore->BeginWaiting();
        waitData.semaphores.push_back(semaphore);
        waitData.waitDstMask.push_back(waitDstStageMask);
        waitMutex.unlock();
    }
    void RenderSyncData::AddSignalSemaphore(SemaphoreData* semaphore) {
        signalMutex.lock();
        signalSemaphores.push_back(semaphore);
        signalMutex.unlock();
    }
    void RenderSyncData::SetWaitData(uint8_t frameIndex, VkSubmitInfo& submitInfo) {
        waitMutex.lock();
        for (auto& waitSem : previousWait[frameIndex].semaphores) {
            waitSem->FinishWaiting();
        }
        previousWait[frameIndex].semaphores.clear();
        previousWait[frameIndex].waitDstMask.clear();
        previousWait[frameIndex].semaphoreData.clear();
        previousWait[frameIndex] = waitData; //i think this makes the above clears repetitive, but im not sure
        waitData.semaphores.clear();
        waitData.waitDstMask.clear();
        waitData.semaphoreData.clear();

        assert(previousWait[frameIndex].semaphores.size() == previousWait[frameIndex].waitDstMask.size());

        for (auto& waitSem : previousWait[frameIndex].semaphores) {
            previousWait[frameIndex].semaphoreData.push_back(waitSem->semaphore);
        }
        previousWait[frameIndex].semaphoreData.push_back(imageAvailableSemaphore[frameIndex]);
        previousWait[frameIndex].waitDstMask.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        submitInfo.pWaitDstStageMask = previousWait[frameIndex].waitDstMask.data();
        submitInfo.waitSemaphoreCount = previousWait[frameIndex].semaphoreData.size();
        submitInfo.pWaitSemaphores = previousWait[frameIndex].semaphoreData.data();

        waitMutex.unlock();
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
        ret.push_back(renderFinishedSemaphore[frameIndex]);
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
            EWE_VK(vkCreateFence, device, &fenceInfo, nullptr, &fences[i].fence);
        }

        VkSemaphoreCreateInfo semInfo{};
        semInfo.flags = 0;
        semInfo.pNext = nullptr;
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for (uint8_t i = 0; i < size * 2; i++) {
            EWE_VK(vkCreateSemaphore, device, &semInfo, nullptr, &semaphores[i].semaphore);
#if SEMAPHORE_TRACKING
            semaphores[i].device = device;
#endif
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

    void SyncPool::CheckFencesForCallbacks(std::mutex& transferPoolMutex) {

        std::vector<std::function<void()>> cmdFreeCallbacks{};
        std::vector<std::function<void()>> otherCallbacks{};
        for (uint16_t i = 0; i < size; i++) {
            if (fences[i].inUse) {
                TransferCallbackReturn transferCBReturns = fences[i].WaitReturnCallbacks(device, 0);
                if (transferCBReturns.otherCallbacks != nullptr) {
                    otherCallbacks.push_back(transferCBReturns.otherCallbacks);
                }
                if (transferCBReturns.freeCommandBufferCallback != nullptr) {
                    cmdFreeCallbacks.push_back(transferCBReturns.freeCommandBufferCallback);
                }
            }
        }
        if (otherCallbacks.size() == 1) {
            ThreadPool::EnqueueVoidFunction(otherCallbacks[0]);
        }
        else if (otherCallbacks.size() > 1) {
            auto cbFunc = [otherCallbacks] {
                for (auto const& cb : otherCallbacks) {
                    cb();
                }
            };
            ThreadPool::EnqueueVoid(cbFunc);
        }
        if (cmdFreeCallbacks.size() == 1) {
            auto cmdFreeWrapper = [&transferPoolMutex, cmdFreeCallbacks] {
                transferPoolMutex.lock();
                cmdFreeCallbacks[0]();
                transferPoolMutex.unlock();
            };
            ThreadPool::EnqueueVoidFunction(cmdFreeWrapper);
        }
        else if (cmdFreeCallbacks.size() > 1) {
            auto cmdFreeWrapper = [&transferPoolMutex, cmdFreeCallbacks] {
                transferPoolMutex.lock();
                for (auto& cb : cmdFreeCallbacks) {
                    cb();
                }
                transferPoolMutex.unlock();
            };
            ThreadPool::EnqueueVoidFunction(cmdFreeWrapper);
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