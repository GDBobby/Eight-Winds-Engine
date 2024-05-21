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

		static void BindGraphicsPipeline(VkCommandBuffer commandBuffer, VkPipeline graphicsPipeline);

		EWERenderer(MainWindow& window, EWECamera& camera);
		~EWERenderer();

		EWERenderer(const EWERenderer&) = delete;
		EWERenderer& operator=(const EWERenderer&) = delete;

		//VkRenderPass getSwapChainRenderPass() const { return eweSwapChain->getRenderPass(); }
		float GetAspectRatio() const { return eweSwapChain->ExtentAspectRatio(); }

		std::pair<uint32_t, uint32_t> GetExtent() { 
			needToReadjust = false;
			return eweSwapChain->GetExtent(); 
		}

		bool IsFrameInProgresss() const { return isFrameStarted; }
		VkCommandBuffer GetCurrentCommandBuffer() const { 
			assert(isFrameStarted && "Cannot get command buffer when frame is not in progress!");
			//printf("currentFrameIndex: commandBuffers size, maxFIF - %d:%d:%d \n", currentFrameIndex, commandBuffers.size(), MAX_FRAMES_IN_FLIGHT);
			return syncHub->GetRenderBuffer(currentFrameIndex);
		}
		int GetFrameIndex() const {
			assert(isFrameStarted && "Cannot get frameindex when frame is not in progress!");
			return currentFrameIndex;
		}

		FrameInfo BeginFrame();
		bool EndFrame();
		bool EndFrame(VkSemaphore waitSemaphore);
		bool EndFrameAndWaitForFence();

		//void beginSecondarySwapChainRenderPass(std::pair<VkCommandBuffer, VkCommandBuffer> commandBufferPair);
		void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);
		TextOverlay* MakeTextOverlay() {
			//assert(!loadingState && "text overlay being made in loading screen renderer?");
			assert(!hasTextOverlayBeenMade && "textoverlay has already been made?");
			hasTextOverlayBeenMade = true;
			printf("CREATING TEXT OVERLAY\n");

			return new TextOverlay{static_cast<float>(eweSwapChain->Width()), static_cast<float>(eweSwapChain->Height()), *eweSwapChain->GetPipelineInfo()};
		}

		//void updateTextOverlay(float time, float peakTime, float averageTime, float minTime, float highTime, VkCommandBuffer commandBuffer);
		//std::unique_ptr<TextOverlay> textOverlay;

		bool needToReadjust = false;

		VkPipelineRenderingCreateInfo* getPipelineInfo() {
			return eweSwapChain->GetPipelineInfo();
		}

	private:
		VkViewport viewport{};
		VkRect2D scissor{};

		
		//void createCommandBuffers();
		//void freeCommandBuffers();

		void RecreateSwapChain();

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

