#pragma once

#include "EWEngine/MainWindow.h"
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Swapchain.hpp"
#include "EWEngine/Graphics/TextOverlay.h"
#include "EWEngine/Graphics/Camera.h"

#include <cassert>
#include <memory>
#include <vector>

namespace EWE {
	class EWERenderer {
	private:
		static EWERenderer* instance;

	public:

		static void bindGraphicsPipeline(VkCommandBuffer commandBuffer, VkPipeline graphicsPipeline) {
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

		EWERenderer(MainWindow& window, EWECamera& camera);
		~EWERenderer();

		EWERenderer(const EWERenderer&) = delete;
		EWERenderer& operator=(const EWERenderer&) = delete;

		//VkRenderPass getSwapChainRenderPass() const { return eweSwapChain->getRenderPass(); }
		float getAspectRatio() const { return eweSwapChain->extentAspectRatio(); }

		std::pair<uint32_t, uint32_t> getExtent() { 
			needToReadjust = false;
			return eweSwapChain->getExtent(); 
		}

		bool isFrameInProgresss() const { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer() const { 
			assert(isFrameStarted && "Cannot get command buffer when frame is not in progress!");
			//printf("currentFrameIndex: commandBuffers size, maxFIF - %d:%d:%d \n", currentFrameIndex, commandBuffers.size(), MAX_FRAMES_IN_FLIGHT);
			return syncHub->GetRenderBuffer(currentFrameIndex);
		}
		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frameindex when frame is not in progress!");
			return currentFrameIndex;
		}

		FrameInfo beginFrame();
		bool endFrame();

		//void beginSecondarySwapChainRenderPass(std::pair<VkCommandBuffer, VkCommandBuffer> commandBufferPair);
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
		TextOverlay* makeTextOverlay() {
			//assert(!loadingState && "text overlay being made in loading screen renderer?");
			assert(!hasTextOverlayBeenMade && "textoverlay has already been made?");
			hasTextOverlayBeenMade = true;
			printf("CREATING TEXT OVERLAY\n");

			return new TextOverlay{static_cast<float>(eweSwapChain->width()), static_cast<float>(eweSwapChain->height()), *eweSwapChain->getPipelineInfo()};
		}

		//void updateTextOverlay(float time, float peakTime, float averageTime, float minTime, float highTime, VkCommandBuffer commandBuffer);
		//std::unique_ptr<TextOverlay> textOverlay;

		bool needToReadjust = false;

		VkPipelineRenderingCreateInfo* getPipelineInfo() {
			return eweSwapChain->getPipelineInfo();
		}

	private:
		VkViewport viewport{};
		VkRect2D scissor{};

		
		void createCommandBuffers();
		void freeCommandBuffers();

		void recreateSwapChain();

		bool hasTextOverlayBeenMade = false;
		
		EWECamera& camera;
		MainWindow& mainWindow;
		std::unique_ptr<EWESwapChain> eweSwapChain;

		uint32_t currentImageIndex;
		uint8_t currentFrameIndex{0};
		bool isFrameStarted{false};

		std::shared_ptr<SyncHub> syncHub;

	};
}

