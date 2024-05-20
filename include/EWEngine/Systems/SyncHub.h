#pragma once

#include <EWEngine/Data/EWE_Memory.h>
#include <EWEngine/Data/TransitionManager.h>

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
		friend class EWEDevice;

		static SyncHub* syncHubSingleton;

		VkDevice device{};
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkQueue computeQueue;
		VkQueue transferQueue;
		uint32_t transferQueueIndex;

		VkCommandPool transferCommandPool;
		VkCommandPool graphicsCommandPool;
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
		Transition_Manager transitionManager;


		using StageMask = std::vector<VkSemaphore>;
		//VkFence fence{ VK_NULL_HANDLE };
		VkFence singleTimeFenceGraphics{VK_NULL_HANDLE};
		VkFence singleTimeFenceTransfer{ VK_NULL_HANDLE };

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
		static SyncHub* GetSyncHubInstance() {
			return syncHubSingleton;
			
		}
		SyncHub() : transitionManager{2, 2} {

			std::cout << "COSTRUCTING SYNCHUB" << std::endl;
		}
		~SyncHub() {
			std::cout << "DE COSTRUCTING SYNCHUB" << std::endl;
		}

		//only class this from EWEDevice
		static void Initialize(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkQueue computeQueue, VkQueue transferQueue, VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool, uint32_t transferQueueIndex);
		void SetImageCount(uint32_t imageCount) {
			imagesInFlight.resize(imageCount, VK_NULL_HANDLE);
		}
		void Destroy(VkCommandPool renderPool, VkCommandPool computePool, VkCommandPool transferPool);

		//void enableCompute();
		//void disableCompute();

		VkCommandBuffer GetRenderBuffer(uint8_t frameIndex) {
			return renderBuffers[frameIndex];
		}

		//void setComputeMask(VkSubmitInfo& computeSubmitInfo);
		VkFence* GetFlightFence(uint8_t frameIndex) {
			return &inFlightFences[frameIndex];
		}
		VkSemaphore GetImageAvailableSemaphore(uint8_t frameIndex) {
			return imageAvailableSemaphores[frameIndex];
		}

		void SubmitGraphics(VkSubmitInfo& submitInfo, uint8_t frameIndex, uint32_t* imageIndex);
		VkResult PresentKHR(VkPresentInfoKHR& presentInfo, uint8_t currentFrame);

		void DomDemand();
		void DomRelease();
		void CuckRequest();
		void CuckSubmit();

		VkCommandBuffer BeginSingleTimeCommandGraphics();
		void EndSingleTimeCommandGraphics(VkCommandBuffer cmdBuf);
		void EndSingleTimeCommandGraphicsSignal(VkCommandBuffer cmdBuf, VkSemaphore signalSemaphore);
		void EndSingleTimeCommandGraphicsWaitAndSignal(VkCommandBuffer cmdBuf, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore);

		VkCommandBuffer BeginSingleTimeCommandTransfer();

		void EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, BufferQueueTransitionData const& bufferData);
		void EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, std::vector<BufferQueueTransitionData> const& bufferData);
		void EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, ImageQueueTransitionData const& imageData);
		void EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, std::vector<ImageQueueTransitionData> const& imageData);

		//void PrepTransferSubmission(VkCommandBuffer transferBuffer, VkBuffer buffer, uint32_t dstQueue);
		//void PrepTransferSubmission(VkCommandBuffer transferBuffer, VkBuffer* buffers, uint8_t bufferCount, uint32_t dstQueue);
		//void PrepTransferSubmission(VkCommandBuffer transferBuffer, VkImage image, bool generateMips, uint32_t dstQueue);
		//void PrepTransferSubmission(VkCommandBuffer transferBuffer, VkImage* images, uint8_t imageCount, bool generateMips, uint32_t dstQueue);

		void AttemptTransferSubmission();

		void WaitOnTransferFence();

	private:

		void CreateSyncObjects();

		void InitWaitMask();
		void InitSignalMask();

		void SetMaxFramesInFlight();
		void CreateBuffers(VkCommandPool commandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool);

		void SubmitTransferBuffers(VkSemaphore signalSemaphore);

	};
}