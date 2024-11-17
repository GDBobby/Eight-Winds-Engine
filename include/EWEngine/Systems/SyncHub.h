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


		VkSubmitInfo transferSubmitInfo{};

		bool readyForNextTransmit = true;
		//std::mutex transferFlipFlopMutex{};
		std::mutex graphicsAsyncMut{};
		//std::mutex mutexNextTransmit{};
		bool transferFlipFlop = false;


		//VkFence fence{ VK_NULL_HANDLE };
		VkFence singleTimeFenceGraphics{ VK_NULL_HANDLE };

		RenderSyncData renderSyncData;

		std::vector<VkFence> imagesInFlight{};

		struct SubmitGroup {
			CommandBuffer* cmdBuf;
			std::vector<SemaphoreData*> semaphoreData;
			SubmitGroup(CommandBuffer& cmdBuf, std::vector<SemaphoreData*>& semaphoreData) :
				cmdBuf{ &cmdBuf }, 
				semaphoreData{ std::move(semaphoreData) } 
			{}
		};
		std::vector<SubmitGroup> graphicsSTCGroup{};

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
		SyncHub();
#if EWE_DEBUG
		~SyncHub();
#endif

		//only class this from EWEDevice
		static void Initialize();
		void SetImageCount(uint32_t imageCount) {
			imagesInFlight.resize(imageCount, VK_NULL_HANDLE);
		}
		void Destroy();

		VkFence* GetFlightFence() {
			return &renderSyncData.inFlight[VK::Object->frameIndex];
		}
		VkSemaphore GetImageAvailableSemaphore() {
			return renderSyncData.imageAvailableSemaphore[VK::Object->frameIndex];
		}

		void SubmitGraphics(VkSubmitInfo& submitInfo, uint32_t* imageIndex);
		VkResult PresentKHR(VkPresentInfoKHR& presentInfo);

		void DomDemand();
		void DomRelease();
		void CuckRequest();
		void CuckSubmit();

		//this needs to be called from the graphics thread
		//these have a fence with themselves
		void EndSingleTimeCommandGraphics(CommandBuffer& cmdBuf);
		//void EndSingleTimeCommandGraphicsSignal(CommandBufferData& cmdBuf, VkSemaphore signalSemaphore);
		//void EndSingleTimeCommandGraphicsWaitAndSignal(CommandBufferData& cmdBuf, VkSemaphore& waitSemaphore, VkSemaphore& signalSemaphore);

		static void EndSingleTimeCommandGraphicsGroup(CommandBuffer& cmdBuf, std::vector<SemaphoreData*> waitSemaphores);
		//void SubmitGraphicsSTCGroup();

		//void EndSingleTimeCommandTransfer(CommandBuffer cmdBuf);
		void EndSingleTimeCommandTransfer();

		CommandBuffer& BeginSingleTimeCommand(Queue::Enum queue);
		CommandBuffer& BeginSingleTimeCommandGraphics();
		CommandBuffer& BeginSingleTimeCommandTransfer();

		void AttemptTransferSubmission();

		void WaitOnGraphicsFence() {
			EWE_VK(vkWaitForFences, VK::Object->vkDevice, 1, &renderSyncData.inFlight[VK::Object->frameIndex], VK_TRUE, UINT64_MAX);
		}

		void RunGraphicsCallbacks();
		bool CheckFencesForUsage() {
			return syncPool.CheckFencesForUsage();
		}

	private:

		void CreateSyncObjects();

		void CreateBuffers();

		void SubmitTransferBuffers();

		const std::thread::id main_thread;

	};
}