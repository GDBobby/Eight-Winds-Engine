#include "EWEngine/Graphics/Renderer.h"

#include <array>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace EWE {
	void EWERenderer::BindGraphicsPipeline(VkCommandBuffer commandBuffer, VkPipeline graphicsPipeline) {
#if _DEBUG
		if (instance == nullptr) {
			std::cout << "ewe renderer was nullptr \n";
			std::cout << "ewe renderer was nullptr \n";
			std::cout << "ewe renderer was nullptr \n";
		}
		assert(instance != nullptr);
#endif

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		vkCmdSetViewport(commandBuffer, 0, 1, &instance->viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &instance->scissor);
	}



	EWERenderer* EWERenderer::instance{ nullptr };

	EWERenderer::EWERenderer(MainWindow& window, EWECamera& camera) : camera{ camera }, mainWindow{ window }, syncHub{ SyncHub::GetSyncHubInstance() } {
		instance = this;
		//printf("EWE renderer constructor \n");
#if GPU_LOGGING
		{
			std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
			logFile << "before creating swap chain" << std::endl;
			logFile.close();
		}
#endif
		RecreateSwapChain();
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

	void EWERenderer::RecreateSwapChain() {

		std::cout << "recreating swap chain" << std::endl;
		needToReadjust = true;
		auto extent = mainWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = mainWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(EWEDevice::GetVkDevice());


		if (eweSwapChain == nullptr) {
			eweSwapChain = std::make_unique<EWESwapChain>(extent, true);
		}
		else {
			std::shared_ptr<EWESwapChain> oldSwapChain = std::move(eweSwapChain);
			eweSwapChain = std::make_unique<EWESwapChain>(extent, true, oldSwapChain);

			assert(oldSwapChain->CompareSwapFormats(*eweSwapChain.get()) && "Swap chain image(or depth) format has changed!");
		}


		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(eweSwapChain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(eweSwapChain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		scissor = { {0, 0}, eweSwapChain->GetSwapChainExtent() };
	}

	FrameInfo EWERenderer::BeginFrame() {
#if _DEBUG
		if (isFrameStarted) {
			std::cout << "frame was started, finna throw an error " << std::endl;
		}
#endif
		assert(!isFrameStarted && "cannot call begin frame while frame is in progress!");

		//std::cout << "begin frame 1" << std::endl;

		auto result = eweSwapChain->AcquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "out of date KHR " << std::endl;
			RecreateSwapChain();
			return { VK_NULL_HANDLE, currentFrameIndex };
		}
		//std::cout << "begin frame 2" << std::endl;

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			EWE_VK_RESULT_ASSERT(result);
		}
		//std::cout << "begin frame 3" << std::endl;

		isFrameStarted = true;
		auto commandBuffer = syncHub->GetRenderBuffer(currentFrameIndex);
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		//std::cout << "begin frame 4" << std::endl;
		

		EWE_VK_ASSERT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		return { commandBuffer, currentFrameIndex };
	}
	bool EWERenderer::EndFrame() {
		//printf("end frame :: isFrameStarted : %d \n", isFrameStarted);
		auto commandBuffer = syncHub->GetRenderBuffer(currentFrameIndex);

		EWE_VK_ASSERT(vkEndCommandBuffer(commandBuffer));
		//printf("after end command buffer \n");
		VkResult vkResult = eweSwapChain->SubmitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR || mainWindow.wasWindowResized()) {
			mainWindow.resetWindowResizedFlag();
			RecreateSwapChain();
			camera.SetPerspectiveProjection(glm::radians(70.0f), eweSwapChain->ExtentAspectRatio(), 0.1f, 10000.0f);
			isFrameStarted = false;
			currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
			return true;
		}
		else if (vkResult != VK_SUCCESS) {
			std::cout << "failed to present swap chain image: " << vkResult << std::endl;
			assert(false && "failed to present swap chain image");
		}
		//printf("after submitting command buffer \n");

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

		return false;
	}

	bool EWERenderer::EndFrameAndWaitForFence() {
		auto commandBuffer = syncHub->GetRenderBuffer(currentFrameIndex);

		EWE_VK_ASSERT(vkEndCommandBuffer(commandBuffer));
		//printf("after end command buffer \n");
		VkResult vkResult = eweSwapChain->SubmitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR || mainWindow.wasWindowResized()) {
			mainWindow.resetWindowResizedFlag();
			RecreateSwapChain();
			camera.SetPerspectiveProjection(glm::radians(70.0f), eweSwapChain->ExtentAspectRatio(), 0.1f, 10000.0f);
			syncHub->WaitOnGraphicsFence(currentFrameIndex);
			isFrameStarted = false;
			currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
			return true;
		}
		EWE_VK_RESULT_ASSERT(vkResult);
		//printf("after submitting command buffer \n");
		syncHub->WaitOnGraphicsFence(currentFrameIndex);

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

		return false;
	}
	/**/
	void EWERenderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress!");
		assert(commandBuffer == GetCurrentCommandBuffer() && "can't begin render pass on command buffer from different frame");
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
			.image = eweSwapChain->GetImage(currentImageIndex),
			.subresourceRange = {
			  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			  .baseMipLevel = 0,
			  .levelCount = 1,
			  .baseArrayLayer = 0,
			  .layerCount = 1,
			}
		};

		image_memory_barrier.srcQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetPresentIndex();
		image_memory_barrier.dstQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetGraphicsIndex();

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // srcStageMask
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
			0,
			0, nullptr,
			0, nullptr,
			1, &image_memory_barrier
		);
		eweSwapChain->BeginRender(commandBuffer, currentImageIndex);
	}
	void EWERenderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress!");
		assert(commandBuffer == GetCurrentCommandBuffer() && "can't end render pass on command buffer from different frame");

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
			.image = eweSwapChain->GetImage(currentImageIndex),
			.subresourceRange = {
			  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			  .baseMipLevel = 0,
			  .levelCount = 1,
			  .baseArrayLayer = 0,
			  .layerCount = 1,
			}
		};
		image_memory_barrier.srcQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetGraphicsIndex();
		image_memory_barrier.dstQueueFamilyIndex = EWEDevice::GetEWEDevice()->GetPresentIndex();
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