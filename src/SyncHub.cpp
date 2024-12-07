#include "EWEngine/Systems/SyncHub.h"

#include <future>
#include <cassert>

namespace EWE {
	SyncHub* SyncHub::syncHubSingleton{ nullptr };

	SyncHub::SyncHub() :
		qSyncPool{ 32 }, 
		renderSyncData{}
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
#if DEBUG_NAMING
		DebugNaming::SetObjectName(VK::Object->commandPools[Queue::transfer], VK_OBJECT_TYPE_COMMAND_BUFFER, "transfer command pool");
		DebugNaming::SetObjectName(VK::Object->commandPools[Queue::graphics], VK_OBJECT_TYPE_COMMAND_BUFFER, "graphics STG command pool");
		DebugNaming::SetObjectName(VK::Object->renderCmdPool, VK_OBJECT_TYPE_COMMAND_BUFFER, "render command pool");
#endif

		syncHubSingleton->transferSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		syncHubSingleton->transferSubmitInfo.pNext = nullptr;

		syncHubSingleton->CreateBuffers();

		syncHubSingleton->CreateSyncObjects();
	}
	void SyncHub::Destroy() {
		assert(syncHubSingleton != nullptr);
#if DECONSTRUCTION_DEBUG
		printf("beginniing synchub destroy \n");
#endif

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
#if COMMAND_BUFFER_TRACING
			if (VK::Object->renderCommands[i].cmdBuf != VK_NULL_HANDLE) {
#else
			if (VK::Object->renderCommands[i] != VK_NULL_HANDLE) {
#endif
				EWE_VK(vkFreeCommandBuffers, VK::Object->vkDevice, VK::Object->renderCmdPool, 1, &VK::Object->renderCommands[i]);
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
		allocInfo.commandPool = VK::Object->renderCmdPool;
		allocInfo.commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
#if COMMAND_BUFFER_TRACING
		std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> tempCmdBuf{};
		EWE_VK(vkAllocateCommandBuffers, VK::Object->vkDevice, &allocInfo, &tempCmdBuf[0]);
		VK::Object->renderCommands[0].cmdBuf = tempCmdBuf[0];
		VK::Object->renderCommands[1].cmdBuf = tempCmdBuf[1];
#else
		EWE_VK(vkAllocateCommandBuffers, VK::Object->vkDevice, &allocInfo, VK::Object->renderCommands.data());
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
		if (std::this_thread::get_id() != VK::Object->mainThreadID) {
			assert(false && "graphics queue STC not on main thread");
		}
#endif
#if DEBUG_NAMING
		CommandBuffer& ret = qSyncPool.GetCmdBufSingleTime(Queue::graphics);
		DebugNaming::SetObjectName(ret.cmdBuf, VK_OBJECT_TYPE_COMMAND_BUFFER, "single time cmd buf[graphics]");
		return ret;
#else
		return qSyncPool.GetCmdBufSingleTime(Queue::graphics);
#endif
	}
	CommandBuffer& SyncHub::BeginSingleTimeCommandTransfer() {
#if DEBUG_NAMING
		CommandBuffer& ret = qSyncPool.GetCmdBufSingleTime(Queue::transfer);
		DebugNaming::SetObjectName(ret.cmdBuf, VK_OBJECT_TYPE_COMMAND_BUFFER, "single time cmd buf[transfer]");
		return ret;
#else
		return qSyncPool.GetCmdBufSingleTime(Queue::transfer);
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

		EWE_VK(vkResetFences, VK::Object->vkDevice, 1, &singleTimeFenceGraphics);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
#if COMMAND_BUFFER_TRACING
		submitInfo.pCommandBuffers = &cmdBuf.cmdBuf;
#else
		submitInfo.pCommandBuffers = &cmdBuf;
#endif
		EWE_VK(vkQueueSubmit, VK::Object->queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics);

		EWE_VK(vkWaitForFences, VK::Object->vkDevice, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX);

		qSyncPool.ResetCommandBuffer(cmdBuf, Queue::graphics);
	}

	void SyncHub::RunGraphicsCallbacks() {
		qSyncPool.CheckFencesForCallbacks();
#if DEBUG_NAMING
		DebugNaming::QueueBegin(VK::Object->queues[Queue::graphics], 1.f, 0.f, 0.f, "graphics callbacks");
#endif
		qSyncPool.graphicsAsyncMut.lock();
		if (qSyncPool.graphicsSTCGroup.size() == 0) {
			qSyncPool.graphicsAsyncMut.unlock();
			return;
		}

		std::vector<Semaphore*> semaphores{};
		std::vector<CommandBuffer*> submittedAsyncBuffers{};
		std::vector<ImageInfo*> imageInfos{};
		for (int i = 0; i < qSyncPool.graphicsSTCGroup.size(); i++) {
			submittedAsyncBuffers.push_back(qSyncPool.graphicsSTCGroup[i].cmdBuf);
			if (qSyncPool.graphicsSTCGroup[i].semaphores.size() > 0) {
				semaphores.insert(semaphores.end(), qSyncPool.graphicsSTCGroup[i].semaphores.begin(), qSyncPool.graphicsSTCGroup[i].semaphores.end());
				imageInfos.insert(imageInfos.end(), qSyncPool.graphicsSTCGroup[i].imageInfos.begin(), qSyncPool.graphicsSTCGroup[i].imageInfos.end());
			}
		}
		qSyncPool.graphicsSTCGroup.clear();
		qSyncPool.graphicsAsyncMut.unlock();

		std::vector<VkSemaphore> waitSems{};
		std::vector<VkPipelineStageFlags> waitStages{};
		for (int i = 0; i < semaphores.size(); i++) {
			waitSems.push_back(semaphores[i]->vkSemaphore);
			waitStages.push_back(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = submittedAsyncBuffers.size();
		assert(submitInfo.commandBufferCount != 0 && "had graphics callbacks but don't have any command buffers");

		std::vector<VkCommandBuffer> cmds{};
		cmds.reserve(submittedAsyncBuffers.size());
		for (auto& command : submittedAsyncBuffers) {
#if COMMAND_BUFFER_TRACING
			cmds.push_back(command->cmdBuf);
#else
			cmds.push_back(*command);
#endif
		}
		submitInfo.pCommandBuffers = cmds.data();

		submitInfo.waitSemaphoreCount = waitSems.size();
		submitInfo.pWaitSemaphores = waitSems.data();

		Semaphore* signalSemaphore = qSyncPool.GetSemaphoreForSignaling();

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore->vkSemaphore;
		submitInfo.pWaitDstStageMask = waitStages.data();
		
		GraphicsFence& graphicsFence = qSyncPool.GetGraphicsFence();
		graphicsFence.fence.waitSemaphores = std::move(semaphores);
		graphicsFence.imageInfos = std::move(imageInfos);
		EWE_VK(vkQueueSubmit, VK::Object->queues[Queue::graphics], 1, &submitInfo, graphicsFence.fence.vkFence);

		/*
		* this is bad, but currently I'm just going to use VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT regardless
		* ideally, i could take the data from the object being uploaded
		** for example, if an image is only used in the fragment stage i could use VK_PIPELINE_STAGE_FRAGMENT_BIT,
		** or for a vertex/index/instancing buffer I could use VK_PIPELINE_STAGE_VERTEX_BIT,
		** or for a barrier I could use VK_PIPELINE_STAGE_TRANSFER_BIT
		* but I believe this would require a major overhaul (for the major overhaul I haven't even gotten working yet)
		*/
		renderSyncData.AddWaitSemaphore(signalSemaphore, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
		graphicsFence.commands = submittedAsyncBuffers;
#if DEBUGGING_FENCES
		graphicsFence.fence.log.push_back("setting graphics fence to submitted");
#endif
		graphicsFence.fence.submitted = true;
#if DEBUG_NAMING
		DebugNaming::QueueEnd(VK::Object->queues[Queue::graphics]);
#endif
	}

	void SyncHub::EndSingleTimeCommandTransfer() {
		/*
		* i suspect its possible for transfer commands to hang without being submitted
		*havent had the issue yet
		* potential solution is to have a dedicated thread just run submittransferbuffers on a loop
		* second potential solution is to have the RunGraphicsCallback function check for available commands, then spawn a thread that has a singular run thru of SubmitTransferBuffers every time commands exist, and SubmitTransferBuffers isn't currently active.

		* example of hang, SubmitTransferBuffers locks the TCM mutex with ::Empty
		* this function attempts to lock the TCM mutex with ::FinalizeCommand
		* SubmitTransferBuffer attempts to exit, but lags a little bit
		* this function completes ::FinalizeCommand and moves to try_lock, and fails to acquire lock, exits
		* SubmitTransferBuffers exits, and unlocks the transferSubmissionMut, with 1 command submitted to TCM
		
		* Might be exceptionally rare for this function to attempt the transferSubmissionMut lock before SubmitTransferBuffer
		* I'll worry about it when it becomes an issue, or I'll just fix it at some point later idk
		*/
		TransferCommandManager::FinalizeCommand();
		if(transferSubmissionMut.try_lock()){
				SubmitTransferBuffers();
				transferSubmissionMut.unlock();
		}
	}

	void SyncHub::SubmitTransferBuffers() {
		assert(!TransferCommandManager::Empty());
		while (!TransferCommandManager::Empty()) {

			TransferFence& transferFence = qSyncPool.GetTransferFence();
			transferFence.callbacks = TransferCommandManager::PrepareSubmit();

			transferSubmitInfo.commandBufferCount = static_cast<uint32_t>(transferFence.callbacks.commands.size());
			std::vector<VkCommandBuffer> cmds{};
			cmds.reserve(transferFence.callbacks.commands.size());

			for (auto& command : transferFence.callbacks.commands) {
#if COMMAND_BUFFER_TRACING
				cmds.push_back(command->cmdBuf);
#else
				cmds.push_back(*command);
#endif
			}
			transferSubmitInfo.pCommandBuffers = cmds.data();

			std::vector<VkSemaphore> sigSems{};
			if ((transferFence.callbacks.images.size() > 0) || (transferFence.callbacks.pipeBarriers.size() > 0)) {
				transferFence.signalSemaphoreForGraphics = qSyncPool.GetSemaphoreForSignaling();
				PipelineBarrier::SimplifyVector(transferFence.callbacks.pipeBarriers);
				transferFence.callbacks.semaphore = transferFence.signalSemaphoreForGraphics;
				transferSubmitInfo.signalSemaphoreCount = 1;
				transferSubmitInfo.pSignalSemaphores = &transferFence.signalSemaphoreForGraphics->vkSemaphore;
			}
			else {
				transferSubmitInfo.pSignalSemaphores = nullptr;
				transferSubmitInfo.signalSemaphoreCount = 0;
			}
			transferSubmitInfo.pWaitSemaphores = nullptr;
			transferSubmitInfo.waitSemaphoreCount = 0;

			VK::Object->poolMutex[Queue::transfer].lock();
			EWE_VK(vkQueueSubmit, VK::Object->queues[Queue::transfer], 1, &transferSubmitInfo, transferFence.fence.vkFence);
			VK::Object->poolMutex[Queue::transfer].unlock();

#if DEBUGGING_FENCES
			transferFence.fence.log.push_back("setting transfer fence to submitted");
#endif
			transferFence.fence.submitted = true;
		}
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
	}

	VkResult SyncHub::PresentKHR(VkPresentInfoKHR& presentInfo) {

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderSyncData.renderFinishedSemaphore[VK::Object->frameIndex];
		return vkQueuePresentKHR(VK::Object->queues[Queue::present], &presentInfo);
	}
}