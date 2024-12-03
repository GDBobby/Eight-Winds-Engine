#include "EWEngine/Graphics/SyncPool.h"

#include "EWEngine/Systems/ThreadPool.h"

#include "EWEngine/Graphics/Texture/ImageFunctions.h"

namespace EWE {
#if !COMMAND_BUFFER_TRACING
    void SyncPool::CommandBufferTracker::Reset() {
        //VkCommandBufferResetFlags flags = VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;
        VkCommandBufferResetFlags flags = 0;
        EWE_VK(vkResetCommandBuffer, cmdBuf, flags);
        inUse = false;
    }
    void SyncPool::CommandBufferTracker::BeginSingleTime() {
        inUse = true;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        EWE_VK(vkBeginCommandBuffer, cmdBuf, &beginInfo);
    }
#endif

    bool FenceData::CheckReturn(uint64_t time) {
        mut.lock();
        VkResult ret = vkWaitForFences(VK::Object->vkDevice, 1, &fence, true, time);
        if (ret == VK_SUCCESS) {
            EWE_VK(vkResetFences, VK::Object->vkDevice, 1, &fence);
            //its up to the calling function to unlock the mutex
            for (auto& waitSem : waitSemaphores) {
                waitSem->FinishWaiting();
            }
            waitSemaphores.clear();
            return true;
        }
        else if (ret == VK_TIMEOUT) {
            //its up to the calling function to unlock the mutex
            return false;
        }
        else {
            //its up to the calling function to unlock the mutex
            EWE_VK_RESULT(ret);
            return false; //error silencing, this should not be reached
        }
    }


