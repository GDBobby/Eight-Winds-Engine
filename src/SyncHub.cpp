#include "EWEngine/Systems/SyncHub.h"

#include <future>
#include <cassert>

namespace EWE {
	SyncHub* SyncHub::syncHubSingleton{ nullptr };

	SyncHub::SyncHub(VkDevice device, std::array<VkQueue, Queue::_count>& queues, std::array<VkCommandPool, Queue::_count>& cmdPools) :
		device{ device },
		queues{ queues },
		cmdPools{cmdPools},
		syncPool{ 32, device, cmdPools }, 
		renderSyncData{ device },
		main_thread{ std::this_thread::get_id() }
	{
#if EWE_DEBUG
		printf("COSTRUCTING SYNCHUB\n");
#endif
	}
#if EWE_DEBUG
	SyncHub::~SyncHub() {
		printf("DE COSTRUCTING SYNCHUB\n");
	}
#endif

	void SyncHub::Initialize(VkDevice device, std::array<VkQueue, Queue::_count>& queues, std::array<VkCommandPool, Queue::_count>& cmdPools, uint32_t transferQueueIndex) {
		syncHubSingleton = Construct<SyncHub>({ device, queues, cmdPools});

		SyncPool::SubmitGraphicsAsync = &SyncHub::EndSingleTimeCommandGraphicsGroup;

		syncHubSingleton->transferQueueIndex = transferQueueIndex;
#if DEBUG_NAMING
		DebugNaming::SetObjectName(device, cmdPools[Queue::transfer], VK_OBJECT_TYPE_COMMAND_BUFFER, "transfer command pool");
		DebugNaming::SetObjectName(device, cmdPools[Queue::graphics], VK_OBJECT_TYPE_COMMAND_BUFFER, "graphics command pool");
#endif


		syncHubSingleton->transferSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		syncHubSingleton->transferSubmitInfo.pNext = nullptr;

		syncHubSingleton->CreateBuffers(cmdPools[Queue::graphics]);

		syncHubSingleton->CreateSyncObjects();
	}
	void SyncHub::Destroy() {
#if DECONSTRUCTION_DEBUG
		printf("beginniing synchub destroy \n");
#endif

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (renderBuffers[i] != VK_NULL_HANDLE) {
				EWE_VK(vkFreeCommandBuffers, device, syncHubSingleton->cmdPools[Queue::graphics], 1, &renderBuffers[i]);
			}
		}

