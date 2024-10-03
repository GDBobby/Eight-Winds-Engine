#include "EWEngine/Systems/SyncHub.h"

#include <future>
#include <cassert>

namespace EWE {
	SyncHub* SyncHub::syncHubSingleton{ nullptr };

	SyncHub::SyncHub(VkDevice device) : 
		device{ device }, 
		syncPool{ 32, device }, 
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

	void SyncHub::Initialize(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkQueue computeQueue, VkQueue transferQueue, VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool, uint32_t transferQueueIndex) {
		syncHubSingleton = new SyncHub(device);
		
		syncHubSingleton->queues[Queue::graphics] = graphicsQueue;
		syncHubSingleton->queues[Queue::present] = presentQueue;
		syncHubSingleton->queues[Queue::compute] = computeQueue;
		syncHubSingleton->queues[Queue::transfer] = transferQueue;

		syncHubSingleton->transferQueueIndex = transferQueueIndex;

		syncHubSingleton->commandPools[Queue::transfer] = transferCommandPool;
		syncHubSingleton->commandPools[Queue::graphics] = renderCommandPool;
#if DEBUG_NAMING
		DebugNaming::SetObjectName(device, transferCommandPool, VK_OBJECT_TYPE_COMMAND_BUFFER, "transfer command pool");
		DebugNaming::SetObjectName(device, renderCommandPool, VK_OBJECT_TYPE_COMMAND_BUFFER, "graphics command pool");
#endif

		syncHubSingleton->bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		syncHubSingleton->transferSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		syncHubSingleton->CreateBuffers(renderCommandPool);

		syncHubSingleton->CreateSyncObjects();
	}
	void SyncHub::Destroy(VkCommandPool renderPool, VkCommandPool computePool, VkCommandPool transferPool) {
#if DECONSTRUCTION_DEBUG
		printf("beginniing synchub destroy \n");
#endif

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (renderBuffers[i] != VK_NULL_HANDLE) {
				vkFreeCommandBuffers(device, renderPool, 1, &renderBuffers[i]);
			}
		}

