#include "EWEngine/Graphics/SyncPool.h"

#include "EWEngine/Systems/ThreadPool.h"

namespace EWE {


    CommandCallbacks FenceData::WaitReturnCallbacks(VkDevice device, uint64_t time) {
        mut.lock();
        VkResult ret = vkWaitForFences(device, 1, &fence, true, time);
        if (ret == VK_SUCCESS) {
            EWE_VK(vkResetFences, device, 1, &fence);
            mut.unlock();
            for (auto& waitSem : waitSemaphores) {
                waitSem->FinishWaiting();
            }
            waitSemaphores.clear();
            for (uint8_t i = 0; i < Queue::_count; i++) {
                if (signalSemaphores[i] != nullptr) {
                    signalSemaphores[i]->FinishSignaling();
                    callbackData.semaphoreData = signalSemaphores[i];
                    signalSemaphores[i] = nullptr;
                }
            }
            inUse = false;
            return callbackData;
        }
        else if (ret == VK_TIMEOUT) {
            mut.unlock();
            return CommandCallbacks{};
        }
        else {
            mut.unlock();
            EWE_VK_RESULT(ret);
            return CommandCallbacks{}; //error silencing, the above line should throw an error
        }
    }


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
    


    void (*SyncPool::SubmitGraphicsAsync)(CommandBufferData&, std::vector<SemaphoreData*>) = nullptr;

