#include "EWEngine/Graphics/QueueSyncPool.h"

#include "EWEngine/Systems/ThreadPool.h"

#include "EWEngine/Graphics/Texture/ImageFunctions.h"

namespace EWE {
#if !COMMAND_BUFFER_TRACING
    void QueueSyncPool::CommandBufferWrapper::Reset() {
        //VkCommandBufferResetFlags flags = VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;
        VkCommandBufferResetFlags flags = 0;
        EWE_VK(vkResetCommandBuffer, cmdBuf, flags);
        inUse = false;
    }
    void QueueSyncPool::CommandBufferWrapper::BeginSingleTime() {
        inUse = true;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        EWE_VK(vkBeginCommandBuffer, cmdBuf, &beginInfo);
    }
#endif

    bool Fence::CheckReturn(uint64_t time) {
        if (!submitted) {
#if DEBUGGING_FENCES
            log.push_back("checked return, not submitted");
#endif
            return false;
        }
#if DEBUGGING_FENCES
        log.push_back("checked return, submitted");
#endif

        
        VkResult ret = vkWaitForFences(VK::Object->vkDevice, 1, &vkFence, true, time);
        if (ret == VK_SUCCESS) {
            EWE_VK(vkResetFences, VK::Object->vkDevice, 1, &vkFence);
            //its up to the calling function to unlock the mutex
            for (auto& waitSem : waitSemaphores) {
                waitSem->FinishWaiting();
            }
            waitSemaphores.clear();
            //makes more sense to clear the submitted flag here, rather than on acquire
            submitted = false;
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


    void GraphicsFence::CheckReturn(std::vector<CommandBuffer*>& output, uint64_t time) {
        if (fence.CheckReturn(time)) {
            assert(commands.size() > 0);

#if DEBUGGING_FENCES
            fence.log.push_back("checked return, continuing in graphics fence");
#endif
            for (auto& imgInfo : imageInfos) {

                imgInfo->descriptorImageInfo.imageLayout = imgInfo->destinationImageLayout;
            }
            imageInfos.clear();
            output.insert(output.end(), commands.begin(), commands.end());
            commands.clear();
#if DEBUGGING_FENCES
            fence.log.push_back("allowing graphics fence to be reobtained");
#endif
            fence.inUse = false;
        }
    }

    void TransferFence::WaitReturnCallbacks(std::vector<TransferCommandCallbacks>& output, uint64_t time) {

        if (fence.CheckReturn(time)) {

#if DEBUGGING_FENCES
            fence.log.push_back("checked return, continuing in transfer");
#endif
            if (signalSemaphoreForGraphics != nullptr) {
                if (callbacks.images.size() > 0 || callbacks.pipeBarriers.size() > 0) {
                    signalSemaphoreForGraphics->BeginWaiting();
                }
                signalSemaphoreForGraphics->FinishSignaling();
                signalSemaphoreForGraphics = nullptr;
            }
            output.push_back(std::move(callbacks));
#if DEBUGGING_FENCES
            fence.log.push_back("allowing transfer fence to be reobtained");
#endif
            fence.inUse = false;
            return;
        }
        //fence.mut.unlock();
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
    void RenderSyncData::AddWaitSemaphore(Semaphore* semaphore, VkPipelineStageFlags waitDstStageMask) {
        waitMutex.lock();
        semaphore->BeginWaiting();
        waitData.semaphores.push_back(semaphore);
        waitData.waitDstMask.push_back(waitDstStageMask);
        waitMutex.unlock();
    }
    void RenderSyncData::AddSignalSemaphore(Semaphore* semaphore) {
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

        assert(previousWait[VK::Object->frameIndex].semaphores.size() == previousWait[VK::Object->frameIndex].waitDstMask.size());

        for (auto& waitSem : previousWait[VK::Object->frameIndex].semaphores) {
            previousWait[VK::Object->frameIndex].semaphoreData.push_back(waitSem->vkSemaphore);
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
            ret.push_back(sigSem->vkSemaphore);
        }
        ret.push_back(renderFinishedSemaphore[VK::Object->frameIndex]);
        signalMutex.unlock();
        return ret;
    }

    QueueSyncPool::QueueSyncPool(uint8_t size) :
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
            EWE_VK(vkCreateFence, VK::Object->vkDevice, &fenceInfo, nullptr, &transferFences[i].fence.vkFence);
            EWE_VK(vkCreateFence, VK::Object->vkDevice, &fenceInfo, nullptr, &graphicsFences[i].fence.vkFence);
        }

        VkSemaphoreCreateInfo semInfo{};
        semInfo.flags = 0;
        semInfo.pNext = nullptr;
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for (uint8_t i = 0; i < size * 2; i++) {
            EWE_VK(vkCreateSemaphore, VK::Object->vkDevice, &semInfo, nullptr, &semaphores[i].vkSemaphore);
#if DEBUG_NAMING
            std::string name = "QueueSyncPool semaphore[" + std::to_string(i) + ']';
            DebugNaming::SetObjectName(semaphores[i].vkSemaphore, VK_OBJECT_TYPE_SEMAPHORE, name.c_str());
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
            if (VK::Object->commandPools[queue] != VK_NULL_HANDLE) {
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
    QueueSyncPool::~QueueSyncPool() {
        for (uint8_t i = 0; i < size; i++) {
            EWE_VK(vkDestroyFence, VK::Object->vkDevice, transferFences[i].fence.vkFence, nullptr);
            EWE_VK(vkDestroyFence, VK::Object->vkDevice, graphicsFences[i].fence.vkFence, nullptr);
            EWE_VK(vkDestroySemaphore, VK::Object->vkDevice, semaphores[i].vkSemaphore, nullptr);
            EWE_VK(vkDestroySemaphore, VK::Object->vkDevice, semaphores[i + size].vkSemaphore, nullptr);
        }

        graphicsFences.clear();
        transferFences.clear();
        semaphores.clear();

        std::vector<VkCommandBuffer> rawCmdBufs(size);
        for (uint8_t queue = 0; queue < Queue::_count; queue++) {
            if (VK::Object->commandPools[queue] != VK_NULL_HANDLE) {
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
    Semaphore& QueueSyncPool::GetSemaphore(VkSemaphore semaphore) {
        for (uint8_t i = 0; i < size * 2; i++) {
            if (semaphores[i].vkSemaphore == semaphore) {
                return semaphores[i];
            }
        }
        assert(false && "failed to find SemaphoreData");
        return semaphores[0]; //DO NOT return this, error silencing
        //only way for this to not return an error is if the return type is changed to pointer and nullptr is returned if not found, or std::conditional which im not a fan of
    }
#endif
    bool QueueSyncPool::CheckFencesForUsage() {
        for (uint16_t i = 0; i < size; i++) {
            if (transferFences[i].fence.inUse) {
                return true;
            }
        }
        for (uint16_t i = 0; i < size; i++) {
            if (graphicsFences[i].fence.inUse) {
                return true;
            }
        }
        return false;
    }

    void QueueSyncPool::EndSingleTimeCommandGraphicsGroup(CommandBuffer& cmdBuf, std::vector<Semaphore*> waitSemaphores, std::vector<ImageInfo*> imageInfos) {
        EWE_VK(vkEndCommandBuffer, cmdBuf);
        graphicsAsyncMut.lock();
        graphicsSTCGroup.emplace_back(cmdBuf, waitSemaphores, imageInfos);
        graphicsAsyncMut.unlock();
    }

    template<typename T>
    void InsertSecondIntoFirst(std::vector<T>& first, std::vector<T>& second) {
        if (second.size() > 0) {
            first.insert(first.begin(), std::make_move_iterator(second.begin()), std::make_move_iterator(second.end()));
        }
    }

    void QueueSyncPool::HandleTransferCallbacks() {
        std::vector<TransferCommandCallbacks> callbacks{};
        for (uint16_t i = 0; i < size; i++) {
            if (transferFences[i].fence.inUse) {
                //the pipeline barriers should already be simplified, might be worth calling it again in case 2 transfers submitted before graphics got to it
                //probably worth profiling

#if DEBUGGING_FENCES
                transferFences[i].fence.log.push_back("beginning transfer fence check return");
#endif
                transferFences[i].WaitReturnCallbacks(callbacks, 0);
            }
        }
        if (callbacks.size() == 0) {
            return;
        }

        std::vector<Semaphore*> semaphoreData{};
        if ((callbacks[0].images.size() > 0) || (callbacks[0].pipeBarriers.size() > 0)) {
            assert(callbacks[0].semaphore != nullptr);
            semaphoreData.push_back(callbacks[0].semaphore);
        }

        for (int i = 1; i < callbacks.size(); i++) {
            InsertSecondIntoFirst(callbacks[0].commands, callbacks[i].commands);
            InsertSecondIntoFirst(callbacks[0].pipeBarriers, callbacks[i].pipeBarriers);
            InsertSecondIntoFirst(callbacks[0].images, callbacks[i].images);
            if (callbacks[i].semaphore != nullptr) {
                semaphoreData.push_back(callbacks[i].semaphore);
            }
#if EWE_DEBUG
            if ((callbacks[i].images.size() > 0) || (callbacks[i].pipeBarriers.size() > 0)) {

                assert(callbacks[i].semaphore != nullptr);
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

            std::vector<ImageInfo*> genMipImages{};
            for (auto& img : callbacks[0].images) {
                if (img->descriptorImageInfo.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
                    genMipImages.push_back(img);
                }
            }
            if (genMipImages.size() > 1) {
                Image::GenerateMipMapsForMultipleImagesTransferQueue(cmdBuf, genMipImages);
            }
            else if (genMipImages.size() == 1) {
                Image::GenerateMipmaps(cmdBuf, genMipImages[0], Queue::transfer);
            }
        
            for (auto& barrier : callbacks[0].pipeBarriers) {
                barrier.Submit(cmdBuf);
            }
            EndSingleTimeCommandGraphicsGroup(cmdBuf, semaphoreData, callbacks[0].images);
        }
    }

    void QueueSyncPool::CheckFencesForCallbacks() {
        if (std::this_thread::get_id() != VK::Object->mainThreadID) {
            assert(false);
        }

        ThreadPool::EnqueueVoid(&QueueSyncPool::HandleTransferCallbacks, this);
        

        std::vector<CommandBuffer*> graphicsCmdBuf{};
        for (uint16_t i = 0; i < size; i++) {
            if (graphicsFences[i].fence.inUse) {
#if DEBUGGING_FENCES
                graphicsFences[i].fence.log.push_back("beginning graphics fence check return");
#endif
                graphicsFences[i].CheckReturn(graphicsCmdBuf, 0);
            }
        }
        if (graphicsCmdBuf.size() > 0) {
            ResetCommandBuffers(graphicsCmdBuf, Queue::graphics);
        }
    }
#if SEMAPHORE_TRACKING
    Semaphore* QueueSyncPool::GetSemaphoreForSignaling(std::source_location srcLoc) {
#else
    Semaphore* QueueSyncPool::GetSemaphoreForSignaling() {
#endif
        std::unique_lock<std::mutex> semLock(semAcqMut);
        while (true) {
            for (uint16_t i = 0; i < size * 2; i++) {
                if (semaphores[i].Idle()) {
#if SEMAPHORE_TRACKING
                    semaphores[i].BeginSignaling(srcLoc);
#else
                    semaphores[i].BeginSignaling();
#endif
                    return &semaphores[i];
                }
            }
            //potentially add a resizing function here
            assert(false && "no semaphore available, if waiting for a semaphore to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    GraphicsFence& QueueSyncPool::GetGraphicsFence() {

        std::unique_lock<std::mutex> graphicsAcqLock(graphicsFenceAcqMut);
        while (true) {
            for (uint8_t i = 0; i < size; i++) {
                if (!graphicsFences[i].fence.inUse) {
                    graphicsFences[i].fence.inUse = true;
#if DEBUGGING_FENCES
                    graphicsFences[i].fence.log.push_back("set graphics fence to in-use");
#endif
                    return graphicsFences[i];
                }
            }
            //potentially add a resizing function here
            assert(false && "no available fence when requested, if waiting for a fence to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    TransferFence& QueueSyncPool::GetTransferFence() {
        std::unique_lock<std::mutex> transferFenceLock(transferFenceAcqMut);
        while (true) {
            for (uint8_t i = 0; i < size; i++) {
                if (!transferFences[i].fence.inUse) {
                    transferFences[i].fence.inUse = true;
#if DEBUGGING_FENCES
                    transferFences[i].fence.log.push_back("set transfer fence to in-use");
#endif
                    return transferFences[i];
                }
            }
            //potentially add a resizing function here
            assert(false && "no available fence when requested, if waiting for a fence to become available instead of crashing is acceptable, comment this line");
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    CommandBuffer& QueueSyncPool::GetCmdBufSingleTime(Queue::Enum queue) {
        std::unique_lock<std::mutex> cmdBufLock(cmdBufAcqMut);
        while (true) {
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
    void QueueSyncPool::ResetCommandBuffer(CommandBuffer& cmdBuf, Queue::Enum queue) {
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
    void QueueSyncPool::ResetCommandBuffer(CommandBuffer& cmdBuf) {
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
    void QueueSyncPool::ResetCommandBuffers(std::vector<CommandBuffer*>& cmdBufVec, Queue::Enum queue) {
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
    void QueueSyncPool::ResetCommandBuffers(std::vector<CommandBuffer*>& cmdBufVec) {

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