		syncHubSingleton->~SyncHub();
		ewe_free(syncHubSingleton);
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
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.pNext = nullptr;
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		EWE_VK(vkCreateFence, device, &fenceInfo, nullptr, &singleTimeFenceGraphics);
	}

	VkCommandBuffer SyncHub::BeginSingleTimeCommandGraphics() {
#if EWE_DEBUG
		if (std::this_thread::get_id() != main_thread) {
			printf("graphics queue STC not on main thread\n");
			assert(false && "graphics queue STC not on main thread");
		}
#endif

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = commandPools[Queue::graphics];

		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
		EWE_VK(vkAllocateCommandBuffers, device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		EWE_VK(vkBeginCommandBuffer, commandBuffer, &beginInfo);
#if DEBUG_NAMING
		DebugNaming::SetObjectName(device, commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, "single time cmd buf[graphics]");
#endif
		return commandBuffer;
	}
	VkCommandBuffer SyncHub::BeginSingleTimeCommandTransfer() {

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = commandPools[Queue::transfer];

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
		transferPoolMutex.lock();
		EWE_VK(vkAllocateCommandBuffers, device, &allocInfo, &commandBuffer);
		EWE_VK(vkBeginCommandBuffer, commandBuffer, &beginInfo);
		transferPoolMutex.unlock();
#if DEBUG_NAMING
		DebugNaming::SetObjectName(device, commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, "single time cmd buf[transfer]");
#endif
		return commandBuffer;
	}

	VkCommandBuffer SyncHub::BeginSingleTimeCommand(Queue::Enum queue) {
		if (queue == Queue::graphics) {
			return BeginSingleTimeCommandGraphics();
		}
		else if (queue == Queue::transfer) {
			return BeginSingleTimeCommandTransfer();
		}
		else {
			assert(false && "unsupported queue");
			return VK_NULL_HANDLE;
		}
	}

	void SyncHub::EndSingleTimeCommandGraphics(VkCommandBuffer cmdBuf) {
		EWE_VK(vkEndCommandBuffer, cmdBuf);
		///prepTransferSubmission(cmdBuf);

		EWE_VK(vkResetFences, device, 1, &singleTimeFenceGraphics);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf;
		//std::cout << "before transfer submit \n";
		EWE_VK(vkQueueSubmit, queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics);
		//std::cout << "after transfer submit \n";

		EWE_VK(vkWaitForFences, device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX);

		//this should be redundant with the vkWaitForFences
		//EWE_VK(vkQueueWaitIdle(graphicsQueue));

		EWE_VK(vkFreeCommandBuffers, device, commandPools[Queue::graphics], 1, &cmdBuf);
	}

	void SyncHub::EndSingleTimeCommandGraphicsGroup(VkCommandBuffer cmdBuf) {
		EWE_VK(vkEndCommandBuffer, cmdBuf);
		graphicsSTCGroup.push_back(cmdBuf);
	}

	void SyncHub::RunGraphicsCallbacks() {
		syncPool.CheckFencesForCallbacks(transferPoolMutex);

		if (transitionManager.Empty()) {
			return;
		}

#if DEBUG_NAMING
		DebugNaming::QueueBegin(queues[Queue::graphics], 1.f, 0.f, 0.f, "graphics callbacks");
#endif
		TransitionData transitionData = transitionManager.Pull();

		transitionData.callback();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = graphicsSTCGroup.size();
#if EWE_DEBUG
		if (submitInfo.commandBufferCount == 0) {
			assert(false && "had graphics callbacks but don't have any command buffers");
		}
#endif
		submitInfo.pCommandBuffers = graphicsSTCGroup.data();

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &transitionData.waitSemaphore->semaphore;

		SemaphoreData* signalSemaphore = syncPool.GetSemaphore();
#if SEMAPHORE_TRACKING
		static uint64_t graphicsCallbackSemaphoreCounter = 0;
		std::string graphicsCallbackSemaphoreName{ "graphics callback" };
		graphicsCallbackSemaphoreName += std::to_string(graphicsCallbackSemaphoreCounter++);
		signalSemaphore->BeginSignaling(graphicsCallbackSemaphoreName.c_str());
#else
		signalSemaphore->BeginSignaling();
#endif
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore->semaphore;
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT };
		submitInfo.pWaitDstStageMask = waitStages;
		
		FenceData& fence = syncPool.GetFence();
		fence.signalSemaphores[Queue::graphics] = signalSemaphore;
		fence.waitSemaphores.push_back(transitionData.waitSemaphore);
		fence.Lock();
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

		fence.inlineCallbacks = [this, cbs = graphicsSTCGroup] {
			vkFreeCommandBuffers(device, commandPools[Queue::graphics], static_cast<uint32_t>(cbs.size()), cbs.data());
		};
		fence.Unlock();
#if DEBUG_NAMING
		DebugNaming::QueueEnd(queues[Queue::graphics]);
#endif
		graphicsSTCGroup.clear();
		//std::cout << "before transfer subm
	}

	void SyncHub::EndSingleTimeCommandGraphicsSignal(VkCommandBuffer cmdBuf, VkSemaphore signalSemaphore){
		EWE_VK(vkEndCommandBuffer, cmdBuf);
		EWE_VK(vkResetFences, device, 1, &singleTimeFenceGraphics);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
		//std::cout << "before graphics EST signal submit \n";
		EWE_VK(vkQueueSubmit, queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics);
		//std::cout << "after graphics EST signal submit \n";

		EWE_VK(vkWaitForFences, device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX);
		EWE_VK(vkFreeCommandBuffers, device, commandPools[Queue::graphics], 1, &cmdBuf);
	}
	void SyncHub::EndSingleTimeCommandGraphicsWaitAndSignal(VkCommandBuffer cmdBuf, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore){
		EWE_VK(vkEndCommandBuffer, cmdBuf);
		EWE_VK(vkResetFences, device, 1, &singleTimeFenceGraphics);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
		//std::cout << "before graphics EST signal submit \n";
		EWE_VK(vkQueueSubmit, queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics);
		//std::cout << "after graphics EST signal submit \n";

		EWE_VK(vkWaitForFences, device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX);
		EWE_VK(vkFreeCommandBuffers, device, commandPools[Queue::graphics], 1, &cmdBuf);
	}

	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf){
		EWE_VK(vkEndCommandBuffer, cmdBuf);
		TransferCommandManager::AddCommand(cmdBuf);

		AttemptTransferSubmission();
	}
	void SyncHub::EndSingleTimeCommandTransfer(CommandWithCallback cmdCb) {
		transferPoolMutex.lock();
		EWE_VK(vkEndCommandBuffer, cmdCb.cmdBuf);
		transferPoolMutex.unlock();
		TransferCommandManager::AddCommand(cmdCb);
		
		AttemptTransferSubmission();
	}
