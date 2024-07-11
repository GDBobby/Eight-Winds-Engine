#include "EWEngine/Systems/SyncHub.h"
#include "EWEngine/Global_Macros.h"

#include <future>
#include <cassert>

namespace EWE {
	SyncHub* SyncHub::syncHubSingleton{ nullptr };

	SyncHub::SyncHub(VkDevice device) : device{device}, syncPool{32, device} {
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

		syncHubSingleton->SetMaxFramesInFlight();

		syncHubSingleton->CreateSyncObjects();

		syncHubSingleton->InitWaitMask();
		syncHubSingleton->InitSignalMask();
	}
	void SyncHub::Destroy(VkCommandPool renderPool, VkCommandPool computePool, VkCommandPool transferPool) {
#if DECONSTRUCTION_DEBUG
		printf("beginniing synchub destroy \n");
#endif
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		if (singleTimeFenceTransfer != VK_NULL_HANDLE) {
			vkDestroyFence(device, singleTimeFenceTransfer, nullptr);
		}

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (renderBuffers[i] != VK_NULL_HANDLE) {
				vkFreeCommandBuffers(device, renderPool, 1, &renderBuffers[i]);
			}
		}

		for (int i = 0; i < transferBuffers.size(); i++) {
			if (transferBuffers[i].size() > 0) {
				vkFreeCommandBuffers(device, transferPool, static_cast<uint32_t>(transferBuffers[i].size()), transferBuffers[i].data());
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

	void SyncHub::WaitOnTransferFence() {
		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceTransfer, VK_TRUE, UINT64_MAX));
	}

	void SyncHub::CreateSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.pNext = nullptr;
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		EWE_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &singleTimeFenceTransfer));
		EWE_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &singleTimeFenceGraphics));
		

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			EWE_VK_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
			EWE_VK_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
			EWE_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]));
			//printf("in flight fences handle id : %llu \n", reinterpret_cast<uint64_t>(inFlightFences[i]));
		}
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

	void SyncHub::SubmitGraphicsSTCGroup() {
		EWE_VK_ASSERT(vkResetFences(device, 1, &singleTimeFenceGraphics));
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = graphicsSTCGroup.size();
		submitInfo.pCommandBuffers = graphicsSTCGroup.data();
		submitInfo.pWaitSemaphores = 2;
		submitInfo.waitSemaphoreCount = 2;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = 1;
		//std::cout << "before transfer submit \n";
		EWE_VK_ASSERT(vkQueueSubmit(queues[Queue::graphics], 1, &submitInfo, singleTimeFenceGraphics));
		//std::cout << "after transfer submit \n";

		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX));

		//this should be redundant with the vkWaitForFences
		//EWE_VK_ASSERT(vkQueueWaitIdle(graphicsQueue));

		for (auto const& cmdBuf : graphicsSTCGroup) {
			vkFreeCommandBuffers(device, commandPools[Queue::graphics], 1, &cmdBuf);
		}
		graphicsSTCGroup.clear();
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
		TransferCommandManager::AddCommand(cmdBuf);

		AttemptTransferSubmission();
	}
	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, std::function<void()> func) {
		TransferCommandManager::AddCommand(cmdBuf, func);
		
		AttemptTransferSubmission();
	}
	void SyncHub::EndSingleTimeCommandTransfer(SyncedCommandQueue* cmdQueue){
		TransferCommandManager::EndCommandQueue(cmdQueue);

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

		auto cmdCbs = TransferCommandManager::PrepareSubmit(transferSubmitInfo);
		FenceData& fenceData = syncPool.GetFenceSignal(Queue::transfer);
		fenceData.PrepareSubmitInfo(transferSubmitInfo);
		fenceData.callback = cmdCbs.CombineCallbacks(device);
		transitionManager.AddTransition(cmdCbs.CombineGraphicsCallbacks(), fenceData.signalSemaphore);
		
		//std::cout << "before transfer submit \n";
		transferPoolMutex.lock();
		EWE_VK_ASSERT(vkQueueSubmit(queues[Queue::transfer], 1, &transferSubmitInfo, fenceData.fence));
		transferPoolMutex.unlock();
		//std::cout << "after transfer submit \n";

		transferPoolMutex.lock();
		vkFreeCommandBuffers(device, commandPools[Queue::transfer], static_cast<uint32_t>(cmdCbs.commands.size()), cmdCbs.commands.data());
		transferPoolMutex.unlock();
	
		//std::cout << "after successful submission \n";
		readyForNextTransmit = true;
	}

	void SyncHub::SubmitGraphics(VkSubmitInfo& submitInfo, uint8_t frameIndex, uint32_t* imageIndex) {

		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) [[likely]] {
			EWE_VK_ASSERT(vkWaitForFences(device, 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX));
		}
		imagesInFlight[*imageIndex] = inFlightFences[frameIndex];

		std::vector<VkSemaphore> waitSemaphores = renderSyncData.GetWaitData(frameIndex);
		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();

		std::vector<VkSemaphore> signalSemaphores = renderSyncData.GetSignalData(frameIndex);
		submitInfo.pSignalSemaphores = signalSemaphores.data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());

		EWE_VK_ASSERT(vkResetFences(device, 1, &inFlightFences[frameIndex]));

		EWE_VK_ASSERT(vkQueueSubmit(queues[Queue::graphics], 1, &submitInfo, inFlightFences[frameIndex]));

		//std::cout << "immediately after submitting graphics \n";
	}

	VkResult SyncHub::PresentKHR(VkPresentInfoKHR& presentInfo, uint8_t currentFrame) {

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderSyncData.renderFinished[currentFrame];
		//std::cout << "imeddiately before presenting \n";
		return vkQueuePresentKHR(queues[Queue::present], &presentInfo);
		//std::cout << "imediately after presenting \n";
	}
	void SyncHub::WaitOnGraphicsFence(const uint8_t frameIndex) {
		EWE_VK_ASSERT(vkWaitForFences(device, 1, &inFlightFences[frameIndex], VK_TRUE, UINT64_MAX));
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