    void GraphicsFenceData::CheckReturn(std::vector<CommandBuffer*>& output, uint64_t time) {
        if (fenceData.CheckReturn(time)) {
            fenceData.mut.unlock();
#if EWE_DEBUG
            assert(commands.size() > 0);
#endif

            for (auto& layout : imageLayouts) {

                *layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            imageLayouts.clear();
            output.insert(output.end(), commands.begin(), commands.end());
            commands.clear();
            fenceData.inUse = false;
            return;
        }

        fenceData.mut.unlock();
    }

    void TransferFenceData::WaitReturnCallbacks(std::vector<TransferCommandCallbacks>& output, uint64_t time) {
        if (fenceData.CheckReturn(time)) {
            fenceData.mut.unlock();

            if (signalSemaphoreForGraphics != nullptr) {
                if (callbacks.images.size() > 0 || callbacks.pipeBarriers.size() > 0) {
                    signalSemaphoreForGraphics->BeginWaiting();
                }
                signalSemaphoreForGraphics->FinishSignaling();
                signalSemaphoreForGraphics = nullptr;
            }
            output.push_back(std::move(callbacks));
            fenceData.inUse = false;
            return;
        }
        fenceData.mut.unlock();
    }


    RenderSyncData::RenderSyncData() {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.pNext = nullptr;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semInfo.pNext = nullptr;
        for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            EWE_VK(vkCreateFence, VK::Object->vkDevice, &fenceInfo, nullptr, &inFlight[i]);
            EWE_VK(vkCreateSemaphore, VK::Object->vkDevice, &semInfo, nullptr, &imageAvailableSemaphore[i]);
            EWE_VK(vkCreateSemaphore, VK::Object->vkDevice, &semInfo, nullptr, &renderFinishedSemaphore[i]);
#if DEBUG_NAMING
            std::string objName{};
            objName = "in flight " + std::to_string(i);
            DebugNaming::SetObjectName(inFlight[i], VK_OBJECT_TYPE_FENCE, objName.c_str());
            objName = "image available " + std::to_string(i);
            DebugNaming::SetObjectName(imageAvailableSemaphore[i], VK_OBJECT_TYPE_SEMAPHORE, objName.c_str());
            objName = "render finished " + std::to_string(i);
            DebugNaming::SetObjectName(renderFinishedSemaphore[i], VK_OBJECT_TYPE_SEMAPHORE, objName.c_str());
#endif
        }
    }
    RenderSyncData::~RenderSyncData() {
        for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            EWE_VK(vkDestroyFence, VK::Object->vkDevice, inFlight[i], nullptr);
            EWE_VK(vkDestroySemaphore, VK::Object->vkDevice, imageAvailableSemaphore[i], nullptr);
            EWE_VK(vkDestroySemaphore, VK::Object->vkDevice, renderFinishedSemaphore[i], nullptr);
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
    void RenderSyncData::SetWaitData(VkSubmitInfo& submitInfo) {
        waitMutex.lock();
        for (auto& waitSem : previousWait[VK::Object->frameIndex].semaphores) {
            waitSem->FinishWaiting();
        }
        previousWait[VK::Object->frameIndex].semaphores.clear();
        previousWait[VK::Object->frameIndex].waitDstMask.clear();
        previousWait[VK::Object->frameIndex].semaphoreData.clear();
        previousWait[VK::Object->frameIndex] = waitData; //i think this makes the above clears repetitive, but im not sure
        waitData.semaphores.clear();
        waitData.waitDstMask.clear();
        waitData.semaphoreData.clear();
#if EWE_DEBUG
        assert(previousWait[VK::Object->frameIndex].semaphores.size() == previousWait[VK::Object->frameIndex].waitDstMask.size());
#endif

        for (auto& waitSem : previousWait[VK::Object->frameIndex].semaphores) {
            previousWait[VK::Object->frameIndex].semaphoreData.push_back(waitSem->semaphore);
        }
        previousWait[VK::Object->frameIndex].semaphoreData.push_back(imageAvailableSemaphore[VK::Object->frameIndex]);
        previousWait[VK::Object->frameIndex].waitDstMask.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        submitInfo.pWaitDstStageMask = previousWait[VK::Object->frameIndex].waitDstMask.data();
        submitInfo.waitSemaphoreCount = previousWait[VK::Object->frameIndex].semaphoreData.size();
        submitInfo.pWaitSemaphores = previousWait[VK::Object->frameIndex].semaphoreData.data();

        waitMutex.unlock();
    }
    std::vector<VkSemaphore> RenderSyncData::GetSignalData() {
        signalMutex.lock();
        for (auto& sigSem : previousSignals[VK::Object->frameIndex]) {
            sigSem->FinishSignaling();
        }
        previousSignals[VK::Object->frameIndex].clear();
        previousSignals[VK::Object->frameIndex] = signalSemaphores;
        signalSemaphores.clear();

        std::vector<VkSemaphore> ret{};
        ret.reserve(previousSignals[VK::Object->frameIndex].size() + 1);
        for (auto& sigSem : previousSignals[VK::Object->frameIndex]) {
            ret.push_back(sigSem->semaphore);
        }
        ret.push_back(renderFinishedSemaphore[VK::Object->frameIndex]);
        signalMutex.unlock();
        return ret;
    }
    


    void (*SyncPool::SubmitGraphicsAsync)(CommandBuffer&, std::vector<SemaphoreData*>, std::vector<VkImageLayout*>) = nullptr;

    SyncPool::SyncPool(uint8_t size) :
        size{ size },
        transferFences{ size },
        graphicsFences{ size },
        semaphores{ static_cast<std::size_t>(size * 2)},
        cmdBufs{}
    {
        assert(size <= 64 && "this isn't optimized very well, don't use big size"); //big size probably also isn't necessary

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.flags = 0;
        fenceInfo.pNext = nullptr;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        for (uint8_t i = 0; i < size; i++) {
            EWE_VK(vkCreateFence, VK::Object->vkDevice, &fenceInfo, nullptr, &transferFences[i].fenceData.fence);
            EWE_VK(vkCreateFence, VK::Object->vkDevice, &fenceInfo, nullptr, &graphicsFences[i].fenceData.fence);
        }

        VkSemaphoreCreateInfo semInfo{};
        semInfo.flags = 0;
        semInfo.pNext = nullptr;
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for (uint8_t i = 0; i < size * 2; i++) {
            EWE_VK(vkCreateSemaphore, VK::Object->vkDevice, &semInfo, nullptr, &semaphores[i].semaphore);
#if DEBUG_NAMING
            std::string name = "syncpool semaphore[" + std::to_string(i) + ']';
            DebugNaming::SetObjectName(semaphores[i].semaphore, VK_OBJECT_TYPE_SEMAPHORE, name.c_str());
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
            if (queue == Queue::graphics) {
                cmdBufs[queue].resize(size);
                cmdBufAllocInfo.commandPool = VK::Object->STGCmdPool;
                EWE_VK(vkAllocateCommandBuffers, VK::Object->vkDevice, &cmdBufAllocInfo, cmdBufVector.data());

                std::sort(cmdBufVector.begin(), cmdBufVector.end());
                for (int i = 0; i < size; i++) {
                    cmdBufs[queue][i].cmdBuf = cmdBufVector[i];
                }
            }
            else if (VK::Object->commandPools[queue] != VK_NULL_HANDLE) {
                cmdBufs[queue].resize(size);
                cmdBufAllocInfo.commandPool = VK::Object->commandPools[queue];
                EWE_VK(vkAllocateCommandBuffers, VK::Object->vkDevice, &cmdBufAllocInfo, cmdBufVector.data());

                std::sort(cmdBufVector.begin(), cmdBufVector.end());
                for (int i = 0; i < size; i++) {
                    cmdBufs[queue][i].cmdBuf = cmdBufVector[i];
                }
            }
        }

    }
    SyncPool::~SyncPool() {
        for (uint8_t i = 0; i < size; i++) {
            EWE_VK(vkDestroyFence, VK::Object->vkDevice, transferFences[i].fenceData.fence, nullptr);
            EWE_VK(vkDestroyFence, VK::Object->vkDevice, graphicsFences[i].fenceData.fence, nullptr);
            EWE_VK(vkDestroySemaphore, VK::Object->vkDevice, semaphores[i].semaphore, nullptr);
            EWE_VK(vkDestroySemaphore, VK::Object->vkDevice, semaphores[i + size].semaphore, nullptr);
        }

        graphicsFences.clear();
        transferFences.clear();
        semaphores.clear();

        std::vector<VkCommandBuffer> rawCmdBufs(size);
        for (uint8_t queue = 0; queue < Queue::_count; queue++) {
            if (queue == Queue::graphics) {
                for (uint8_t i = 0; i < size; i++) {
                    rawCmdBufs[i] = cmdBufs[queue][i].cmdBuf;
                }
                EWE_VK(vkFreeCommandBuffers, VK::Object->vkDevice, VK::Object->STGCmdPool, size, rawCmdBufs.data());
                cmdBufs[queue].clear();
            }
            else if (VK::Object->commandPools[queue] != VK_NULL_HANDLE) {
                for (uint8_t i = 0; i < size; i++) {
                    rawCmdBufs[i] = cmdBufs[queue][i].cmdBuf;
                }
                EWE_VK(vkFreeCommandBuffers, VK::Object->vkDevice, VK::Object->commandPools[queue], size, rawCmdBufs.data());
                cmdBufs[queue].clear();
            }
        }
    }
#if 0//SEMAPHORE_TRACKING

#else
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
#endif
    bool SyncPool::CheckFencesForUsage() {
        for (uint16_t i = 0; i < size; i++) {
            if (transferFences[i].fenceData.inUse) {
                return true;
            }
        }
        for (uint16_t i = 0; i < size; i++) {
            if (graphicsFences[i].fenceData.inUse) {
                return true;
            }
        }
        return false;
    }

    template<typename T>
    void InsertSecondIntoFirst(std::vector<T>& first, std::vector<T>& second) {
        if (second.size() > 0) {
            first.insert(first.begin(), std::make_move_iterator(second.begin()), std::make_move_iterator(second.end()));
        }
    }

    void SyncPool::HandleTransferCallbacks(std::vector<TransferCommandCallbacks> callbacks) {
        std::vector<SemaphoreData*> semaphoreData{};
        if ((callbacks[0].images.size() > 0) || (callbacks[0].pipeBarriers.size() > 0)) {
            assert(callbacks[0].semaphoreData != nullptr);
            semaphoreData.push_back(callbacks[0].semaphoreData);
        }

        for (int i = 1; i < callbacks.size(); i++) {
            InsertSecondIntoFirst(callbacks[0].commands, callbacks[i].commands);
            InsertSecondIntoFirst(callbacks[0].pipeBarriers, callbacks[i].pipeBarriers);
            InsertSecondIntoFirst(callbacks[0].images, callbacks[i].images);
            InsertSecondIntoFirst(callbacks[0].imageLayouts, callbacks[i].imageLayouts);
            if (callbacks[i].semaphoreData != nullptr) {
                semaphoreData.push_back(callbacks[i].semaphoreData);
            }
#if EWE_DEBUG
            if ((callbacks[i].images.size() > 0) || (callbacks[i].pipeBarriers.size() > 0)) {

                assert(callbacks[i].semaphoreData != nullptr);
            }
#endif
        }
        if (callbacks[0].commands.size() > 0) {
            std::sort(callbacks[0].commands.begin(), callbacks[0].commands.end());
        }

        ResetCommandBuffers(callbacks[0].commands, Queue::transfer);

        PipelineBarrier::SimplifyVector(callbacks[0].pipeBarriers);

        for (auto& callback : callbacks) {
            for (auto& sb : callback.stagingBuffers) {
                sb->Free();
            }
        }
        if ((callbacks[0].images.size() > 0) || (callbacks[0].pipeBarriers.size() > 0)) {
            CommandBuffer& cmdBuf = GetCmdBufSingleTime(Queue::graphics);
            if (callbacks[0].images.size() > 1) {

                Image::GenerateMipMapsForMultipleImagesTransferQueue(cmdBuf, callbacks[0].images);
            }
            else if(callbacks[0].images.size() == 1) {

                Image::GenerateMipmaps(cmdBuf, callbacks[0].images[0], Queue::transfer);
            }
        
            for (auto& barrier : callbacks[0].pipeBarriers) {
                barrier.Submit(cmdBuf);
            }
            SubmitGraphicsAsync(cmdBuf, semaphoreData, callbacks[0].imageLayouts);
        }
        callbacks.clear();
    }

    void SyncPool::CheckFencesForCallbacks() {

        std::vector<TransferCommandCallbacks> callbacks{};
        for (uint16_t i = 0; i < size; i++) {
            if (transferFences[i].fenceData.inUse) {
                //the pipeline barriers should already be simplified, might be worth calling it again in case 2 transfers submitted before graphics got to it
                //probably worth profiling
                transferFences[i].WaitReturnCallbacks(callbacks, 0);
            }
        }
#if EWE_DEBUG
        assert(SubmitGraphicsAsync != nullptr);
#endif
        if (callbacks.size() > 0) {
            ThreadPool::EnqueueVoid(&SyncPool::HandleTransferCallbacks, this, std::move(callbacks));
        }

        std::vector<CommandBuffer*> graphicsCmdBuf{};
        for (uint16_t i = 0; i < size; i++) {
            if (graphicsFences[i].fenceData.inUse) {
                graphicsFences[i].CheckReturn(graphicsCmdBuf, 0);
            }
        }
        if (graphicsCmdBuf.size() > 0) {
            ResetCommandBuffers(graphicsCmdBuf, Queue::graphics);
        }
    }
#if SEMAPHORE_TRACKING
    SemaphoreData* SyncPool::GetSemaphoreForSignaling(std::source_location srcLoc) {
#else
    SemaphoreData* SyncPool::GetSemaphoreForSignaling() {
#endif
        while (true) {
            semAcqMut.lock();
            for (uint16_t i = 0; i < size * 2; i++) {
                if (semaphores[i].Idle()) {
#if SEMAPHORE_TRACKING
                    semaphores[i].BeginSignaling(srcLoc);
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
    GraphicsFenceData& SyncPool::GetGraphicsFence() {
        while (true) {
            graphicsFenceAcqMut.lock();
            for (uint8_t i = 0; i < size; i++) {
                if (!graphicsFences[i].fenceData.inUse) {
                    graphicsFences[i].fenceData.mut.lock();
                    graphicsFences[i].fenceData.inUse = true;
                    graphicsFenceAcqMut.unlock();
                    return graphicsFences[i];
                }
            }
            //potentially add a resizing function here
            assert(false && "no available fence when requested, if waiting for a fence to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    TransferFenceData& SyncPool::GetTransferFence() {
        while (true) {
            transferFenceAcqMut.lock();
            for (uint8_t i = 0; i < size; i++) {
                if (!transferFences[i].fenceData.inUse) {
                    transferFences[i].fenceData.mut.lock();
                    transferFences[i].fenceData.inUse = true;
                    transferFenceAcqMut.unlock();
                    return transferFences[i];
                }
            }
            //potentially add a resizing function here
            assert(false && "no available fence when requested, if waiting for a fence to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    CommandBuffer& SyncPool::GetCmdBufSingleTime(Queue::Enum queue) {
        while (true) {
            cmdBufAcqMut.lock();
            for(uint8_t i = 0; i < size; i++){
                if (!cmdBufs[queue][i].inUse) {
                    if (queue == Queue::graphics) {
                        VK::Object->STGMutex.lock();
                        cmdBufs[queue][i].BeginSingleTime();
                        VK::Object->STGMutex.unlock();
                    }
                    else {
                        //i dont know if transfer begin needs a mutex? but i guess so
                        VK::Object->poolMutex[queue].lock();
                        cmdBufs[queue][i].BeginSingleTime();
                        VK::Object->poolMutex[queue].unlock();
                    }
                    cmdBufAcqMut.unlock();
#if COMMAND_BUFFER_TRACING
                    return cmdBufs[queue][i];
#else
                    return cmdBufs[queue][i].cmdBuf;
#endif
                }
            }
            //potentially add a resizing function here
            assert(false && "no available command buffer when requested, if waiting for a cmdbuf to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    void SyncPool::ResetCommandBuffer(CommandBuffer& cmdBuf, Queue::Enum queue) {
        for (uint8_t i = 0; i < size; i++) {
#if COMMAND_BUFFER_TRACING
            if (&cmdBuf == &cmdBufs[queue][i]) {
#else
            if (&cmdBuf == &cmdBufs[queue][i].cmdBuf) {
#endif
                if (queue == Queue::graphics) {
                    VK::Object->STGMutex.lock();
                    cmdBufs[queue][i].Reset();
                    VK::Object->STGMutex.unlock();
                }
                else {
                    VK::Object->poolMutex[queue].lock();
                    cmdBufs[queue][i].Reset();
                    VK::Object->poolMutex[queue].unlock();
                }
                return;
            }
        }
        assert(false && "failed to find the commadn buffer to be reset, incorrect queue potentially");
    }
    void SyncPool::ResetCommandBuffer(CommandBuffer& cmdBuf) {
        for (uint8_t queue = 0; queue < Queue::_count; queue++) {
            for (uint8_t i = 0; i < size; i++) {
#if COMMAND_BUFFER_TRACING
                if (&cmdBuf == &cmdBufs[queue][i]) {
#else
                if (&cmdBuf == &cmdBufs[queue][i].cmdBuf) {
#endif
                    if (queue == Queue::graphics) {
                        VK::Object->STGMutex.lock();
                        cmdBufs[queue][i].Reset();
                        VK::Object->STGMutex.unlock();
                    }
                    else{
                        VK::Object->poolMutex[queue].lock();
                        cmdBufs[queue][i].Reset();
                        VK::Object->poolMutex[queue].unlock();
                    }
                    return;
                }
            }
        }
        assert(false && "failed to find the command buffer to be reset");
    }
    void SyncPool::ResetCommandBuffers(std::vector<CommandBuffer*>& cmdBufVec, Queue::Enum queue) {
        uint8_t i = 0;
         //this works because both containers are sorted
        for (int j = 0; j < cmdBufVec.size(); j++) {
            bool found = false;
            for (; i < size; i++) {
#if COMMAND_BUFFER_TRACING
                if(cmdBufVec[j] == &cmdBufs[queue][i]) {
#else
                if (cmdBufVec[j] == &cmdBufs[queue][i].cmdBuf) {
#endif
                    if (queue == Queue::graphics) {
                        VK::Object->STGMutex.lock();
                        cmdBufs[queue][i].Reset();
                        VK::Object->STGMutex.unlock();
                    }
                    else{
                        VK::Object->poolMutex[queue].lock();
                        cmdBufs[queue][i].Reset();
                        VK::Object->poolMutex[queue].unlock();
                    }
                    found = true;
                    break;
                }
            }
            assert(found && "failed to find the command buffer to be reset");
        }
    }
    void SyncPool::ResetCommandBuffers(std::vector<CommandBuffer*>& cmdBufVec) {

        for (int j = 0; j < cmdBufVec.size(); j++) {
            bool found = false;
            for (uint8_t queue = 0; queue < Queue::_count; queue++) {
                if (found) {
                    break;
                }
                for (uint8_t i = 0; i < size; i++) {
#if COMMAND_BUFFER_TRACING
                    if (cmdBufVec[j] == &cmdBufs[queue][i]) {
#else
                    if (cmdBufVec[j] == &cmdBufs[queue][i].cmdBuf) {
#endif
                        if (queue == Queue::graphics) {
                            VK::Object->STGMutex.lock();
                            cmdBufs[queue][i].Reset();
                            VK::Object->STGMutex.unlock();
                        }
                        else{
                            VK::Object->poolMutex[queue].lock();
                            cmdBufs[queue][i].Reset();
                            VK::Object->poolMutex[queue].unlock();
                        }
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