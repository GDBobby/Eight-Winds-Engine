#include "EWEngine/Systems/SyncHub.h"
#include "EWEngine/Global_Macros.h"

#include <future>
#include <assert.h>

namespace EWE {
	SyncHub* SyncHub::syncHubSingleton{ nullptr };

	void SyncHub::initialize(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkQueue computeQueue, VkQueue transferQueue, VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool) {
		this->graphicsQueue = graphicsQueue;
		this->presentQueue = presentQueue;
		this->computeQueue = computeQueue;
		this->transferQueue = transferQueue;

		this->device = device;

		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		transferSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		computeSubmitInfo.pNext = nullptr;
		computeSubmitInfo.commandBufferCount = 1;
		computeSubmitInfo.pCommandBuffers = &computeBuffer;
		/* swapping to a SIGNLE TIME compute dispatch method
		computeSubmitInfo.waitSemaphoreCount = 1;
		computeSubmitInfo.pWaitSemaphores = &graphicsSemaphore;

		computeSubmitInfo.signalSemaphoreCount = 1;
		computeSubmitInfo.pSignalSemaphores = &computeSemaphore;
		*/
		computeSubmitInfo.waitSemaphoreCount = 0;
		computeSubmitInfo.pWaitSemaphores = nullptr;
		computeSubmitInfo.signalSemaphoreCount = 0;
		computeSubmitInfo.pSignalSemaphores = nullptr;

		computeSubmitInfo.pWaitDstStageMask = &computeWaitStageMask;

		initOceanSubmitInfo();

		createBuffers(renderCommandPool, computeCommandPool, transferCommandPool);

		setMaxFramesInFlight();

		createSyncObjects();

		initEvents();
		initWaitMask();
		initSignalMask();

	}
	void SyncHub::destroy(VkCommandPool renderPool, VkCommandPool computePool, VkCommandPool transferPool) {
#if DECONSTRUCTION_DEBUG
		printf("beginniing synchub destroy \n");
#endif
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		vkDestroySemaphore(device, computeSemaphore, nullptr);
		vkDestroySemaphore(device, graphicsSemaphore, nullptr);

		vkDestroySemaphore(device, computeToGraphicsTransferSemaphore, nullptr);
		vkDestroySemaphore(device, graphicsToComputeTransferSemaphore, nullptr);
		if (singleTimeFence != VK_NULL_HANDLE) {
			vkDestroyFence(device, singleTimeFence, nullptr);
		}
		if (oceanFlightFence != VK_NULL_HANDLE) {
			vkDestroyFence(device, oceanFlightFence, nullptr);
		}

		vkDestroyFence(device, computeInFlightFence, nullptr);
		if (renderBuffers.size() > 0) {
			vkFreeCommandBuffers(device, renderPool, static_cast<uint32_t>(renderBuffers.size()), renderBuffers.data());
		}
		vkFreeCommandBuffers(device, computePool, 1, &oceanTransferBuffers[0]);
		vkFreeCommandBuffers(device, renderPool, 1, &oceanTransferBuffers[1]);


		vkFreeCommandBuffers(device, computePool, 1, &computeBuffer);
		vkFreeCommandBuffers(device, computePool, 5, &oceanBuffers[0]);		
		for (uint8_t i = 0; i < 5; i++) {
			vkDestroySemaphore(device, oceanSemaphores[i], nullptr);
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

	void SyncHub::setMaxFramesInFlight() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	}
	void SyncHub::createBuffers(VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool) {
		this->transferCommandPool = transferCommandPool;
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
		if (vkAllocateCommandBuffers(device, &allocInfo, &oceanTransferBuffers[1]) != VK_SUCCESS) {
			std::cout << "failed to allocate command buffers " << std::endl;
			throw std::runtime_error("failed to allocate command buffers!");
		}

		allocInfo.commandPool = computeCommandPool;
		allocInfo.commandBufferCount = 1;
		if (vkAllocateCommandBuffers(device, &allocInfo, &computeBuffer) != VK_SUCCESS) {
			std::cout << "failed to allocate command buffers " << std::endl;
			throw std::runtime_error("failed to allocate command buffers!");
		}
		if (vkAllocateCommandBuffers(device, &allocInfo, &oceanTransferBuffers[0]) != VK_SUCCESS) {
			std::cout << "failed to allocate command buffers " << std::endl;
			throw std::runtime_error("failed to allocate command buffers!");
		}

		allocInfo.commandBufferCount = 5;
		if (vkAllocateCommandBuffers(device, &allocInfo, &oceanBuffers[0]) != VK_SUCCESS) {
			std::cout << "failed to allocate command buffers " << std::endl;
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}
	void SyncHub::waitOnTransferFence() {
		VkResult vkResult = vkWaitForFences(device, 1, &singleTimeFence, VK_TRUE, UINT64_MAX);
		if (vkResult != VK_SUCCESS) {
			printf("failed to wait for fences : %d \n", vkResult);
			throw std::runtime_error("Failed to wait for fence in endSingleTimeCommands");
		}
	}
	void SyncHub::createSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device, &fenceInfo, nullptr, &singleTimeFence);
		vkCreateFence(device, &fenceInfo, nullptr, &oceanFlightFence);
		for (uint8_t i = 0; i < 5; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &oceanSemaphores[i]) != VK_SUCCESS) {
				std::cout << "OCEAN SEMAPHORE CREATE FAILURE \n";
				throw std::runtime_error("ocean create semaphore fialrue \n");
			}
		}

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeToGraphicsTransferSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &graphicsToComputeTransferSemaphore) != VK_SUCCESS
			) {
			std::cout << "failed to create compute graphics transfer fence \n";
			throw std::runtime_error("fence creation failure \n");
		}
		

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				std::cout << "failed to create rendering semaphores" << std::endl;
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
			//printf("in flight fences handle id : %llu \n", reinterpret_cast<uint64_t>(inFlightFences[i]));
		}
		if(vkCreateFence(device, &fenceInfo, nullptr, &computeInFlightFence) != VK_SUCCESS){
			std::cout << "failed to create compute fence" << std::endl;
			throw std::runtime_error("failed to create compute fence!");
		}
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &graphicsSemaphore) != VK_SUCCESS) {
			std::cout << "failed to create graphics semaphore \n" << std::endl;
			throw std::runtime_error("failed to create graphics semaphore");
		}

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeSemaphore) != VK_SUCCESS) {
			std::cout << "failed to create compute semaphore \n" << std::endl;
			throw std::runtime_error("failed to create compute semaphore");
		}
	}

	std::array<VkCommandBuffer, 5> SyncHub::beginOceanBuffers() {
		VkResult vkResult;

		vkResult = vkWaitForFences(device, 1, &oceanFlightFence, VK_TRUE, UINT64_MAX);
		if (vkResult != VK_SUCCESS) {
			printf("failed to wait for fences : %d \n", vkResult);
			throw std::runtime_error("Failed to wait for compute fence");
		}
		vkResetFences(device, 1, &oceanFlightFence);

		for (uint8_t i = 0; i < 5; i++) {
			//std::cout << "before beginning command buffer " << +i << std::endl;
			vkResult = vkBeginCommandBuffer(oceanBuffers[i], &bufferBeginInfo);
			if (vkResult != VK_SUCCESS) {
				//std::cout << "failed to begin compute command buffer " << std::endl;
				throw std::runtime_error("failed to begin compute command buffer!");
			}
			//std::cout << "after beginning command buffer " << +i << std::endl;
			/*
			if (i > 0) {
				vkCmdWaitEvents(oceanBuffers[i],
					1, &oceanWaitEvents[i - 1],
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0, nullptr, 0, nullptr, 0, nullptr);
			}
			*/
		}

		return oceanBuffers;
	}
	void SyncHub::endOceanBuffers() {
		for (uint8_t i = 0; i < 5; i++) {
			//vkCmdSetEvent(oceanBuffers[i], oceanWaitEvents[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			//std::cout << "before ending command buffer " << +i << std::endl;
			vkEndCommandBuffer(oceanBuffers[i]);
			//std::cout << "after ending command buffer " << +i << std::endl;
		}
		//std::cout << "before ending command buffer 4" << std::endl;
		//vkEndCommandBuffer(oceanBuffers[4]);
		//std::cout << "after ending command buffer 4" << std::endl;
		oceanComputing = true;
	}
	void SyncHub::OceanSubmission() {
		if (graphicsSemaphoreIndex < MAX_FRAMES_IN_FLIGHT) {
			oceanSubmitInfo[0].pWaitSemaphores = nullptr;
			oceanSubmitInfo[0].waitSemaphoreCount = 0;
		}
		else {
			oceanSubmitInfo[0].pWaitSemaphores = &graphicsSemaphore;
			oceanSubmitInfo[0].waitSemaphoreCount = 1;
		}
		for (uint8_t i = 0; i < 4; i++) {
			EWE_VK_ASSERT(vkQueueSubmit(computeQueue, 1, &oceanSubmitInfo[i], nullptr));
		}
		EWE_VK_ASSERT(vkQueueSubmit(computeQueue, 1, &oceanSubmitInfo[4], oceanFlightFence));
	}
	VkCommandBuffer SyncHub::beginComputeBuffer() {
		VkResult vkResult = vkWaitForFences(device, 1, &computeInFlightFence, VK_TRUE, UINT64_MAX);
		if (vkResult != VK_SUCCESS) {
			printf("failed to wait for fences : %d \n", vkResult);
			throw std::runtime_error("Failed to wait for compute fence");
		}
		vkResetFences(device, 1, &computeInFlightFence);

		if (vkBeginCommandBuffer(computeBuffer, &bufferBeginInfo) != VK_SUCCESS) {
			std::cout << "failed to begin compute command buffer " << std::endl;
			throw std::runtime_error("failed to begin compute command buffer!");
		}
		return computeBuffer;
	}
	void SyncHub::endComputeBuffer() {
		vkEndCommandBuffer(computeBuffer);
		submitCompute();
		//computing = true;
	}
	VkCommandBuffer SyncHub::BeginSingleTimeCommand(VkCommandPool cmdPool){
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = cmdPool;

		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	VkCommandBuffer SyncHub::BeginSingleTimeCommands() {
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
	void SyncHub::prepTransferSubmission(VkCommandBuffer transferBuffer) {

		//std::cout << "pushing for transfer submission \n";
		transferBuffers[transferFlipFlop].push_back(transferBuffer);

		if (readyForNextTransmit) {
			readyForNextTransmit = false;
			transferFlipFlop = !transferFlipFlop;
			auto future = std::async(&SyncHub::submitTransferBuffers, this);
		}

	}
	void SyncHub::submitTransferBuffers() {

		//printf("begin submit transfer \n");
#ifdef _DEBUG
		assert(transferBuffers[!transferFlipFlop].size() > 0);
#endif
		vkResetFences(device, 1, &singleTimeFence);

		//std::cout << "submitting transfer buffers : " << transferBuffers[!transferFlipFlop].size() << std::endl;
		transferSubmitInfo.commandBufferCount = static_cast<uint32_t>(transferBuffers[!transferFlipFlop].size());
		transferSubmitInfo.pCommandBuffers = transferBuffers[!transferFlipFlop].data();
		//std::cout << "before transfer submit \n";
		EWE_VK_ASSERT(vkQueueSubmit(transferQueue, 1, &transferSubmitInfo, singleTimeFence));
		//std::cout << "after transfer submit \n";

		EWE_VK_ASSERT(vkWaitForFences(device, 1, &singleTimeFence, VK_TRUE, UINT64_MAX));

		vkResetFences(device, 1, &singleTimeFence);

		EWE_VK_ASSERT(vkQueueWaitIdle(transferQueue));

		{
			std::lock_guard<std::mutex> lock(transferPoolMutex);
			vkFreeCommandBuffers(device, transferCommandPool, static_cast<uint32_t>(transferBuffers[!transferFlipFlop].size()), transferBuffers[!transferFlipFlop].data());
		}
		//std::cout << "after successful submission \n";
		transferBuffers[!transferFlipFlop].clear();
		readyForNextTransmit = true;


		//need to sync between threads before clearing

	}
	void SyncHub::submitCompute() {
		/* this fixes the INITIAL submission but im swapping to a ONE TIME compute dispatch method
		if (graphicsSemaphoreIndex < MAX_FRAMES_IN_FLIGHT) {
			computeSubmitInfo.pWaitSemaphores = nullptr;
			computeSubmitInfo.waitSemaphoreCount = 0;
		}
		else {
			computeSubmitInfo.pWaitSemaphores = &graphicsSemaphore;
			computeSubmitInfo.waitSemaphoreCount = 1;
		}
		*/

		EWE_VK_ASSERT(vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, computeInFlightFence));
	}

	void SyncHub::submitGraphics(VkSubmitInfo& submitInfo, uint8_t frameIndex, uint32_t* imageIndex) {
		/* swapping to a single time compute dispatch method
		if (computing) {
			submitCompute();
			computing = false;

			graphicsSemaphoreIndex = frameIndex + MAX_FRAMES_IN_FLIGHT;
			std::cout << "currently computing : " << computing << std::endl;
		}
		else 
		*/
		if (oceanComputing) {
			//std::cout << "OCEAN SUBMISSION \n";
			OceanSubmission();
			graphicsSemaphoreIndex = frameIndex + MAX_FRAMES_IN_FLIGHT;
			//std::cout << "AFTER OCEAN SUBMISISON \n";
			//std::cout << "currently ocean computing : " << oceanComputing << std::endl;
			/*
			oceanTransfersSubmitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			oceanTransfersSubmitInfo[0].pNext = nullptr;
			oceanTransfersSubmitInfo[0].commandBufferCount = 1;
			oceanTransfersSubmitInfo[0].pCommandBuffers = &oceanTransferBuffers[0];
			oceanTransfersSubmitInfo[0].waitSemaphoreCount = 1;
			oceanTransfersSubmitInfo[0].pWaitSemaphores = &oceanSemaphores[4];
			oceanTransfersSubmitInfo[0].signalSemaphoreCount = 1;
			oceanTransfersSubmitInfo[0].pSignalSemaphores = &computeToGraphicsTransferSemaphore;
			oceanTransfersSubmitInfo[0].pWaitDstStageMask = &computeWaitStageMask;
			std::cout << "before OTG 0 submit :" << oceanTransfersSubmitInfo[0].sType << std::endl;
			if (oceanTransfersSubmitInfo[0].sType != VK_STRUCTURE_TYPE_SUBMIT_INFO) {
				std::cout << "incorrect submit info??? \n";
				throw std::exception("WHY?");
			}
			vkQueueSubmit(computeQueue, 1, &oceanTransfersSubmitInfo[0], nullptr);
			std::cout << "after OTG 0 submit :" << oceanTransfersSubmitInfo[0].sType << std::endl;
			*/
		}
		else {
			graphicsSemaphoreIndex = frameIndex;
		}

		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(device, 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[*imageIndex] = inFlightFences[frameIndex];

		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(graphicsWait[graphicsSemaphoreIndex].size());
		submitInfo.pWaitSemaphores = graphicsWait[graphicsSemaphoreIndex].data();

		submitInfo.pSignalSemaphores = graphicsSignal[graphicsSemaphoreIndex].data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(graphicsSignal[graphicsSemaphoreIndex].size());

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
	VkResult SyncHub::presentKHR(VkPresentInfoKHR& presentInfo, uint8_t currentFrame) {

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
		//std::cout << "imeddiately before presenting \n";
		VkResult ret = vkQueuePresentKHR(presentQueue, &presentInfo);
		//std::cout << "imediately after presenting \n";
		return ret;
	}

	void SyncHub::initOceanSubmitInfo() {
		oceanSubmitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		oceanSubmitInfo[0].pNext = nullptr;
		oceanSubmitInfo[0].commandBufferCount = 1;
		oceanSubmitInfo[0].pCommandBuffers = &oceanBuffers[0];
		oceanSubmitInfo[0].waitSemaphoreCount = 1; //testing
		oceanSubmitInfo[0].pWaitSemaphores = &graphicsToComputeTransferSemaphore; //testing

		oceanSubmitInfo[0].signalSemaphoreCount = 1;
		oceanSubmitInfo[0].pSignalSemaphores = &oceanSemaphores[0];
		oceanSubmitInfo[0].pWaitDstStageMask = &computeWaitStageMask;


		for (uint8_t i = 1; i < 4; i++) {
			oceanSubmitInfo[i].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			oceanSubmitInfo[i].pNext = nullptr;
			oceanSubmitInfo[i].commandBufferCount = 1;
			oceanSubmitInfo[i].pCommandBuffers = &oceanBuffers[i];
			oceanSubmitInfo[i].waitSemaphoreCount = 1;
			oceanSubmitInfo[i].pWaitSemaphores = &oceanSemaphores[i - 1];

			oceanSubmitInfo[i].signalSemaphoreCount = 1;
			oceanSubmitInfo[i].pSignalSemaphores = &oceanSemaphores[i];

			oceanSubmitInfo[i].pWaitDstStageMask = &computeWaitStageMask;
		}

		oceanSubmitInfo[4].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		oceanSubmitInfo[4].pNext = nullptr;
		oceanSubmitInfo[4].commandBufferCount = 1;
		oceanSubmitInfo[4].pCommandBuffers = &oceanBuffers[4];
		oceanSubmitInfo[4].waitSemaphoreCount = 1;
		oceanSubmitInfo[4].pWaitSemaphores = &oceanSemaphores[3];

		oceanSubmitInfo[4].signalSemaphoreCount = 0;
		oceanSubmitInfo[4].pSignalSemaphores = nullptr;
		oceanSubmitInfo[4].signalSemaphoreCount = 1;
		oceanSubmitInfo[4].pSignalSemaphores = &oceanSemaphores[4];
		oceanSubmitInfo[4].pWaitDstStageMask = &computeWaitStageMask;


		std::cout << "initializing ocean transfer submit info \n";
		oceanTransfersSubmitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		oceanTransfersSubmitInfo[0].pNext = nullptr;
		oceanTransfersSubmitInfo[0].commandBufferCount = 1;
		oceanTransfersSubmitInfo[0].pCommandBuffers = &oceanTransferBuffers[0];
		oceanTransfersSubmitInfo[0].waitSemaphoreCount = 1;
		oceanTransfersSubmitInfo[0].pWaitSemaphores = &oceanSemaphores[4];
		oceanTransfersSubmitInfo[0].signalSemaphoreCount = 1;
		oceanTransfersSubmitInfo[0].pSignalSemaphores = &computeToGraphicsTransferSemaphore;
		oceanTransfersSubmitInfo[0].pWaitDstStageMask = &computeWaitStageMask;

		oceanTransfersSubmitInfo[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		oceanTransfersSubmitInfo[1].pNext = nullptr;
		oceanTransfersSubmitInfo[1].commandBufferCount = 1;
		oceanTransfersSubmitInfo[1].pCommandBuffers = &oceanTransferBuffers[1];
		oceanTransfersSubmitInfo[1].waitSemaphoreCount = 1;
		oceanTransfersSubmitInfo[1].pWaitSemaphores = &graphicsSemaphore;
		oceanTransfersSubmitInfo[1].signalSemaphoreCount = 1;
		oceanTransfersSubmitInfo[1].pSignalSemaphores = &graphicsToComputeTransferSemaphore;
		oceanTransfersSubmitInfo[1].pWaitDstStageMask = &graphicsToComputeWaitStageMask;
		std::cout << "after initializing ocean transfer submit info \n";

	}

	void SyncHub::initSignalMask() {
		graphicsSignal.resize(MAX_FRAMES_IN_FLIGHT * 2);

		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			graphicsSignal[i] = { renderFinishedSemaphores[i] };
			//graphicsSignal[i + MAX_FRAMES_IN_FLIGHT] = { renderFinishedSemaphores[i], graphicsToComputeTransferSemaphore };
			graphicsSignal[i + MAX_FRAMES_IN_FLIGHT] = { renderFinishedSemaphores[i], graphicsSemaphore };
		}

	}
	void SyncHub::initWaitMask() {
		graphicsWait.resize(MAX_FRAMES_IN_FLIGHT * 2);
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			graphicsWait[i] = { imageAvailableSemaphores[i] };
			//graphicsWait[i + MAX_FRAMES_IN_FLIGHT] = { computeToGraphicsTransferSemaphore, imageAvailableSemaphores[i] };
			graphicsWait[i + MAX_FRAMES_IN_FLIGHT] = { oceanSemaphores[4], imageAvailableSemaphores[i] };
		}
	}

	void SyncHub::domDemand() {

		{
			std::unique_lock<std::mutex> cuckLock(domCuckSync.cuckMutex);
			domCuckSync.cuckCondition.wait(cuckLock, [this]
				{ return !domCuckSync.cuckConditionHeld; }
			);
		}
		std::unique_lock<std::mutex> renderLock(domCuckSync.domMutex);
		domCuckSync.domConditionHeld = true;
	}
	void SyncHub::domRelease() {
		domCuckSync.domConditionHeld = false;
		domCuckSync.domCondition.notify_one();
	}
	void SyncHub::cuckRequest() {
		{
			domCuckSync.cuckConditionHeld = true;
			std::unique_lock<std::mutex> cuckLock(domCuckSync.cuckMutex);
			std::unique_lock<std::mutex> domLock(domCuckSync.domMutex);

			domCuckSync.domCondition.wait(domLock, [this]
				{ return !domCuckSync.domConditionHeld; }
			);
		}
	}
	void SyncHub::cuckSubmit() {
		domCuckSync.cuckConditionHeld = false;
		domCuckSync.cuckCondition.notify_one();
	}
}