#pragma once

#include "EWEngine/Graphics/PipelineBarrier.h"
#include "EWEngine/Data/EngineDataTypes.h"
#include <queue>
#include <vector>
#include <cassert>
#include <mutex>
#include <functional>

#define TRANSFER_COMMAND_DEPTH_MAXIMUM 10

namespace EWE {

	struct CommandWithCallback{
		VkCommandBuffer cmdBuf;
		std::function<void()> callback{nullptr};
	};
	struct CommandCallbacks{
		std::vector<VkCommandBuffer> commands{};
		std::vector<StagingBuffer> stagingBuffers{}; //need to be freed
		std::vector<std::function<void()>> callbacks{};
		std::vector<std::function<void()>> graphicsCallbacks{};

		void PushBack(CommandWithCallback const& cmdCb) {
			commands.push_back(cmdCb.cmdBuf);
			callbacks.push_back(cmdCb.callback);
		}
		std::function<void()> CombineCallbacks(VkDevice vkDevice);
		std::function<void()> CombineGraphicsCallbacks();
	};
	struct GraphicsCallbacks {
		VkSemaphore waitSemaphore{};
		VkSemaphore signalSemaphore{};
		std::vector<std::function<void()>> callbacks;
	};

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
		StagingBuffer stagingBuffer{};

		std::function<void()> callback{nullptr};
		std::function<void()> graphicsCallback{ nullptr };
		SyncedCommandQueue() {}
		VkCommandBuffer Pop();
		void Push(VkCommandBuffer cmdBuf);
		bool Finished() const;

		void SetBarrier(PipelineBarrier const& pipeBarrier);
		void GenerateMips(const bool generating);
	};

	namespace TransferCommandManager {

		bool Empty();
		CommandCallbacks PrepareSubmit(VkSubmitInfo& submitInfo);
		void AddCommand(VkCommandBuffer cmdBuf, std::function<void()> fnc);
		template<typename F, typename ... Args>
		void AddCommand(VkCommandBuffer cmdBuf, F&& f, Args&&... args) {
			requires(std::is_invocable_v<F, Args>)
			AddCommand(cmdBuf, std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		}


		SyncedCommandQueue* BeginCommandQueue();
		void AddCommand(VkCommandBuffer cmdBuf);
		void EndCommandQueue(SyncedCommandQueue* cmdQ);
		//this requires the pointer to be deleted
	};
}//namespace EWE