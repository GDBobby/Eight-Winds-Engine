#include "EWEngine/Systems/SyncHub.h"

#include <future>
#include <cassert>

namespace EWE {
	SyncHub* SyncHub::syncHubSingleton{ nullptr };

	SyncHub::SyncHub() :
		syncPool{ 32 }, 
		renderSyncData{},
		main_thread{ std::this_thread::get_id() }
	{
#if EWE_DEBUG

		printf("CONSTRUCTING SYNCHUB\n");
#endif
	}
#if EWE_DEBUG
	SyncHub::~SyncHub() {
#if DECONSTRUCTION_DEBUG
		printf("DECONSTRUCTING SYNCHUB\n");
#endif
	}
#endif

	void SyncHub::Initialize() {
		syncHubSingleton = Construct<SyncHub>({});

		SyncPool::SubmitGraphicsAsync = &SyncHub::EndSingleTimeCommandGraphicsGroup;
#if DEBUG_NAMING
		DebugNaming::SetObjectName(VK::Object->commandPools[Queue::transfer], VK_OBJECT_TYPE_COMMAND_BUFFER, "transfer command pool");
		DebugNaming::SetObjectName(VK::Object->commandPools[Queue::graphics], VK_OBJECT_TYPE_COMMAND_BUFFER, "graphics command pool");
		DebugNaming::SetObjectName(VK::Object->STGCmdPool, VK_OBJECT_TYPE_COMMAND_BUFFER, "STG command pool");
#endif

		syncHubSingleton->transferSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		syncHubSingleton->transferSubmitInfo.pNext = nullptr;

		syncHubSingleton->CreateBuffers();

		syncHubSingleton->CreateSyncObjects();
	}
	void SyncHub::Destroy() {
#if DECONSTRUCTION_DEBUG
		printf("beginniing synchub destroy \n");
#endif

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (VK::Object->renderCommands[i].cmdBuf != VK_NULL_HANDLE) {
				EWE_VK(vkFreeCommandBuffers, VK::Object->vkDevice, VK::Object->commandPools[Queue::graphics], 1, &VK::Object->renderCommands[i]);
			}
		}

		Deconstruct(syncHubSingleton);
		syncHubSingleton = nullptr;

#if DECONSTRUCTION_DEBUG
		printf("end synchub destroy \n");
#endif
	}

	void SyncHub::CreateBuffers() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VK::Object->commandPools[Queue::graphics];
		allocInfo.commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
#if COMMAND_BUFFER_TRACING
		std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> tempCmdBuf{};
		EWE_VK(vkAllocateCommandBuffers, VK::Object->vkDevice, &allocInfo, &tempCmdBuf[0]);
		VK::Object->renderCommands[0].cmdBuf = tempCmdBuf[0];
		VK::Object->renderCommands[1].cmdBuf = tempCmdBuf[1];
#else
		EWE_VK(vkAllocateCommandBuffers, VK::Object->vkDevice, &allocInfo, VK::Object->renderCommands);
