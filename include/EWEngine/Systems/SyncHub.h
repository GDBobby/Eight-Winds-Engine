#pragma once

#include "EWEngine/Graphics/QueueSyncPool.h"
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

		QueueSyncPool qSyncPool;


		VkSubmitInfo transferSubmitInfo{};

		std::mutex transferSubmissionMut{};


		//VkFence fence{ VK_NULL_HANDLE };
		VkFence singleTimeFenceGraphics{ VK_NULL_HANDLE };

		RenderSyncData renderSyncData;

		std::vector<VkFence> imagesInFlight{};


		bool transferring = false;
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

		//this needs to be called from the graphics thread
		//these have a fence with themselves
		void EndSingleTimeCommandGraphics(CommandBuffer& cmdBuf);

		void EndSingleTimeCommandTransfer();

		CommandBuffer& BeginSingleTimeCommand(Queue::Enum queue);
		CommandBuffer& BeginSingleTimeCommandGraphics();
		CommandBuffer& BeginSingleTimeCommandTransfer();

		void WaitOnGraphicsFence() {
			EWE_VK(vkWaitForFences, VK::Object->vkDevice, 1, &renderSyncData.inFlight[VK::Object->frameIndex], VK_TRUE, UINT64_MAX);
		}

		void RunGraphicsCallbacks();
		bool CheckFencesForUsage() {
			return qSyncPool.CheckFencesForUsage();
		}
		//only access this from EndSingleTimeCommandTransfer for concurrency purposes
		void SubmitTransferBuffers();

	private:

		void CreateSyncObjects();

		void CreateBuffers();
		bool transferSubmissionThreadActive = false;

	};
}