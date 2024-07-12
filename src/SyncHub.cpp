#include "EWEngine/Systems/SyncHub.h"

#include <future>
#include <cassert>

namespace EWE {
	SyncHub* SyncHub::syncHubSingleton{ nullptr };

	SyncHub::SyncHub(VkDevice device) : 
		device{ device }, 
		syncPool{ 32, device }, 
		renderSyncData{ device } 
	{
#ifdef _DEBUG
		printf("COSTRUCTING SYNCHUB\n");
#endif
	}
	SyncHub::~SyncHub() {
#ifdef _DEBUG
		printf("DE COSTRUCTING SYNCHUB\n");
#endif
	}

	void SyncHub::Initialize(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkQueue computeQueue, VkQueue transferQueue, VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool, uint32_t transferQueueIndex) {
		syncHubSingleton = new SyncHub(device);
		
		syncHubSingleton->queues[Queue::graphics] = graphicsQueue;
		syncHubSingleton->queues[Queue::present] = presentQueue;
		syncHubSingleton->queues[Queue::compute] = computeQueue;
		syncHubSingleton->queues[Queue::transfer] = transferQueue;

		syncHubSingleton->transferQueueIndex = transferQueueIndex;

		syncHubSingleton->commandPools[Queue::transfer] = transferCommandPool;
		syncHubSingleton->commandPools[Queue::graphics] = renderCommandPool;

		syncHubSingleton->bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		syncHubSingleton->transferSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		syncHubSingleton->CreateBuffers(renderCommandPool, computeCommandPool, transferCommandPool);

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

	void SyncHub::CreateBuffers(VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = renderCommandPool;
		allocInfo.commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		EWE_VK_ASSERT(vkAllocateCommandBuffers(device, &allocInfo, renderBuffers));
	}

	void SyncHub::CreateSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.pNext = nullptr;
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		EWE_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &singleTimeFenceGraphics));
	}

	VkCommandBuffer SyncHub::BeginSingleTimeCommand(Queue::Enum queue) {

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = commandPools[queue];

		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
		EWE_VK_ASSERT(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		EWE_VK_ASSERT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		return commandBuffer;

#if defined(_MSC_VER) && !defined(__clang__) // MSVC
		__assume(false);
#else // GCC, Clang
		__builtin_unreachable();
#endif
	}

	void SyncHub::EndSingleTimeCommandGraphics(VkCommandBuffer cmdBuf) {
		EWE_VK_ASSERT(vkEndCommandBuffer(cmdBuf));
		///prepTransferSubmission(cmdBuf);

		EWE_VK_ASSERT(vkResetFences(device, 1, &singleTimeFenceGraphics));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf;
		//std::cout << "before transfer submit \n";
		EWE_VK_ASSERT(vkQueueSubmit(queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics));
		//std::cout << "after transfer submit \n";

		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX));

		//this should be redundant with the vkWaitForFences
		//EWE_VK_ASSERT(vkQueueWaitIdle(graphicsQueue));

		vkFreeCommandBuffers(device, commandPools[Queue::graphics], 1, &cmdBuf);
	}

	void SyncHub::EndSingleTimeCommandGraphicsGroup(VkCommandBuffer cmdBuf) {
		EWE_VK_ASSERT(vkEndCommandBuffer(cmdBuf));
		graphicsSTCGroup.push_back(cmdBuf);
	}
	void SyncHub::RunGraphicsCallbacks() {
		syncPool.CheckFences();

		if (transitionManager.Empty()) {
			return;
		}
		TransitionData transitionData = transitionManager.Pull();
		transitionData.callback();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = graphicsSTCGroup.size();
		submitInfo.pCommandBuffers = graphicsSTCGroup.data();

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &transitionData.waitSemaphore->semaphore;
		submitInfo.signalSemaphoreCount = 1;

		SemaphoreData* signalSemaphore = syncPool.GetSemaphore();
		signalSemaphore->BeginSignaling();
		submitInfo.pSignalSemaphores = &signalSemaphore->semaphore;
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT };
		submitInfo.pWaitDstStageMask = waitStages;

		renderSyncData.AddWaitSemaphore(signalSemaphore);
		
		FenceData& fence = syncPool.GetFence();
		fence.signalSemaphores[Queue::graphics] = signalSemaphore;
		fence.waitSemaphores.push_back(transitionData.waitSemaphore);

		EWE_VK_ASSERT(vkQueueSubmit(queues[Queue::graphics], 1, &submitInfo, fence.fence));
		fence.callback = [this, cbs = graphicsSTCGroup] {
			vkFreeCommandBuffers(device, commandPools[Queue::graphics], static_cast<uint32_t>(cbs.size()), cbs.data());
		};
		graphicsSTCGroup.clear();
		//std::cout << "before transfer subm
	}

	void SyncHub::EndSingleTimeCommandGraphicsSignal(VkCommandBuffer cmdBuf, VkSemaphore signalSemaphore){
		EWE_VK_ASSERT(vkEndCommandBuffer(cmdBuf));
		EWE_VK_ASSERT(vkResetFences(device, 1, &singleTimeFenceGraphics));
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
		//std::cout << "before graphics EST signal submit \n";
		EWE_VK_ASSERT(vkQueueSubmit(queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics));
		//std::cout << "after graphics EST signal submit \n";

		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX));
		vkFreeCommandBuffers(device, commandPools[Queue::graphics], 1, &cmdBuf);
	}
	void SyncHub::EndSingleTimeCommandGraphicsWaitAndSignal(VkCommandBuffer cmdBuf, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore){
		EWE_VK_ASSERT(vkEndCommandBuffer(cmdBuf));
		EWE_VK_ASSERT(vkResetFences(device, 1, &singleTimeFenceGraphics));
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
		EWE_VK_ASSERT(vkQueueSubmit(queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics));
		//std::cout << "after graphics EST signal submit \n";

		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX));
		vkFreeCommandBuffers(device, commandPools[Queue::graphics], 1, &cmdBuf);
	}

	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf){
		EWE_VK_ASSERT(vkEndCommandBuffer(cmdBuf));
		TransferCommandManager::AddCommand(cmdBuf);

		AttemptTransferSubmission();
	}
	void SyncHub::EndSingleTimeCommandTransfer(CommandWithCallback cmdCb) {
		EWE_VK_ASSERT(vkEndCommandBuffer(cmdCb.cmdBuf));
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
		int debugLoopTracker = 0;
		auto cmdCbs = TransferCommandManager::PrepareSubmit();
		while (cmdCbs.commands.size() > 0) {
			printf("looping in submit transfer buffers : %d\n", debugLoopTracker++);
			transferSubmitInfo.commandBufferCount = cmdCbs.commands.size();
			transferSubmitInfo.pCommandBuffers = cmdCbs.commands.data();

			FenceData& fenceData = syncPool.GetFence();
			fenceData.callback = cmdCbs.CombineCallbacks(device, commandPools[Queue::transfer]);
			std::function<void()> graphicsCallbacks = nullptr;

			if (cmdCbs.CleanGraphicsCallbacks()) {
				fenceData.signalSemaphores[Queue::graphics] = syncPool.GetSemaphore();
				fenceData.signalSemaphores[Queue::graphics]->BeginSignaling();
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
			EWE_VK_ASSERT(vkQueueSubmit(queues[Queue::transfer], 1, &transferSubmitInfo, fenceData.fence));
			transferPoolMutex.unlock();
			//printf("after transfer submit\n");

			if (graphicsCallbacks != nullptr) {
				transitionManager.Add(graphicsCallbacks, fenceData.signalSemaphores[Queue::graphics]);
				printf("signal semaphore address : %zu\n", fenceData.signalSemaphores[Queue::graphics]);
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
			EWE_VK_ASSERT(vkWaitForFences(device, 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX));
		}
		imagesInFlight[*imageIndex] = renderSyncData.inFlight[frameIndex];

		std::vector<VkSemaphore> waitSemaphores = renderSyncData.GetWaitData(frameIndex);
		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();

		std::vector<VkSemaphore> signalSemaphores = renderSyncData.GetSignalData(frameIndex);
		submitInfo.pSignalSemaphores = signalSemaphores.data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());

		EWE_VK_ASSERT(vkResetFences(device, 1, &renderSyncData.inFlight[frameIndex]));

		EWE_VK_ASSERT(vkQueueSubmit(queues[Queue::graphics], 1, &submitInfo, renderSyncData.inFlight[frameIndex]));

		//std::cout << "immediately after submitting graphics \n";
	}

	VkResult SyncHub::PresentKHR(VkPresentInfoKHR& presentInfo, uint8_t currentFrame) {

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderSyncData.renderFinished[currentFrame];
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