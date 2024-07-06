#pragma once

#include "EWEngine/Graphics/PipelineBarrier.h"
#include "EWEngine/Data/EngineDataTypes.h"
#include <queue>
#include <vector>
#include <cassert>
#include <mutex>

#define TRANSFER_COMMAND_DEPTH_MAXIMUM 10

namespace EWE {
	class SyncedCommandQueue {
	private:
		VkCommandBuffer commandBuffers[TRANSFER_COMMAND_DEPTH_MAXIMUM] = { 
			VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, 
			VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE 
		};
		PipelineBarrier pipelineBarrier{};
		StagingBuffer stagingBuffer{};
		bool generateMips = false;
		uint8_t currentIndex{ 0 };
		uint8_t size{ 0 };

	public:
		SyncedCommandQueue() {}
		VkCommandBuffer Pop();
		void Push(VkCommandBuffer cmdBuf);
		void SetBarrier(PipelineBarrier const& pipeBarrier);
		void SetStagingBuffer(StagingBuffer const& stagingBuffer);
		void GenerateMips(const bool generating);
	};

	namespace TransferCommandManager {
		SyncedCommandQueue* BeginCommandQueue();
		void AddCommand(VkCommandBuffer cmdBuf);
		VkCommandBuffer PullCommand();
		void EndCommandQueue(SyncedCommandQueue* cmdQ);
		//this requires the pointer to be deleted
		SyncedCommandQueue* PullCommandQueue();
	};
}//namespace EWE