#if SYNC_QUEUE
	void SyncHub::EndSingleTimeCommandTransfer(SyncedCommandQueue* cmdQueue){
		TransferCommandManager::EndCommandQueue(cmdQueue);

		AttemptTransferSubmission();
	}
#endif
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
#endif
		auto cmdCbs = TransferCommandManager::PrepareSubmit();
		while (cmdCbs.commands.size() > 0) {
#if EWE_DEBUG
			printf("looping in submit transfer buffers : %d\n", debugLoopTracker++);
#endif
			transferSubmitInfo.commandBufferCount = cmdCbs.commands.size();
			transferSubmitInfo.pCommandBuffers = cmdCbs.commands.data();

			FenceData& fenceData = syncPool.GetFence();
			fenceData.transferCallbacks = cmdCbs.CombineCallbacks(device, commandPools[Queue::transfer]);
			std::function<void()> graphicsCallbacks = nullptr;

			if (cmdCbs.CleanGraphicsCallbacks()) {
				fenceData.signalSemaphores[Queue::graphics] = syncPool.GetSemaphore();
#if SEMAPHORE_TRACKING
				static uint64_t transferSignalSemaphoreCounter = 0;
				std::string transferSignalSemaphoreName = "transfer ";
				transferSignalSemaphoreName += std::to_string(transferSignalSemaphoreCounter++);

				fenceData.signalSemaphores[Queue::graphics]->BeginSignaling(transferSignalSemaphoreName.c_str());
#else
				fenceData.signalSemaphores[Queue::graphics]->BeginSignaling();
#endif
				graphicsCallbacks = cmdCbs.CombineGraphicsCallbacks();
			}

			std::vector<VkSemaphore> sigSems{};
			for (uint8_t i = 0; i < Queue::_count; i++) {
				if (fenceData.signalSemaphores[i] != nullptr) {
					sigSems.push_back(fenceData.signalSemaphores[i]->semaphore);
				}
			}

			transferSubmitInfo.pSignalSemaphores = sigSems.data();
			transferSubmitInfo.signalSemaphoreCount = static_cast<uint32_t>(sigSems.size());

			std::vector<VkSemaphore> waitSems{};
			waitSems.reserve(fenceData.waitSemaphores.size());
			for (auto const& sem : fenceData.waitSemaphores) {
				waitSems.push_back(sem->semaphore);
			}

			transferSubmitInfo.pWaitSemaphores = waitSems.data();
			transferSubmitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSems.size());
		
			//printf("before transfer submit\n");
			transferPoolMutex.lock();
			EWE_VK(vkQueueSubmit, queues[Queue::transfer], 1, &transferSubmitInfo, fenceData.fence);
			transferPoolMutex.unlock();
			//printf("after transfer submit\n");

			if (graphicsCallbacks != nullptr) {
				fenceData.signalSemaphores[Queue::graphics]->BeginWaiting();
				transitionManager.Add(graphicsCallbacks, fenceData.signalSemaphores[Queue::graphics]);
#if EWE_DEBUG
				printf("signal semaphore address : %zu\n", reinterpret_cast<std::size_t>(fenceData.signalSemaphores[Queue::graphics]));
#endif
			}

			transferPoolMutex.lock();
			transferPoolMutex.unlock();

			cmdCbs = TransferCommandManager::PrepareSubmit();
		}
	
		//std::cout << "after successful submission \n";
		readyForNextTransmit = true;
	}

	void SyncHub::SubmitGraphics(VkSubmitInfo& submitInfo, uint8_t frameIndex, uint32_t* imageIndex) {

		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) [[likely]] {
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