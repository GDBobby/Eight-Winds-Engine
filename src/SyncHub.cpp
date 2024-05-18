#include "EWEngine/Systems/SyncHub.h"
#include "EWEngine/Global_Macros.h"

#include <future>
#include <assert.h>

namespace EWE {
	SyncHub* SyncHub::syncHubSingleton{ nullptr };

	void SyncHub::Initialize(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkQueue computeQueue, VkQueue transferQueue, VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool, uint32_t transferQueueIndex) {
		syncHubSingleton = new SyncHub();
		
		syncHubSingleton->graphicsQueue = graphicsQueue;
		syncHubSingleton->presentQueue = presentQueue;
		syncHubSingleton->computeQueue = computeQueue;
		syncHubSingleton->transferQueue = transferQueue;
		syncHubSingleton->transferQueueIndex = transferQueueIndex;

		syncHubSingleton->transferCommandPool = transferCommandPool;
		syncHubSingleton->graphicsCommandPool = renderCommandPool;

		syncHubSingleton->device = device;

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

		if (renderBuffers.size() > 0) {
			vkFreeCommandBuffers(device, renderPool, static_cast<uint32_t>(renderBuffers.size()), renderBuffers.data());
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

	void SyncHub::SetMaxFramesInFlight() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	}
	void SyncHub::CreateBuffers(VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool) {

		renderBuffers.clear();
		renderBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = renderCommandPool;
		allocInfo.commandBufferCount = static_cast<uint32_t>(renderBuffers.size());
		if (vkAllocateCommandBuffers(device, &allocInfo, renderBuffers.data()) != VK_SUCCESS) {
			std::cout << "failed to allocate command buffers " << std::endl;
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}
	void SyncHub::WaitOnTransferFence() {
		VkResult vkResult = vkWaitForFences(device, 1, &singleTimeFenceTransfer, VK_TRUE, UINT64_MAX);
		if (vkResult != VK_SUCCESS) {
			printf("failed to wait for fences : %d \n", vkResult);
			throw std::runtime_error("Failed to wait for fence in endSingleTimeCommands");
		}
	}
	void SyncHub::CreateSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device, &fenceInfo, nullptr, &singleTimeFenceTransfer);
		

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				std::cout << "failed to create rendering semaphores" << std::endl;
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
			//printf("in flight fences handle id : %llu \n", reinterpret_cast<uint64_t>(inFlightFences[i]));
		}
	}

	VkCommandBuffer SyncHub::BeginSingleTimeCommandGraphics(){
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = graphicsCommandPool;

		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	VkCommandBuffer SyncHub::BeginSingleTimeCommandTransfer() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = transferCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
		{
			std::lock_guard<std::mutex> lock(transferPoolMutex);
			vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}
	//this might turn legacy
	void SyncHub::EndSingleTimeCommandGraphics(VkCommandBuffer cmdBuf) {
		vkEndCommandBuffer(cmdBuf);
		///prepTransferSubmission(cmdBuf);

		vkResetFences(device, 1, &singleTimeFenceGraphics);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf;
		//std::cout << "before transfer submit \n";
		EWE_VK_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, singleTimeFenceGraphics));
		//std::cout << "after transfer submit \n";

		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX));

		//this should be redundant with the vkWaitForFences
		//EWE_VK_ASSERT(vkQueueWaitIdle(graphicsQueue));

		vkFreeCommandBuffers(device, graphicsCommandPool, 1, &cmdBuf);
	}
	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, VkBuffer buffer, uint32_t dstQueue){
		vkEndCommandBuffer(cmdBuf);		
		transitionManager.GetStagingBuffer()->buffers.emplace_back(buffer, dstQueue);
		
		transferBuffers[transferFlipFlop].push_back(cmdBuf);
		if (readyForNextTransmit) {
			readyForNextTransmit = false;
			transferFlipFlop = !transferFlipFlop;
			auto future = std::async(&SyncHub::SubmitTransferBuffers, this);
		}
	}
	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, VkBuffer* buffers, uint8_t bufferCount, uint32_t dstQueue){
		vkEndCommandBuffer(cmdBuf);
		transferBuffers[transferFlipFlop].push_back(cmdBuf);

		auto* stagingBuffer = transitionManager.GetStagingBuffer();
		for(uint8_t i = 0; i < bufferCount; i++){
			stagingBuffer->buffers.emplace_back(buffers[i], dstQueue);
		}


		if (readyForNextTransmit) {
			readyForNextTransmit = false;
			transferFlipFlop = !transferFlipFlop;
			auto future = std::async(&SyncHub::SubmitTransferBuffers, this);
		}
		//for(uint8_t i = 0; i < bufferCount; i++){
		//}
	}
	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, VkImage image, bool generateMips, uint32_t dstQueue) {
		vkEndCommandBuffer(cmdBuf);
		
		transferBuffers[transferFlipFlop].push_back(cmdBuf);
		transitionManager.GetStagingBuffer()->images.emplace_back(image, generateMips, dstQueue);

		if (readyForNextTransmit) {
			readyForNextTransmit = false;
			transferFlipFlop = !transferFlipFlop;
			auto future = std::async(&SyncHub::SubmitTransferBuffers, this);
		}
	}
	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, VkImage* images, uint8_t imageCount, bool generateMips, uint32_t dstQueue){
		vkEndCommandBuffer(cmdBuf);

		transferBuffers[transferFlipFlop].push_back(cmdBuf);
		auto* stagingBuffer = transitionManager.GetStagingBuffer();
		for(uint8_t i = 0; i < imageCount; i++){
			stagingBuffer->images.emplace_back(images[i], generateMips, dstQueue);
		}

		if (readyForNextTransmit) {
			readyForNextTransmit = false;
			transferFlipFlop = !transferFlipFlop;
			auto future = std::async(&SyncHub::SubmitTransferBuffers, this);
		}
	}

	void SyncHub::SubmitTransferBuffers() {

		//printf("begin submit transfer \n");
#ifdef _DEBUG
		assert(transferBuffers[!transferFlipFlop].size() > 0);
#endif
		vkResetFences(device, 1, &singleTimeFenceTransfer);

		//std::cout << "submitting transfer buffers : " << transferBuffers[!transferFlipFlop].size() << std::endl;
		transferSubmitInfo.commandBufferCount = static_cast<uint32_t>(transferBuffers[!transferFlipFlop].size());
		transferSubmitInfo.pCommandBuffers = transferBuffers[!transferFlipFlop].data();
		//std::cout << "before transfer submit \n";
		EWE_VK_ASSERT(vkQueueSubmit(transferQueue, 1, &transferSubmitInfo, singleTimeFenceTransfer));
		//std::cout << "after transfer submit \n";

		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceTransfer, VK_TRUE, UINT64_MAX));

		//this should be redundant with the vkWaitForFences
		//EWE_VK_ASSERT(vkQueueWaitIdle(transferQueue));

		{
			std::lock_guard<std::mutex> lock(transferPoolMutex);
			vkFreeCommandBuffers(device, transferCommandPool, static_cast<uint32_t>(transferBuffers[!transferFlipFlop].size()), transferBuffers[!transferFlipFlop].data());
		}
		//std::cout << "after successful submission \n";
		transferBuffers[!transferFlipFlop].clear();
		readyForNextTransmit = true;


		//need to sync between threads before clearing

	}

	void SyncHub::SubmitGraphics(VkSubmitInfo& submitInfo, uint8_t frameIndex, uint32_t* imageIndex) {
		

		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(device, 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[*imageIndex] = inFlightFences[frameIndex];

		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(graphicsWait[frameIndex].size());
		submitInfo.pWaitSemaphores = graphicsWait[frameIndex].data();

		submitInfo.pSignalSemaphores = graphicsSignal[frameIndex].data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(graphicsSignal[frameIndex].size());

		EWE_VK_ASSERT(vkResetFences(device, 1, &inFlightFences[frameIndex]));

		EWE_VK_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[frameIndex]));

		/*
		if (oceanComputing) {
			oceanComputing = false;
			std::cout << "before GTO 1 submit \n";
			oceanTransfersSubmitInfo[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			oceanTransfersSubmitInfo[1].pNext = nullptr;
			oceanTransfersSubmitInfo[1].commandBufferCount = 1;
			oceanTransfersSubmitInfo[1].pCommandBuffers = &oceanTransferBuffers[1];
			oceanTransfersSubmitInfo[1].waitSemaphoreCount = 1;
			oceanTransfersSubmitInfo[1].pWaitSemaphores = &graphicsSemaphore;
			oceanTransfersSubmitInfo[1].signalSemaphoreCount = 1;
			oceanTransfersSubmitInfo[1].pSignalSemaphores = &graphicsToComputeTransferSemaphore;
			oceanTransfersSubmitInfo[1].pWaitDstStageMask = &graphicsToComputeWaitStageMask;
			vkQueueSubmit(computeQueue, 1, &oceanTransfersSubmitInfo[1], nullptr);
			std::cout << "before GTO 1 submit \n";
		}
		*/
		//std::cout << "immediately after submitting graphics \n";
	}
	VkResult SyncHub::PresentKHR(VkPresentInfoKHR& presentInfo, uint8_t currentFrame) {

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
		//std::cout << "imeddiately before presenting \n";
		VkResult ret = vkQueuePresentKHR(presentQueue, &presentInfo);
		//std::cout << "imediately after presenting \n";
		return ret;
	}
	void SyncHub::InitWaitMask() {
		graphicsWait.resize(MAX_FRAMES_IN_FLIGHT);
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			graphicsWait[i] = { imageAvailableSemaphores[i] };
		}
	}
	void SyncHub::InitSignalMask() {
		graphicsSignal.resize(MAX_FRAMES_IN_FLIGHT);

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			graphicsSignal[i] = { renderFinishedSemaphores[i] };
		}

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