    SyncPool::SyncPool(uint8_t size, VkDevice device, std::array<VkCommandPool, Queue::_count>& cmdPools) :
        size{ size },
        device{ device },
        fences{ new FenceData[size] },
        semaphores{ new SemaphoreData[size * 2] },
        cmdBufs{ new CommandBufferData[size], new CommandBufferData[size], new CommandBufferData[size], new CommandBufferData[size] },
        cmdPools{cmdPools}
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
        std::vector<VkCommandBuffer> cmdBufVector{};
        //im assuming the input cmdBuf doesn't matter, and it's overwritten without being read
        //if there's a bug, set the resize default to VK_NULL_HANDLE
        cmdBufVector.resize(size); 
        VkCommandBufferAllocateInfo cmdBufAllocInfo{};
        cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocInfo.pNext = nullptr;
        cmdBufAllocInfo.commandBufferCount = cmdBufVector.size();
        cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        for (int queue = 0; queue < Queue::_count; queue++) {
            if (cmdPools[queue] == VK_NULL_HANDLE) {

                continue;
            }
            cmdBufAllocInfo.commandPool = cmdPools[queue];
            EWE_VK(vkAllocateCommandBuffers, device, &cmdBufAllocInfo, cmdBufVector.data());

            std::sort(cmdBufVector.begin(), cmdBufVector.end());
            for (int i = 0; i < size; i++) {
                cmdBufs[queue][i].cmdBuf = cmdBufVector[i];
            }
        }

    }
    SyncPool::~SyncPool() {
        for (uint8_t i = 0; i < size; i++) {
            EWE_VK(vkDestroyFence, device, fences[i].fence, nullptr);
            EWE_VK(vkDestroySemaphore, device, semaphores[i].semaphore, nullptr);
        }

        delete[] fences;
        delete[] semaphores;

        std::vector<VkCommandBuffer> rawCmdBufs(size);
        for (uint8_t queue = 0; queue < Queue::_count; queue++) {
            for (uint8_t i = 0; i < size; i++) {
                rawCmdBufs[i] = cmdBufs[queue][i].cmdBuf;
            }
            EWE_VK(vkFreeCommandBuffers, device, cmdPools[queue], size, rawCmdBufs.data());
            delete[] cmdBufs[queue];
        }
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

    void SyncPool::HandleCallbacks(std::vector<CommandCallbacks> callbacks) {
        std::vector<SemaphoreData*> semaphoreData{};
        semaphoreData.push_back(callbacks[0].semaphoreData);
        for (int i = 1; i < callbacks.size(); i++) {
            callbacks[0].commands.insert(callbacks[0].commands.end(), callbacks[i].commands.begin(), callbacks[i].commands.end());
            callbacks[0].pipeBarriers.insert(callbacks[0].pipeBarriers.end(), callbacks[i].pipeBarriers.begin(), callbacks[i].pipeBarriers.end());
            callbacks[0].mipParamPacks.insert(callbacks[0].mipParamPacks.end(), callbacks[i].mipParamPacks.begin(), callbacks[i].mipParamPacks.end());
            semaphoreData.push_back(callbacks[i].semaphoreData);
        }
        std::sort(callbacks[0].commands.begin(), callbacks[0].commands.end());

        ResetCommandBuffers(callbacks[0].commands, Queue::transfer);


        PipelineBarrier::SimplifyVector(callbacks[0].pipeBarriers);

        for (auto& callback : callbacks) {
            for (auto& sb : callback.stagingBuffers) {
                sb->Free(device);
            }
        }
        if ((callbacks[0].mipParamPacks.size() > 0) || (callbacks[0].pipeBarriers.size() > 0)) {
            CommandBufferData& cmdBuf = GetCmdBufSingleTime(Queue::graphics);
            if (callbacks[0].mipParamPacks.size() > 1) {
                Image::GenerateMipMapsForMultipleImages(cmdBuf.cmdBuf, callbacks[0].mipParamPacks);
            }
            else {
                Image::GenerateMipmaps(cmdBuf.cmdBuf, callbacks[0].mipParamPacks[0]);
            }
        
            for (auto& barrier : callbacks[0].pipeBarriers) {
                barrier.Submit(cmdBuf.cmdBuf);
            }
            SubmitGraphicsAsync(cmdBuf, semaphoreData);
        }
        callbacks.clear();
    }

    void SyncPool::CheckFencesForCallbacks() {

        std::vector<CommandCallbacks> callbacks{};
        for (uint16_t i = 0; i < size; i++) {
            if (fences[i].inUse) {
                //the pipeline barriers should already be simplified, might be worth calling it again in case 2 transfers submitted before graphics got to it
                //probably worth profiling
                callbacks.push_back(fences[i].WaitReturnCallbacks(device, 0));
            }
        }
#if EWE_DEBUG
        assert(SubmitGraphicsAsync != nullptr);
#endif
        ThreadPool::EnqueueVoid(&SyncPool::HandleCallbacks, this, std::move(callbacks));
    }
#if SEMAPHORE_TRACKING
    SemaphoreData* SyncPool::GetSemaphoreForSignaling(const char* signalName) {
#else
    SemaphoreData* SyncPool::GetSemaphoreForSignaling() {
#endif
        while (true) {
            semAcqMut.lock();
            for (uint16_t i = 0; i < size * 2; i++) {
                if (semaphores[i].Idle()) {
#if SEMAPHORE_TRACKING
                    semaphores[i].BeginSignaling(signalName);
#else
                    semaphores[i].BeginSignaling();
#endif
                    semAcqMut.unlock();
                    return &semaphores[i];
                }
            }
            //potentially add a resizing function here
            assert(false && "no semaphore available, if waiting for a semaphore to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    FenceData& SyncPool::GetFence() {
        while (true) {
            fenceAcqMut.lock();
            for (uint8_t i = 0; i < size; i++) {
                if (!fences[i].inUse) {
                    fences[i].Lock();
                    fences[i].inUse = true;
                    fenceAcqMut.unlock();
                    return fences[i];
                }
            }
            //potentially add a resizing function here
            assert(false && "no available fence when requested, if waiting for a fence to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    CommandBufferData& SyncPool::GetCmdBufSingleTime(Queue::Enum queue) {
        while (true) {
            cmdBufAcqMut.lock();
            for(uint8_t i = 0; i < size; i++){
                if (!cmdBufs[queue][i].inUse) {
                    cmdBufs[queue][i].BeginSingleTime();
                    cmdBufAcqMut.unlock();
                    return cmdBufs[queue][i];
                }
            }
            //potentially add a resizing function here
            assert(false && "no available command buffer when requested, if waiting for a cmdbuf to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    void SyncPool::ResetCommandBuffer(CommandBufferData& cmdBuf, Queue::Enum queue) {
        for (uint8_t i = 0; i < size; i++) {
            if (&cmdBuf == &cmdBufs[queue][i]) {
                cmdBufs[queue][i].Reset();
                return;
            }
        }
        assert(false && "failed to find the commadn buffer to be reset, incorrect queue potentially");
    }
    void SyncPool::ResetCommandBuffer(CommandBufferData& cmdBuf) {
        for (uint8_t queue = 0; queue < Queue::_count; queue++) {
            for (uint8_t i = 0; i < size; i++) {
                if (&cmdBuf == &cmdBufs[queue][i]) {
                    cmdBufs[queue][i].Reset();
                    return;
                }
            }
        }
        assert(false && "failed to find the command buffer to be reset");
    }
    void SyncPool::ResetCommandBuffers(std::vector<CommandBufferData*>& cmdBufVec, Queue::Enum queue) {
        uint8_t i = 0; //this works because both containers are sorted
        for (int j = 0; j < cmdBufVec.size(); j++) {
            bool found = false;
            for (; i < size; i++) {
                if (cmdBufVec[j] == &cmdBufs[queue][i]) {
                    cmdBufs[queue][i].Reset();
                    found = true;
                    break;
                }
            }
            assert(found && "failed to find the command buffer to be reset");
        }
    }
    void SyncPool::ResetCommandBuffers(std::vector<CommandBufferData*>& cmdBufVec) {
        for (int j = 0; j < cmdBufVec.size(); j++) {
            bool found = false;
            for (uint8_t queue = 0; queue < Queue::_count; queue++) {
                if (found) {
                    break;
                }
                for (uint8_t i = 0; i < size; i++) {
                    if (cmdBufVec[j] == &cmdBufs[queue][i]) {
                        cmdBufs[queue][i].Reset();
                        found = true;
                        break;
                    }
                }
            }
            assert(found && "failed to find the command buffer to be reset");
        }
    }
}//namespace EWE

//brb