#endif
	}

	void SyncHub::CreateSyncObjects() {

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		EWE_VK(vkCreateFence, VK::Object->vkDevice, &fenceInfo, nullptr, &singleTimeFenceGraphics);
	}

	CommandBuffer& SyncHub::BeginSingleTimeCommandGraphics() {
#if EWE_DEBUG
		if (std::this_thread::get_id() != main_thread) {
			assert(false && "graphics queue STC not on main thread");
		}
#endif
#if DEBUG_NAMING
		CommandBuffer& ret = syncPool.GetCmdBufSingleTime(Queue::graphics);
		DebugNaming::SetObjectName(ret.cmdBuf, VK_OBJECT_TYPE_COMMAND_BUFFER, "single time cmd buf[graphics]");
		return ret;
#else
		return syncPool.GetCmdBufSingleTime(Queue::graphics);
#endif
	}
	CommandBuffer& SyncHub::BeginSingleTimeCommandTransfer() {
#if DEBUG_NAMING
		CommandBuffer& ret = syncPool.GetCmdBufSingleTime(Queue::transfer);
		DebugNaming::SetObjectName(ret.cmdBuf, VK_OBJECT_TYPE_COMMAND_BUFFER, "single time cmd buf[transfer]");
		return ret;
#else
		return syncPool.GetCmdBufSingleTime(Queue::transfer);
#endif
	}

	CommandBuffer& SyncHub::BeginSingleTimeCommand(Queue::Enum queue) {
		if (queue == Queue::graphics) {
			return BeginSingleTimeCommandGraphics();
		}
		else if (queue == Queue::transfer) {
			return BeginSingleTimeCommandTransfer();
		}
		else {
			assert(false && "unsupported queue");
			return BeginSingleTimeCommandGraphics(); //error silencing, DO NOT RETURN THIS
		}
	}

	void SyncHub::EndSingleTimeCommandGraphics(CommandBuffer& cmdBuf) {
		EWE_VK(vkEndCommandBuffer, cmdBuf);
		///prepTransferSubmission(cmdBuf);

		EWE_VK(vkResetFences, VK::Object->vkDevice, 1, &singleTimeFenceGraphics);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf.cmdBuf;
		//std::cout << "before transfer submit \n";
		EWE_VK(vkQueueSubmit, VK::Object->queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics);
		//std::cout << "after transfer submit \n";

		EWE_VK(vkWaitForFences, VK::Object->vkDevice, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX);

		//this should be redundant with the vkWaitForFences
		//EWE_VK(vkQueueWaitIdle(graphicsQueue));

		syncPool.ResetCommandBuffer(cmdBuf, Queue::graphics);
	}

	void SyncHub::EndSingleTimeCommandGraphicsGroup(CommandBuffer& cmdBuf, std::vector<SemaphoreData*> waitSemaphores) {
		EWE_VK(vkEndCommandBuffer, cmdBuf);
		syncHubSingleton->graphicsAsyncMut.lock();
		syncHubSingleton->graphicsSTCGroup.emplace_back(cmdBuf, waitSemaphores);
		syncHubSingleton->graphicsAsyncMut.unlock();
	}

	void SyncHub::RunGraphicsCallbacks() {
		syncPool.CheckFencesForCallbacks();
#if DEBUG_NAMING
		DebugNaming::QueueBegin(VK::Object->queues[Queue::graphics], 1.f, 0.f, 0.f, "graphics callbacks");
#endif
		graphicsAsyncMut.lock();
		if (graphicsSTCGroup.size() == 0) {
			graphicsAsyncMut.unlock();
			return;
		}
		std::vector<SemaphoreData*> semaphoreDatas{};
		std::vector<CommandBuffer*> submittedAsyncBuffers{};
		for (int i = 0; i < graphicsSTCGroup.size(); i++) {
			submittedAsyncBuffers.push_back(graphicsSTCGroup[i].cmdBuf);
			if (graphicsSTCGroup[i].semaphoreData.size() > 0) {
				semaphoreDatas.insert(semaphoreDatas.end(), graphicsSTCGroup[i].semaphoreData.begin(), graphicsSTCGroup[i].semaphoreData.end());
			}
		}
		graphicsSTCGroup.clear();
		graphicsAsyncMut.unlock();
		std::vector<VkSemaphore> waitSems{};
		std::vector<VkPipelineStageFlags> waitStages{};
		for (int i = 0; i < semaphoreDatas.size(); i++) {
			waitSems.push_back(semaphoreDatas[i]->semaphore);
			waitStages.push_back(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = submittedAsyncBuffers.size();
#if EWE_DEBUG
		if (submitInfo.commandBufferCount == 0) {
			assert(false && "had graphics callbacks but don't have any command buffers");
		}
#endif
		std::vector<VkCommandBuffer> cmds{};
		cmds.reserve(submittedAsyncBuffers.size());
		for (auto& command : submittedAsyncBuffers) {
			cmds.push_back(command->cmdBuf);
		}
		submitInfo.pCommandBuffers = cmds.data();

		submitInfo.waitSemaphoreCount = waitSems.size();
		submitInfo.pWaitSemaphores = waitSems.data();

		SemaphoreData* signalSemaphore = syncPool.GetSemaphoreForSignaling();

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore->semaphore;
		submitInfo.pWaitDstStageMask = waitStages.data();
		
		GraphicsFenceData& fence = syncPool.GetGraphicsFence();
		fence.fenceData.waitSemaphores = std::move(semaphoreDatas);
		EWE_VK(vkQueueSubmit, VK::Object->queues[Queue::graphics], 1, &submitInfo, fence.fenceData.fence);


		/*
		* this is bad, but currently I'm just going to use VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT regardless
		* ideally, i could take the data from the object being uploaded
		** for example, if an image is only used in the fragment stage i could use VK_PIPELINE_STAGE_FRAGMENT_BIT,
		** or for a vertex/index/instancing buffer I could use VK_PIPELINE_STAGE_VERTEX_BIT,
		** or for a barrier I could use VK_PIPELINE_STAGE_TRANSFER_BIT
		* but I believe this would require a major overhaul (for the major overhaul I haven't even gotten working yet)
		*/
		renderSyncData.AddWaitSemaphore(signalSemaphore, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
		fence.commands = submittedAsyncBuffers;
		fence.fenceData.mut.unlock();
#if DEBUG_NAMING
		DebugNaming::QueueEnd(VK::Object->queues[Queue::graphics]);
#endif
		//std::cout << "before transfer subm
	}

	void SyncHub::EndSingleTimeCommandTransfer() {

		TransferCommandManager::FinalizeCommand();
		AttemptTransferSubmission();
	}


	void SyncHub::AttemptTransferSubmission(){
		if (readyForNextTransmit) {
			readyForNextTransmit = false;
			transferFlipFlop = !transferFlipFlop;
			SubmitTransferBuffers();
		}
	}

	void SyncHub::SubmitTransferBuffers() {
#if EWE_DEBUG
		int debugLoopTracker = 0;
		assert(!TransferCommandManager::Empty());
#endif
		while (!TransferCommandManager::Empty()) {
#if EWE_DEBUG
			printf("looping in submit transfer buffers : %d\n", debugLoopTracker++);
#endif

			TransferFenceData& fenceData = syncPool.GetTransferFence();
			fenceData.callbacks = TransferCommandManager::PrepareSubmit();
			transferSubmitInfo.commandBufferCount = static_cast<uint32_t>(fenceData.callbacks.commands.size());
			std::vector<VkCommandBuffer> cmds{};
			cmds.reserve(fenceData.callbacks.commands.size());
			for (auto& command : fenceData.callbacks.commands) {
				cmds.push_back(command->cmdBuf);
			}
			transferSubmitInfo.pCommandBuffers = cmds.data();

			std::vector<VkSemaphore> sigSems{};
			if ((fenceData.callbacks.images.size() > 0) || (fenceData.callbacks.pipeBarriers.size() > 0)) {
				fenceData.signalSemaphoreForGraphics = syncPool.GetSemaphoreForSignaling();
				PipelineBarrier::SimplifyVector(fenceData.callbacks.pipeBarriers);
				fenceData.callbacks.semaphoreData = fenceData.signalSemaphoreForGraphics;
				transferSubmitInfo.pSignalSemaphores = &fenceData.signalSemaphoreForGraphics->semaphore;
				transferSubmitInfo.signalSemaphoreCount = 1;
			}
			else {
				transferSubmitInfo.pSignalSemaphores = nullptr;
				transferSubmitInfo.signalSemaphoreCount = 0;
			}


			transferSubmitInfo.pWaitSemaphores = nullptr;
			transferSubmitInfo.waitSemaphoreCount = 0;
		
			//printf("before transfer submit\n");
			VK::Object->poolMutex[Queue::transfer].lock();
			EWE_VK(vkQueueSubmit, VK::Object->queues[Queue::transfer], 1, &transferSubmitInfo, fenceData.fenceData.fence);
			VK::Object->poolMutex[Queue::transfer].unlock();
			//printf("after transfer submit\n");
			fenceData.fenceData.mut.unlock();
		}
	
		//std::cout << "after successful submission \n";
		readyForNextTransmit = true;
	}

	void SyncHub::SubmitGraphics(VkSubmitInfo& submitInfo, uint32_t* imageIndex) {

		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			EWE_VK(vkWaitForFences, VK::Object->vkDevice, 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[*imageIndex] = renderSyncData.inFlight[VK::Object->frameIndex];

		renderSyncData.SetWaitData(submitInfo);

		std::vector<VkSemaphore> signalSemaphores = renderSyncData.GetSignalData();
		submitInfo.pSignalSemaphores = signalSemaphores.data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());

		EWE_VK(vkResetFences, VK::Object->vkDevice, 1, &renderSyncData.inFlight[VK::Object->frameIndex]);

		EWE_VK(vkQueueSubmit, VK::Object->queues[Queue::graphics], 1, &submitInfo, renderSyncData.inFlight[VK::Object->frameIndex]);

		//std::cout << "immediately after submitting graphics \n";
	}

	VkResult SyncHub::PresentKHR(VkPresentInfoKHR& presentInfo) {

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderSyncData.renderFinishedSemaphore[VK::Object->frameIndex];
		//std::cout << "imeddiately before presenting \n";
		return vkQueuePresentKHR(VK::Object->queues[Queue::present], &presentInfo);
		//std::cout << "imediately after presenting \n";
	}

	void SyncHub::DomDemand() {

		{
			std::unique_lock<std::mutex> cuckLock(domCuckSync.cuckMutex);
			domCuckSync.cuckCondition.wait(cuckLock, [this]
				{ return !domCuckSync.cuckConditionHeld; }
			);
		}
		std::unique_lock<std::mutex> renderLock(domCuckSync.domMutex);
		domCuckSync.domConditionHeld = true;
	}
	void SyncHub::DomRelease() {
		domCuckSync.domConditionHeld = false;
		domCuckSync.domCondition.notify_one();
	}
	void SyncHub::CuckRequest() {
		{
			domCuckSync.cuckConditionHeld = true;
			std::unique_lock<std::mutex> cuckLock(domCuckSync.cuckMutex);
			std::unique_lock<std::mutex> domLock(domCuckSync.domMutex);

			domCuckSync.domCondition.wait(domLock, [this]
				{ return !domCuckSync.domConditionHeld; }
			);
		}
	}
	void SyncHub::CuckSubmit() {
		domCuckSync.cuckConditionHeld = false;
		domCuckSync.cuckCondition.notify_one();
	}
}