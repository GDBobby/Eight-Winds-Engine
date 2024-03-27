#pragma once

#include <EWEngine/Data/EWE_Memory.h>

#include <mutex>
#include <condition_variable>
#include <vector>

#include <vulkan/vulkan.h>
#include <array>

#include <iostream>
#include <stdexcept>

#define DECONSTRUCTION_DEBUG true

static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;
namespace EWE {
	class SyncHub {
	private:
		static SyncHub* syncHubSingleton;

		VkDevice device{};
		VkQueue graphicsQueue{};
		VkQueue presentQueue{};
		VkQueue computeQueue{};
		VkQueue transferQueue{};

		VkCommandPool transferCommandPool{};
		VkCommandBufferBeginInfo bufferBeginInfo{};

		VkSubmitInfo computeSubmitInfo{};
		std::array<VkSubmitInfo, 5> oceanSubmitInfo{};
		VkPipelineStageFlags graphicsToComputeWaitStageMask{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkPipelineStageFlags computeWaitStageMask{ VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
		VkSubmitInfo transferSubmitInfo{};

		bool readyForNextTransmit = true;
		//std::mutex transferFlipFlopMutex{};
		std::mutex transferPoolMutex{};
		//std::mutex mutexNextTransmit{};
		bool transferFlipFlop = false;
		std::array<std::vector<VkCommandBuffer>, 2> transferBuffers{};

		using StageMask = std::vector<VkSemaphore>;
		//VkFence fence{ VK_NULL_HANDLE };
		VkFence singleTimeFence{ VK_NULL_HANDLE };

		//0 is compute -> graphics, 1 is graphics -> compute
		std::array<VkCommandBuffer, 2> oceanTransferBuffers{ VK_NULL_HANDLE, VK_NULL_HANDLE };
		std::array<VkSubmitInfo, 2> oceanTransfersSubmitInfo{};

		VkSemaphore computeToGraphicsTransferSemaphore{VK_NULL_HANDLE};
		VkSemaphore graphicsToComputeTransferSemaphore{VK_NULL_HANDLE};

		VkFence oceanFlightFence;
		std::array<VkSemaphore, 5> oceanSemaphores{VK_NULL_HANDLE};
		//std::array<VkEvent, 4> oceanWaitEvents;

		uint8_t graphicsSemaphoreIndex{ 0 };
		VkSemaphore computeSemaphore{ VK_NULL_HANDLE };
		VkSemaphore graphicsSemaphore{ VK_NULL_HANDLE };
		std::vector<VkSemaphore> imageAvailableSemaphores{}; //resized to maxFramesInFlight
		std::vector<VkSemaphore> renderFinishedSemaphores{};//resized to maxFramesInFlight
		std::vector<VkCommandBuffer> renderBuffers{};
		VkCommandBuffer computeBuffer{VK_NULL_HANDLE};
		std::array<VkCommandBuffer, 5> oceanBuffers = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
		VkCommandBuffer transferBuffer{ VK_NULL_HANDLE };

		VkFence computeInFlightFence{VK_NULL_HANDLE};
		std::vector<VkFence> inFlightFences{}; //resized to maxFramesInFlight
		std::vector<VkFence> imagesInFlight{}; //resized to maxFramesInFlight

		VkSemaphore transferSemaphore{}; //currently doing dom cuck sync

		bool transferring = false;

		bool oceanComputing = false;
		bool computing = false;
		bool rendering = true;
		std::vector<StageMask> graphicsWait{};
		std::vector<StageMask> graphicsSignal{};

		std::vector<StageMask> computeWait{};
		std::vector<StageMask> computeSignal{};

		struct DomCuckSync {
			std::mutex domMutex{};
			std::mutex cuckMutex{};
			std::condition_variable domCondition{};
			std::condition_variable cuckCondition{};
			bool domConditionHeld = false;
			bool cuckConditionHeld = false;
		};
		DomCuckSync domCuckSync{};
	public:		
		static SyncHub* getSyncHubInstance() {
			static std::mutex mtx;

			std::lock_guard<std::mutex> lock(mtx);
			if (syncHubSingleton == nullptr) {
				syncHubSingleton = ConstructSingular<SyncHub>(ewe_call_trace);
				std::cout << "COSTRUCTING SYNCHUB" << std::endl;
			}
			return syncHubSingleton;
		}
		SyncHub() {
			std::cout << "COSTRUCTING SYNCHUB" << std::endl;
		}
		~SyncHub() {
			std::cout << "DE COSTRUCTING SYNCHUB" << std::endl;
		}

		//only class this from EWEDevice
		void initialize(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkQueue computeQueue, VkQueue transferQueue, VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool);
		void setImageCount(uint32_t imageCount) {
			imagesInFlight.resize(imageCount, VK_NULL_HANDLE);
		}
		void destroy(VkCommandPool renderPool, VkCommandPool computePool, VkCommandPool transferPool);

		//void enableCompute();
		//void disableCompute();

		VkCommandBuffer getRenderBuffer(uint8_t frameIndex) {
			return renderBuffers[frameIndex];
		}

		//void setComputeMask(VkSubmitInfo& computeSubmitInfo);
		VkFence* getFlightFence(uint8_t frameIndex) {
			return &inFlightFences[frameIndex];
		}
		VkSemaphore getImageAvailableSemaphore(uint8_t frameIndex) {
			return imageAvailableSemaphores[frameIndex];
		}

		void submitGraphics(VkSubmitInfo& submitInfo, uint8_t frameIndex, uint32_t* imageIndex);
		VkResult presentKHR(VkPresentInfoKHR& presentInfo, uint8_t currentFrame);

		void domDemand();
		void domRelease();
		void cuckRequest();
		void cuckSubmit();

		std::array<VkCommandBuffer, 5> beginOceanBuffers();
		void endOceanBuffers();
		
		void oceanSubmission();
		VkCommandBuffer beginComputeBuffer();
		void endComputeBuffer();

		VkCommandBuffer beginSingleTimeCommands();

		void prepTransferSubmission(VkCommandBuffer transferBuffer);
		void submitCompute();

		VkCommandBuffer beginTransferringOceanToGraphics() {
			std::cout << "beginning OTG transfer 0 \n";
			vkBeginCommandBuffer(oceanTransferBuffers[0], &bufferBeginInfo);
			std::cout << "after beginning OTG transfer 0 \n";
			return oceanTransferBuffers[0];
		}
		void endTransferringOceanToGraphics() {
			vkEndCommandBuffer(oceanTransferBuffers[0]);
		}
		VkCommandBuffer beginTransferringGraphicsToOcean() {
			std::cout << "beginning GTO transfer 1 \n";
			vkBeginCommandBuffer(oceanTransferBuffers[1], &bufferBeginInfo);
			std::cout << "after GTO transfer 1 \n";
			return oceanTransferBuffers[1];
		}
		void endTransferringGraphicsToOcean() {
			vkEndCommandBuffer(oceanTransferBuffers[1]);
		}

		void waitOnTransferFence();

	private:

		void createSyncObjects();

		void initEvents() {
			/*
			for (uint8_t i = 0; i < 4; i++) {
				VkEventCreateInfo eventInfo = {};
				eventInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
				vkCreateEvent(device, &eventInfo, nullptr, &oceanWaitEvents[i]);
			}
			*/
		}
		void initOceanSubmitInfo();
		void initWaitMask();
		void initSignalMask();

		void setMaxFramesInFlight();
		void createBuffers(VkCommandPool commandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool);

		void submitTransferBuffers();
		
		/*
		void computeBarriers(VkCommandBuffer renderBuf) {
			//need this for derivatives, turbulence, and displacement

			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			// We won't be changing the layout of the image
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			//imageMemoryBarrier.image = textureComputeTarget.image;
			imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vkCmdPipelineBarrier(
				renderBuf,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier
			);
		}
		*/

	};
}