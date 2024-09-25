#pragma once

#include "EWEngine/Graphics/SyncPool.h"
#include "EWEngine/Data/EWE_Memory.h"
#include "EWEngine/Graphics/TransferCommandManager.h"
#include "EWEngine/Graphics/TransitionManager.h"

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
#if EWE_DEBUG
		~SyncHub();
#endif

		//only class this from EWEDevice
		static void Initialize(VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, VkQueue computeQueue, VkQueue transferQueue, VkCommandPool renderCommandPool, VkCommandPool computeCommandPool, VkCommandPool transferCommandPool, uint32_t transferQueueIndex);
		void SetImageCount(uint32_t imageCount) {
			imagesInFlight.resize(imageCount, VK_NULL_HANDLE);
		}
		void Destroy(VkCommandPool renderPool, VkCommandPool computePool, VkCommandPool transferPool);

		VkCommandBuffer GetRenderBuffer(uint8_t frameIndex) {
			return renderBuffers[frameIndex];
		}

		VkFence* GetFlightFence(uint8_t frameIndex) {
			return &renderSyncData.inFlight[frameIndex];
		}
		VkSemaphore GetImageAvailableSemaphore(uint8_t frameIndex) {
			return renderSyncData.imageAvailable[frameIndex];
		}

		void SubmitGraphics(VkSubmitInfo& submitInfo, uint8_t frameIndex, uint32_t* imageIndex);
		VkResult PresentKHR(VkPresentInfoKHR& presentInfo, uint8_t currentFrame);

		void DomDemand();
		void DomRelease();
		void CuckRequest();
		void CuckSubmit();

		//this needs to be called from the graphics thread
		//these have a fence with themselves
		void EndSingleTimeCommandGraphics(VkCommandBuffer cmdBuf);
		void EndSingleTimeCommandGraphicsSignal(VkCommandBuffer cmdBuf, VkSemaphore signalSemaphore);
		void EndSingleTimeCommandGraphicsWaitAndSignal(VkCommandBuffer cmdBuf, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore);

		void EndSingleTimeCommandGraphicsGroup(VkCommandBuffer cmdBuf);
		void SubmitGraphicsSTCGroup();

		void EndSingleTimeCommandTransfer(VkCommandBuffer cmdBuf);
		void EndSingleTimeCommandTransfer(CommandWithCallback cmdCb);
#if SYNC_QUEUE
		void EndSingleTimeCommandTransfer(SyncedCommandQueue* cmdQueue);
#endif

		VkCommandBuffer BeginSingleTimeCommand(Queue::Enum queue);
		VkCommandBuffer BeginSingleTimeCommandGraphics();
		VkCommandBuffer BeginSingleTimeCommandTransfer();

		void AttemptTransferSubmission();

		void WaitOnGraphicsFence(const uint8_t frameIndex) {
			EWE_VK_ASSERT(vkWaitForFences(device, 1, &renderSyncData.inFlight[frameIndex], VK_TRUE, UINT64_MAX));
		}

		void RunGraphicsCallbacks();
		bool CheckFencesForUsage() {
			return syncPool.CheckFencesForUsage();
		}

	private:

		void CreateSyncObjects();

		void CreateBuffers(VkCommandPool graphicsCommandPool);

		void SubmitTransferBuffers();

		const std::thread::id main_thread;

	};
}