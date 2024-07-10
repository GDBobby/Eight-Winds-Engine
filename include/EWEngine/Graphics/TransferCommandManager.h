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

		void PushBack(CommandWithCallback const& cmdCb) {
			commands.push_back(cmdCb.cmdBuf);
			callbacks.push_back(cmdCb.callback);
		}
		std::function<void()> CombineCallbacks() {
			for(uint16_t i = 0; i < callbacks.size(); i++){
				if(callbacks[i] == nullptr){
					callbacks.erase(callbacks.begin() + i);
					i--;
				}
			}

			return [cbs = std::move(callbacks)]{
				for(auto const& cb : cbs){
					cb();
				}
			};
		}
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
		SyncedCommandQueue() {}
		VkCommandBuffer Pop();
		void Push(VkCommandBuffer cmdBuf);
		bool Finished() const;

		void SetBarrier(PipelineBarrier const& pipeBarrier);
		void GenerateMips(const bool generating);
	};

	namespace TransferCommandManager {

		bool Empty();
		template<typename F, typename ... Args>
		void AddCommand(VkCommandBuffer cmdBuf, F&& f, Args&&... args){
			requires(std::is_invocable_v<F, Args>)
			AddCommand(cmdBuf, std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		}

		CommandCallbacks PrepareSubmit(VkSubmitInfo& submitInfo);
		void AddCommand(VkCommandBuffer cmdBuf, std::function<void()> fnc);

		SyncedCommandQueue* BeginCommandQueue();
		void AddCommand(VkCommandBuffer cmdBuf);
		void EndCommandQueue(SyncedCommandQueue* cmdQ);
		//this requires the pointer to be deleted
	};
}//namespace EWE