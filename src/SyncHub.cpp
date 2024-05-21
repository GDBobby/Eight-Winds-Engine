#include "EWEngine/Systems/SyncHub.h"
#include "EWEngine/Global_Macros.h"

#include <future>
#include <assert.h>

namespace EWE {
	SyncHub* SyncHub::syncHubSingleton{ nullptr };

	SyncHub::SyncHub() : transitionManager{ 2, 2 } {
#ifdef _DEBUG
		std::cout << "COSTRUCTING SYNCHUB" << std::endl;
#endif
	}
	SyncHub::~SyncHub() {
#ifdef _DEBUG
		std::cout << "DE COSTRUCTING SYNCHUB" << std::endl;
#endif
	}

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

		syncHubSingleton->transitionManager.InitializeSemaphores(device);

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
		EWE_VK_ASSERT(vkAllocateCommandBuffers(device, &allocInfo, renderBuffers.data()));

	}
	void SyncHub::WaitOnTransferFence() {
		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceTransfer, VK_TRUE, UINT64_MAX));
	}
	void SyncHub::CreateSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device, &fenceInfo, nullptr, &singleTimeFenceTransfer);
		

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			EWE_VK_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
			EWE_VK_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
			EWE_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]));
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
	void SyncHub::EndSingleTimeCommandGraphicsSignal(VkCommandBuffer cmdBuf, VkSemaphore signalSemaphore){
		vkEndCommandBuffer(cmdBuf);
		vkResetFences(device, 1, &singleTimeFenceGraphics);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
		//std::cout << "before graphics EST signal submit \n";
		EWE_VK_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, singleTimeFenceGraphics));
		//std::cout << "after graphics EST signal submit \n";

		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX));
		vkFreeCommandBuffers(device, graphicsCommandPool, 1, &cmdBuf);
	}
	void SyncHub::EndSingleTimeCommandGraphicsWaitAndSignal(VkCommandBuffer cmdBuf, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore){
		vkEndCommandBuffer(cmdBuf);
		vkResetFences(device, 1, &singleTimeFenceGraphics);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		//std::cout << "before graphics EST signal submit \n";
		EWE_VK_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, singleTimeFenceGraphics));
		//std::cout << "after graphics EST signal submit \n";

		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceGraphics, VK_TRUE, UINT64_MAX));
		vkFreeCommandBuffers(device, graphicsCommandPool, 1, &cmdBuf);
	}

	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, BufferQueueTransitionData const& bufferData){
		vkEndCommandBuffer(cmdBuf);		
		transferBuffers[transferFlipFlop].push_back(cmdBuf);
		transitionManager.GetStagingBuffer()->buffers.push_back(bufferData);
		AttemptTransferSubmission();
	}
	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, std::vector<BufferQueueTransitionData> const& bufferData){
		vkEndCommandBuffer(cmdBuf);
		transferBuffers[transferFlipFlop].push_back(cmdBuf);

		auto* stagingBuffer = transitionManager.GetStagingBuffer();
    	std::copy(bufferData.begin(), bufferData.end(), std::back_inserter(stagingBuffer->buffers));

		AttemptTransferSubmission();
	}
	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, ImageQueueTransitionData const& imageData) {
		vkEndCommandBuffer(cmdBuf);
		
		transferBuffers[transferFlipFlop].push_back(cmdBuf);
		transitionManager.GetStagingBuffer()->images.push_back(imageData);

		AttemptTransferSubmission();
	}
	void SyncHub::EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, std::vector<ImageQueueTransitionData> const& imageData){
		vkEndCommandBuffer(cmdBuf);

		transferBuffers[transferFlipFlop].push_back(cmdBuf);
		auto* stagingBuffer = transitionManager.GetStagingBuffer();

    	std::copy(imageData.begin(), imageData.end(), std::back_inserter(stagingBuffer->images));
		AttemptTransferSubmission();
	}
	void SyncHub::AttemptTransferSubmission(){
		if (readyForNextTransmit) {
			QueueTransitionContainer* transitionContainer = transitionManager.PrepareSubmission();
			
			//i dont remember why im null checking signalSemaphore
			if(transitionContainer != nullptr){

				readyForNextTransmit = false;
				transferFlipFlop = !transferFlipFlop;
				auto future = std::async(&SyncHub::SubmitTransferBuffers, this, transitionContainer);
			}
		}
	}

	void SyncHub::SubmitTransferBuffers(QueueTransitionContainer* transitionContainer) {

		//printf("begin submit transfer \n");
#ifdef _DEBUG
		assert(transferBuffers[!transferFlipFlop].size() > 0);
#endif
		vkResetFences(device, 1, &singleTimeFenceTransfer);

		//std::cout << "submitting transfer buffers : " << transferBuffers[!transferFlipFlop].size() << std::endl;
		transferSubmitInfo.commandBufferCount = static_cast<uint32_t>(transferBuffers[!transferFlipFlop].size());
		transferSubmitInfo.pCommandBuffers = transferBuffers[!transferFlipFlop].data();
		transferSubmitInfo.pSignalSemaphores = &transitionContainer->semaphore;
		transferSubmitInfo.signalSemaphoreCount = 1;
		//std::cout << "before transfer submit \n";
		EWE_VK_ASSERT(vkQueueSubmit(transferQueue, 1, &transferSubmitInfo, singleTimeFenceTransfer));
		//std::cout << "after transfer submit \n";

		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFenceTransfer, VK_TRUE, UINT64_MAX));
		transitionContainer->DestroyImageStagingBuffers(device);


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

		//std::cout << "immediately after submitting graphics \n";
	}	
	void SyncHub::SubmitGraphics(VkSubmitInfo& submitInfo, uint8_t frameIndex, uint32_t* imageIndex, VkSemaphore waitSemaphore) {

		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(device, 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[*imageIndex] = inFlightFences[frameIndex];
		std::vector<VkSemaphore> waitSemaphores{ graphicsWait[frameIndex] };
		waitSemaphores.push_back(waitSemaphore);

		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		submitInfo.pWaitSemaphores = waitSemaphores.data();

		submitInfo.pSignalSemaphores = graphicsSignal[frameIndex].data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(graphicsSignal[frameIndex].size());

		EWE_VK_ASSERT(vkResetFences(device, 1, &inFlightFences[frameIndex]));

		EWE_VK_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[frameIndex]));

		//std::cout << "immediately after submitting graphics \n";
	}
	VkResult SyncHub::PresentKHR(VkPresentInfoKHR& presentInfo, uint8_t currentFrame) {

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
		//std::cout << "imeddiately before presenting \n";
		return vkQueuePresentKHR(presentQueue, &presentInfo);
		//std::cout << "imediately after presenting \n";
	}
	void SyncHub::WaitOnGraphicsFence(const uint8_t frameIndex) {
		vkWaitForFences(device, 1, &inFlightFences[frameIndex], VK_TRUE, UINT64_MAX);
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