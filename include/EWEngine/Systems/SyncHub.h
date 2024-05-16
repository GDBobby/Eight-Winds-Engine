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
		struct ImageQueueTransitionStruct {
			VkImage image;
			bool generateMips;
			uint32_t dstQueue;
			ImageQueueTransitionStruct(VkImage image, bool mips, uint32_t dstQueue) : image(image), generateMips(mips), dstQueue(dstQueue) {}
		};
		struct SingleTimeStruct {

			std::vector<ImageQueueTransitionStruct> images{};
			//buffer is not currently supported, but if it was, it would be put here
			//std::vector<VkBuffer> buffers{};

		};
		struct GraphicsTransitionStruct {
			std::vector<ImageQueueTransitionStruct> images{};
			std::vector<VkSemaphore> waitSemaphores{};
		};


		static SyncHub* syncHubSingleton;

		VkDevice device{};
		VkQueue graphicsQueue{};
		VkQueue presentQueue{};
		VkQueue computeQueue{};
		VkQueue transferQueue{};
		uint32_t transferQueueIndex;

		VkCommandPool transferCommandPool{};
		VkCommandBufferBeginInfo bufferBeginInfo{};

		VkSubmitInfo transferSubmitInfo{};

		bool readyForNextTransmit = true;
		//std::mutex transferFlipFlopMutex{};
		std::mutex transferPoolMutex{};
		//std::mutex mutexNextTransmit{};
		bool transferFlipFlop = false;

		//one in flight, one being built
		//the transfer queue doesn't work like the graphics queue, which could have 2 frames in flight
		//the transfer queue will only have one submission in flight
		std::array<std::vector<VkCommandBuffer>, 2> transferBuffers{};

		//one for each frame in flight, one for each built transfer
		//i think if everything goes right, there should never be more than one in flight. but it really depends
		//on how quickly the transfer queue works. if it processes 3 submissions in 1 graphics submission, singleTimeStructs will be overflowed
		//if the transfer queue works slower than the graphics queue, this structure will be fine
		std::array<SingleTimeStruct, MAX_FRAMES_IN_FLIGHT> singleTimeStructs{}; 

		using StageMask = std::vector<VkSemaphore>;
		//VkFence fence{ VK_NULL_HANDLE };
		VkFence singleTimeFence{ VK_NULL_HANDLE };

		std::vector<VkSemaphore> imageAvailableSemaphores{}; //resized to maxFramesInFlight
		std::vector<VkSemaphore> renderFinishedSemaphores{};//resized to maxFramesInFlight
		std::vector<VkCommandBuffer> renderBuffers{};
		VkCommandBuffer transferBuffer{ VK_NULL_HANDLE };

		std::vector<VkFence> inFlightFences{}; //resized to maxFramesInFlight
		std::vector<VkFence> imagesInFlight{}; //resized to maxFramesInFlight

		VkSemaphore transferSemaphore{}; //currently doing dom cuck sync

		bool transferring = false;

		bool oceanComputing = false;
		bool computing = false;
		bool rendering = true;
		std::vector<StageMask> graphicsWait{};
		std::vector<StageMask> graphicsSignal{};

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
				//std::cout << "COSTRUCTING SYNCHUB" << std::endl;
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
		void initialize(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkQueue computeQueue, VkQueue transferQueue, VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool, uint32_t transferQueueIndex);
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

		VkCommandBuffer BeginSingleTimeCommand(VkCommandPool cmdPool);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommand(VkCommandBuffer cmdBuf);
		void EndSingleTimeCommandTransition(VkCommandBuffer cmdBuf, VkImage image, bool generateMips, uint32_t dstQueue);

		void prepTransferSubmission(VkCommandBuffer transferBuffer);
		void prepTransferSubmission(VkCommandBuffer transferBuffer, VkImage image, bool generateMips, uint32_t dstQueue);

		void waitOnTransferFence();

	private:

		void createSyncObjects();

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