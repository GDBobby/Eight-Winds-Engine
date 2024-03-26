#include "EWEngine/Graphics/Renderer.h"

#include <array>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace EWE {

	EWERenderer* EWERenderer::instance{ nullptr };

	EWERenderer::EWERenderer(MainWindow& window, EWECamera& camera) : camera{ camera }, mainWindow{ window }, eweDevice{ device }, syncHub{ SyncHub::getSyncHubInstance() } {
		instance = this;
		//printf("EWE renderer constructor \n");
#if GPU_LOGGING
		{
			std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
			logFile << "before creating swap chain" << std::endl;
			logFile.close();
		}
#endif
		recreateSwapChain();
#if GPU_LOGGING
		{
			std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
			logFile << "after creating swap chain" << std::endl;
			logFile.close();
		}
#endif

	}

	EWERenderer::~EWERenderer() {
		instance = nullptr;
#if DECONSTRUCTION_DEBUG
		printf("beginning EWErender deconstruction \n");
#endif
		
#if DECONSTRUCTION_DEBUG
		printf("end EWErender deconstruciton \n");
#endif
	}

	void EWERenderer::recreateSwapChain() {

		std::cout << "recreating swap chain" << std::endl;
		needToReadjust = true;
		auto extent = mainWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = mainWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(eweDevice.device());


		if (eweSwapChain == nullptr) {
			eweSwapChain = std::make_unique<EWESwapChain>(eweDevice, extent, true);
		}
		else {
			std::shared_ptr<EWESwapChain> oldSwapChain = std::move(eweSwapChain);
			eweSwapChain = std::make_unique<EWESwapChain>(eweDevice, extent, true, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*eweSwapChain.get())) {
				std::cout << "Swap chain image(or depth) format has changed!" << std::endl;
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}


		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(eweSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(eweSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		scissor = { {0, 0}, eweSwapChain->getSwapChainExtent() };
	}

	FrameInfo EWERenderer::beginFrame() {
#if _DEBUG
		if (isFrameStarted) {
			std::cout << "frame was started, finna throw an error " << std::endl;
		}
#endif
		assert(!isFrameStarted && "cannot call begin frame while frame is in progress!");

		//std::cout << "begin frame 1" << std::endl;

		auto result = eweSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "out of date KHR " << std::endl;
			recreateSwapChain();
			return { VK_NULL_HANDLE, currentFrameIndex };
		}
		//std::cout << "begin frame 2" << std::endl;

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			std::cout << "failed to acquire next swap chain image " << std::endl;
			printf("failed to acquire next swap chain image \n");
			throw std::runtime_error("failed to acquire next swap chain image");
		}
		//std::cout << "begin frame 3" << std::endl;

		isFrameStarted = true;
		auto commandBuffer = syncHub->getRenderBuffer(currentFrameIndex);
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		//std::cout << "begin frame 4" << std::endl;
		

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			std::cout << "failed to begin recording command buffer " << std::endl;
			//printf("failed to begin recording command buffer \n");
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return { commandBuffer, currentFrameIndex };
	}
	bool EWERenderer::endFrame() {
		bool restartedSwap = false;
		//printf("end frame :: isFrameStarted : %d \n", isFrameStarted);
		auto commandBuffer = syncHub->getRenderBuffer(currentFrameIndex);

		VkResult vkResult = vkEndCommandBuffer(commandBuffer);
		if (vkResult != VK_SUCCESS) {
			std::cout << "failed to record command buffer: " << vkResult << std::endl;
			throw std::runtime_error("failed to record command buffer");
		}
		//printf("after end command buffer \n");
		vkResult = eweSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR || mainWindow.wasWindowResized()) {
			mainWindow.resetWindowResizedFlag();
			restartedSwap = true;
			recreateSwapChain();
			camera.setPerspectiveProjection(glm::radians(70.0f), eweSwapChain->extentAspectRatio(), 0.1f, 10000.0f);
		}
		else if (vkResult != VK_SUCCESS) {
			std::cout << "failed to present swap chain image: " << vkResult << std::endl;
			throw std::runtime_error("failed to present swap chain image!");
		}
		//printf("after submitting command buffer \n");

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

		return restartedSwap;
	}
	/**/
	void EWERenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress!");
		assert(commandBuffer == getCurrentCommandBuffer() && "can't begin render pass on command buffer from different frame");
		/*
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = eweSwapChain->getRenderPass();
		renderPassInfo.framebuffer = eweSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = eweSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.f, 0.f, 0.f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		*/
		VkImageMemoryBarrier image_memory_barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.image = eweSwapChain->getImage(currentImageIndex),
			.subresourceRange = {
			  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			  .baseMipLevel = 0,
			  .levelCount = 1,
			  .baseArrayLayer = 0,
			  .layerCount = 1,
			}
		};

		if (eweDevice.getGraphicsIndex() != eweDevice.getPresentIndex()) {
			image_memory_barrier.srcQueueFamilyIndex = eweDevice.getPresentIndex();
			image_memory_barrier.dstQueueFamilyIndex = eweDevice.getGraphicsIndex();
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // srcStageMask
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
			0,
			0,
			nullptr,
			0,
			nullptr,
			1, // imageMemoryBarrierCount
			&image_memory_barrier // pImageMemoryBarriers
		);
		eweSwapChain->beginRender(commandBuffer, currentImageIndex);
	}
	void EWERenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress!");
		assert(commandBuffer == getCurrentCommandBuffer() && "can't end render pass on command buffer from different frame");

		//vkCmdEndRenderPass(commandBuffer);

		//std::cout << "before vkCmdEndRendering : " << std::endl;
		vkCmdEndRendering(commandBuffer);
		//std::cout << "after vkCmdEndRendering : " << std::endl;
		//vkCmdEndRenderingKHR

		VkImageMemoryBarrier image_memory_barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.image = eweSwapChain->getImage(currentImageIndex),
			.subresourceRange = {
			  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			  .baseMipLevel = 0,
			  .levelCount = 1,
			  .baseArrayLayer = 0,
			  .layerCount = 1,
			}
		};
		if (eweDevice.getGraphicsIndex() != eweDevice.getPresentIndex()) {
			image_memory_barrier.srcQueueFamilyIndex = eweDevice.getGraphicsIndex();
			image_memory_barrier.dstQueueFamilyIndex = eweDevice.getPresentIndex();
		}
		//printf("end frame :: get currentCommandBuffer \n");

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // dstStageMask
			0,
			0,
			nullptr,
			0,
			nullptr,
			1, // imageMemoryBarrierCount
			&image_memory_barrier // pImageMemoryBarriers
		);
	}





}