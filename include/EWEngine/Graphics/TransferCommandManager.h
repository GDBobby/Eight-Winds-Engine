#pragma once

#include "EWEngine/Graphics/PipelineBarrier.h"
#include "EWEngine/Data/EngineDataTypes.h"
#include <queue>
#include <vector>
#include <cassert>
#include <mutex>
#include <functional>

#define TRANSFER_COMMAND_DEPTH_MAXIMUM 10

#define SYNC_QUEUES false

namespace EWE {


	struct CommandWithCallback{
		VkCommandBuffer cmdBuf;
		std::function<void()> callback{nullptr};
		std::function<void()> graphicsCallback{ nullptr };
	};
	struct CommandCallbacks{
		std::vector<VkCommandBuffer> commands{};
		std::vector<std::function<void()>> callbacks{};
		std::vector<std::function<void()>> graphicsCallbacks{};

		void PushBack(CommandWithCallback const& cmdCb);
		TransferCallbackReturn CombineCallbacks(VkDevice vkDevice, VkCommandPool transferPool);
		bool CleanGraphicsCallbacks();
		std::function<void()> CombineGraphicsCallbacks();
	};
	struct GraphicsCallbacks {
		VkSemaphore waitSemaphore{};
		VkSemaphore signalSemaphore{};
		std::vector<std::function<void()>> callbacks;
	};
#if SYNC_QUEUES
	class SyncedCommandQueue {
	private:
		VkCommandBuffer commandBuffers[TRANSFER_COMMAND_DEPTH_MAXIMUM] = { 
			VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, 
			VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE 
		};
		PipelineBarrier pipelineBarrier{};
		bool generateMips = false;
		uint8_t currentIndex{ 0 };
		uint8_t size{ 0 };

	public:
		SemaphoreData* lastSignaled{ nullptr };
		StagingBuffer stagingBuffer{};

		std::function<void()> callback{nullptr};
		std::function<void()> graphicsCallback{ nullptr };
		SyncedCommandQueue() {}
		VkCommandBuffer Pop();
		void Push(VkCommandBuffer cmdBuf);
		bool Finished() const;

		void SetBarrier(PipelineBarrier const& pipeBarrier);
	};
#endif

	namespace TransferCommandManager {

		bool Empty();
		CommandCallbacks PrepareSubmit();
		void AddCommand(CommandWithCallback cmdCb);

#if SYNC_QUEUES
		SyncedCommandQueue* BeginCommandQueue();
		void EndCommandQueue(SyncedCommandQueue* cmdQ);
#endif
		void AddCommand(VkCommandBuffer cmdBuf);
		//this requires the pointer to be deleted
	};
}//namespace EWE