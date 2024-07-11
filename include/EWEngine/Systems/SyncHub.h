#pragma once

#include "EWEngine/Graphics/SyncPool.h"
#include "EWEngine/Data/EWE_Memory.h"
#include "EWEngine/Graphics/TransferCommandManager.h"
#include "EWEngine/Graphics/TransitionManager.h

#include <mutex>
#include <condition_variable>
#include <vector>

#include <array>


namespace EWE {


	class SyncHub {
	private:
		friend class EWEDevice;

		static SyncHub* syncHubSingleton;

		SyncPool syncPool;
		TransitionManager transitionManager;
		
		VkDevice device;
		VkQueue queues[Queue::_count];
		uint32_t transferQueueIndex;

		VkCommandPool commandPools[Queue::_count];
		VkCommandBufferBeginInfo bufferBeginInfo{};

		VkSubmitInfo transferSubmitInfo{};

		bool readyForNextTransmit = true;
		//std::mutex transferFlipFlopMutex{};
		std::mutex transferPoolMutex{};
		//std::mutex mutexNextTransmit{};
		bool transferFlipFlop = false;


		//VkFence fence{ VK_NULL_HANDLE };
		VkFence singleTimeFenceGraphics{ VK_NULL_HANDLE };

		VkCommandBuffer renderBuffers[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
		RenderSyncData renderSyncData;

		VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
		std::vector<VkFence> imagesInFlight{};

		std::vector<VkCommandBuffer> graphicsSTCGroup{};

		bool transferring = false;

		//this is designed for synchronizing 2 threads, in which the cuck thread defers to the dom thread
		//specifically, it'd be used if one of the 2 threads has a higher priority
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
		SyncHub(VkDevice device);
		~SyncHub();

		//only class this from EWEDevice
		static void Initialize(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkQueue computeQueue, VkQueue transferQueue, VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool, uint32_t transferQueueIndex);
		void SetImageCount(uint32_t imageCount) {
			imagesInFlight.resize(imageCount, VK_NULL_HANDLE);
		}
		void Destroy(VkCommandPool renderPool, VkCommandPool computePool, VkCommandPool transferPool);

		VkCommandBuffer GetRenderBuffer(uint8_t frameIndex) {
			return renderBuffers[frameIndex];
		}

		//VkFence* GetFlightFence(uint8_t frameIndex) {
		//	return &inFlightFences[frameIndex];
		//}
		//VkSemaphore GetImageAvailableSemaphore(uint8_t frameIndex) {
		//	return imageAvailableSemaphores[frameIndex];
		//}

		void SubmitGraphics(VkSubmitInfo& submitInfo, uint8_t frameIndex, uint32_t* imageIndex);
		VkResult PresentKHR(VkPresentInfoKHR& presentInfo, uint8_t currentFrame);
		void WaitOnGraphicsFence(const uint8_t frameIndex);

		void DomDemand();
		void DomRelease();
		void CuckRequest();
		void CuckSubmit();

		VkCommandBuffer BeginSingleTimeCommandGraphics();

		//this needs to be called from the graphics thread
		//these have a fence with themselves
		void EndSingleTimeCommandGraphics(VkCommandBuffer cmdBuf);
		void EndSingleTimeCommandGraphicsSignal(VkCommandBuffer cmdBuf, VkSemaphore signalSemaphore);
		void EndSingleTimeCommandGraphicsWaitAndSignal(VkCommandBuffer cmdBuf, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore);

		void EndSingleTimeCommandGraphicsGroup(VkCommandBuffer cmdBuf);
		void SubmitGraphicsSTCGroup();

		VkCommandBuffer BeginSingleTimeCommandTransfer();

		void EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf);
		void EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf, std::function<void()> func);
		void EndSingleTimeCommandTransfer(SyncedCommandQueue* cmdQueue);

		VkCommandBuffer BeginSingleTimeCommand(Queue::Enum queue);

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

		void SubmitTransferBuffers();

	};
}