		Deconstruct(syncHubSingleton);
#if DECONSTRUCTION_DEBUG
		printf("end synchub destroy \n");
#endif
	}

	void SyncHub::CreateBuffers(VkCommandPool graphicsCommandPool) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = graphicsCommandPool;
		allocInfo.commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		EWE_VK(vkAllocateCommandBuffers, device, &allocInfo, renderBuffers);
	}

	void SyncHub::CreateSyncObjects() {

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		EWE_VK(vkCreateFence, device, &fenceInfo, nullptr, &singleTimeFenceGraphics);
	}

	CommandBufferData& SyncHub::BeginSingleTimeCommandGraphics() {
#if EWE_DEBUG
		if (std::this_thread::get_id() != main_thread) {
			assert(false && "graphics queue STC not on main thread");
		}
#endif
#if DEBUG_NAMING
		CommandBufferData& ret = syncPool.GetCmdBufSingleTime(Queue::graphics);
		DebugNaming::SetObjectName(device, ret.cmdBuf, VK_OBJECT_TYPE_COMMAND_BUFFER, "single time cmd buf[graphics]");
		return ret;
#else
		return syncPool.GetCmdBufSingleTime(Queue::graphics);
#endif
	}
	CommandBufferData& SyncHub::BeginSingleTimeCommandTransfer() {
#if DEBUG_NAMING
		CommandBufferData& ret = syncPool.GetCmdBufSingleTime(Queue::transfer);
		DebugNaming::SetObjectName(device, ret.cmdBuf, VK_OBJECT_TYPE_COMMAND_BUFFER, "single time cmd buf[transfer]");
		return ret;
#else
		return syncPool.GetCmdBufSingleTime(Queue::transfer);
#endif
	}

	CommandBufferData& SyncHub::BeginSingleTimeCommand(Queue::Enum queue) {
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

	void SyncHub::EndSingleTimeCommandGraphics(CommandBufferData& cmdBuf) {
		EWE_VK(vkEndCommandBuffer, cmdBuf.cmdBuf);
		///prepTransferSubmission(cmdBuf);

		EWE_VK(vkResetFences, device, 1, &singleTimeFenceGraphics);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf.cmdBuf;
		//std::cout << "before transfer submit \n";
		EWE_VK(vkQueueSubmit, queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics);
		//std::cout << "after transfer submit \n";

		EWE_VK(vkWaitForFences, device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX);

		//this should be redundant with the vkWaitForFences
		//EWE_VK(vkQueueWaitIdle(graphicsQueue));

		syncPool.ResetCommandBuffer(cmdBuf, Queue::graphics);
	}

	void SyncHub::EndSingleTimeCommandGraphicsGroup(CommandBufferData& cmdBuf, std::vector<SemaphoreData*> waitSemaphores) {
		EWE_VK(vkEndCommandBuffer, cmdBuf.cmdBuf);
		syncHubSingleton->graphicsAsyncMut.lock();
		syncHubSingleton->graphicsSTCGroup.emplace_back(cmdBuf, waitSemaphores);
		syncHubSingleton->graphicsAsyncMut.unlock();
	}

	void SyncHub::RunGraphicsCallbacks() {
		syncPool.CheckFencesForCallbacks();
#if DEBUG_NAMING
		DebugNaming::QueueBegin(queues[Queue::graphics], 1.f, 0.f, 0.f, "graphics callbacks");
#endif
		graphicsAsyncMut.lock();
		if (graphicsSTCGroup.size() == 0) {
			graphicsAsyncMut.unlock();
			return;
		}
		std::vector<SemaphoreData*> semaphoreDatas{};
		std::vector<CommandBufferData*> submittedAsyncBuffers{};
		for (int i = 0; i < graphicsSTCGroup.size(); i++) {
			submittedAsyncBuffers.push_back(graphicsSTCGroup[i].cmdBuf);
			semaphoreDatas.insert(semaphoreDatas.end(), graphicsSTCGroup[i].semaphoreData.begin(), graphicsSTCGroup[i].semaphoreData.end());
		}
		graphicsSTCGroup.clear();
		graphicsAsyncMut.unlock();
		std::vector<VkSemaphore> waitSems{};

		for (int i = 0; i < semaphoreDatas.size(); i++) {
			waitSems.push_back(semaphoreDatas[i]->semaphore);
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

#if SEMAPHORE_TRACKING
		static uint64_t graphicsCallbackSemaphoreCounter = 0;
		std::string graphicsCallbackSemaphoreName{ "graphics callback" };
		graphicsCallbackSemaphoreName += std::to_string(graphicsCallbackSemaphoreCounter++);
		SemaphoreData* signalSemaphore = syncPool.GetSemaphoreForSignaling(graphicsCallbackSemaphoreName.c_str());
#else
		SemaphoreData* signalSemaphore = syncPool.GetSemaphoreForSignaling();
#endif
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore->semaphore;
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT };
		submitInfo.pWaitDstStageMask = waitStages;
		
		FenceData& fence = syncPool.GetFence();
		fence.signalSemaphores[Queue::graphics] = signalSemaphore;
		fence.waitSemaphores = std::move(semaphoreDatas);
		EWE_VK(vkQueueSubmit, queues[Queue::graphics], 1, &submitInfo, fence.fence);


		/*
		* this is bad, but currently I'm just going to use VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT regardless
		* ideally, i could take the data from the object being uploaded
		** for example, if an image is only used in the fragment stage i could use VK_PIPELINE_STAGE_FRAGMENT_BIT,
		** or for a vertex/index/instancing buffer I could use VK_PIPELINE_STAGE_VERTEX_BIT,
		** or for a barrier I could use VK_PIPELINE_STAGE_TRANSFER_BIT
		* but I believe this would require a major overhaul (for the major overhaul I haven't even gotten working yet)
		*/
		renderSyncData.AddWaitSemaphore(signalSemaphore, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
		fence.callbackData.commands.insert(fence.callbackData.commands.end(), submittedAsyncBuffers.begin(), submittedAsyncBuffers.end());
		fence.Unlock();
#if DEBUG_NAMING
		DebugNaming::QueueEnd(queues[Queue::graphics]);
#endif
		//std::cout << "before transfer subm
	}

	void SyncHub::EndSingleTimeCommandGraphicsSignal(CommandBufferData& cmdBuf, VkSemaphore signalSemaphore){
		EWE_VK(vkEndCommandBuffer, cmdBuf.cmdBuf);
		EWE_VK(vkResetFences, device, 1, &singleTimeFenceGraphics);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf.cmdBuf;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
		//std::cout << "before graphics EST signal submit \n";
		EWE_VK(vkQueueSubmit, queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics);
		//std::cout << "after graphics EST signal submit \n";

		EWE_VK(vkWaitForFences, device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX);
		syncPool.ResetCommandBuffer(cmdBuf, Queue::graphics);
	}
	void SyncHub::EndSingleTimeCommandGraphicsWaitAndSignal(CommandBufferData& cmdBuf, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore){
		EWE_VK(vkEndCommandBuffer, cmdBuf.cmdBuf);
		EWE_VK(vkResetFences, device, 1, &singleTimeFenceGraphics);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf.cmdBuf;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
		//std::cout << "before graphics EST signal submit \n";
		EWE_VK(vkQueueSubmit, queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics);
		//std::cout << "after graphics EST signal submit \n";

		EWE_VK(vkWaitForFences, device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX);
		syncPool.ResetCommandBuffer(cmdBuf, Queue::graphics);
	}

	//void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf){
	//	TransferCommandManager::AddCommand(cmdBuf);
	//	TransferCommandManager::FinalizeCommand();

	//	AttemptTransferSubmission();
	//}
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

			FenceData& fenceData = syncPool.GetFence();
			fenceData.callbackData = TransferCommandManager::PrepareSubmit();
			transferSubmitInfo.commandBufferCount = static_cast<uint32_t>(fenceData.callbackData.commands.size());
			std::vector<VkCommandBuffer> cmds{};
			cmds.reserve(fenceData.callbackData.commands.size());
			for (auto& command : fenceData.callbackData.commands) {
				cmds.push_back(command->cmdBuf);
			}
			transferSubmitInfo.pCommandBuffers = cmds.data();

			std::vector<VkSemaphore> sigSems{};
			if (fenceData.callbackData.mipParamPacks.size() > 0 || fenceData.callbackData.pipeBarriers.size() > 0) {
#if SEMAPHORE_TRACKING
				static uint64_t transferSignalSemaphoreCounter = 0;
				std::string transferSignalSemaphoreName = "transfer ";
				transferSignalSemaphoreName += std::to_string(transferSignalSemaphoreCounter++);
				fenceData.signalSemaphores[Queue::graphics] = syncPool.GetSemaphoreForSignaling(transferSignalSemaphoreName.c_str());
#else
				fenceData.signalSemaphores[Queue::graphics] = syncPool.GetSemaphoreForSignaling();
#endif
				PipelineBarrier::SimplifyVector(fenceData.callbackData.pipeBarriers);
			}
			

			for (uint8_t i = 0; i < Queue::_count; i++) {
				if (fenceData.signalSemaphores[i] != nullptr) {
					sigSems.push_back(fenceData.signalSemaphores[i]->semaphore);
					fenceData.signalSemaphores[i]->BeginWaiting();
				}
			}

			transferSubmitInfo.pSignalSemaphores = sigSems.data();
			transferSubmitInfo.signalSemaphoreCount = static_cast<uint32_t>(sigSems.size());

			transferSubmitInfo.pWaitSemaphores = nullptr;
			transferSubmitInfo.waitSemaphoreCount = 0;
		
			//printf("before transfer submit\n");
			transferPoolMutex.lock();
			EWE_VK(vkQueueSubmit, queues[Queue::transfer], 1, &transferSubmitInfo, fenceData.fence);
			transferPoolMutex.unlock();
			//printf("after transfer submit\n");
			fenceData.Unlock();
		}
	
		//std::cout << "after successful submission \n";
		readyForNextTransmit = true;
	}

	void SyncHub::SubmitGraphics(VkSubmitInfo& submitInfo, uint8_t frameIndex, uint32_t* imageIndex) {

		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			EWE_VK(vkWaitForFences, device, 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[*imageIndex] = renderSyncData.inFlight[frameIndex];

		renderSyncData.SetWaitData(frameIndex, submitInfo);

		std::vector<VkSemaphore> signalSemaphores = renderSyncData.GetSignalData(frameIndex);
		submitInfo.pSignalSemaphores = signalSemaphores.data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());

		EWE_VK(vkResetFences, device, 1, &renderSyncData.inFlight[frameIndex]);

		EWE_VK(vkQueueSubmit, queues[Queue::graphics], 1, &submitInfo, renderSyncData.inFlight[frameIndex]);

		//std::cout << "immediately after submitting graphics \n";
	}

	VkResult SyncHub::PresentKHR(VkPresentInfoKHR& presentInfo, uint8_t currentFrame) {

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderSyncData.renderFinishedSemaphore[currentFrame];
		//std::cout << "imeddiately before presenting \n";
		return vkQueuePresentKHR(queues[Queue::present], &presentInfo);
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