#include "EWEngine/Graphics/TransferCommandManager.h"

#include <iterator>

namespace EWE {
	std::function<void()> CommandCallbacks::CombineCallbacks(VkDevice vkDevice) {
		for (uint16_t i = 0; i < callbacks.size(); i++) {
			if (callbacks[i] == nullptr) {
				callbacks.erase(callbacks.begin() + i);
				i--;
			}
		}
		if (callbacks.size() == 0) {
			return nullptr;
		}

		return [cbs = std::move(callbacks), sbs = std::move(stagingBuffers), device = vkDevice] {
			for (auto& stagingBuffer : sbs) {
				stagingBuffer.Free(device);
			}
			for (auto const& cb : cbs) {
				cb();
			}
		};
	}
	std::function<void()> CommandCallbacks::CombineGraphicsCallbacks() {
		for (uint16_t i = 0; i < graphicsCallbacks.size(); i++) {
			if (graphicsCallbacks[i] == nullptr) {
				graphicsCallbacks.erase(graphicsCallbacks.begin() + i);
				i--;
			}
		}
		if (graphicsCallbacks.size() == 0) {
			return nullptr;
		}

		return [cbs = std::move(graphicsCallbacks)] {
			for (auto const& cb : cbs) {
				cb();
			}
		};
	}

	VkCommandBuffer SyncedCommandQueue::Pop() {
		assert(size > 0 && "popping from an empty queue?");
		const uint8_t retIndex = currentIndex;
		if (retIndex == size || retIndex > TRANSFER_COMMAND_DEPTH_MAXIMUM) {
			assert(false && "the queue should've already been destroyed");
		}
		currentIndex++;
		return commandBuffers[retIndex];
	}
	void SyncedCommandQueue::Push(VkCommandBuffer cmdBuf) {
		assert(size < TRANSFER_COMMAND_DEPTH_MAXIMUM && "pushing too many command buffers to one synchronization queue");
		vkEndCommandBuffer(cmdBuf);
		commandBuffers[size] = cmdBuf;
		size++;
	}
	bool SyncedCommandQueue::Finished() const{
		return currentIndex == size;
	}

	void SyncedCommandQueue::SetBarrier(PipelineBarrier const& pipeBarrier) {
		pipelineBarrier = pipeBarrier;
	}

	void SyncedCommandQueue::GenerateMips(const bool generating) {
		generateMips = generating;
	}

	namespace TransferCommandManager {
		std::mutex syncQueueMutex{};
		std::mutex singleMutex{};
		std::mutex wrappedMutex{};
		std::mutex callbackMutex{};

		std::vector<SyncedCommandQueue*> synchronizedCommandBuffers{};
		std::vector<VkCommandBuffer> commandBuffers{};
		std::vector<CommandWithCallback> wrappedCmds{};

		bool Empty(){
			return (synchronizedCommandBuffers.size() == 0) && (commandBuffers.size() == 0) && (wrappedCmds.size() == 0);
		}

		SyncedCommandQueue* BeginCommandQueue() {
			return new SyncedCommandQueue();
		}

		CommandCallbacks PrepareSubmit(VkSubmitInfo& submitInfo){
			CommandCallbacks cmdCbs{};

			cmdCbs.commands.reserve(synchronizedCommandBuffers.size() + wrappedCmds.size() + commandBuffers.size());

			syncQueueMutex.lock();
			for(uint8_t i = 0; i < synchronizedCommandBuffers.size(); i++){
				VkCommandBuffer cmdBuf = synchronizedCommandBuffers[i]->Pop();
				cmdCbs.commands.push_back(cmdBuf);
				if(synchronizedCommandBuffers[i]->Finished()){
					cmdCbs.callbacks.push_back(std::move(synchronizedCommandBuffers[i]->callback));
					if(synchronizedCommandBuffers[i]->stagingBuffer.buffer != VK_NULL_HANDLE){
						cmdCbs.stagingBuffers.push_back(std::move(synchronizedCommandBuffers[i]->stagingBuffer));
					}
					delete synchronizedCommandBuffers[i];
					synchronizedCommandBuffers.erase(synchronizedCommandBuffers.begin() + i);
					i--;
				}
			}
			syncQueueMutex.unlock();

			singleMutex.lock();
			std::copy(commandBuffers.begin(), commandBuffers.end(), std::back_inserter(cmdCbs.commands));
			commandBuffers.clear();
			singleMutex.unlock();

			wrappedMutex.lock();
			for(auto& wrappedCmd : wrappedCmds){
				cmdCbs.commands.push_back(wrappedCmd.cmdBuf);
				cmdCbs.PushBack(wrappedCmd);

				cmdCbs.callbacks.push_back(wrappedCmd.callback);
			}
			wrappedCmds.clear();
			wrappedMutex.unlock();

			submitInfo.commandBufferCount = cmdCbs.commands.size();
			submitInfo.pCommandBuffers = cmdCbs.commands.data();
			return cmdCbs;
		}
		void AddCommand(VkCommandBuffer cmdBuf) {
			singleMutex.lock();
			commandBuffers.push_back(cmdBuf);
			singleMutex.unlock();
		}

		void AddCommand(VkCommandBuffer cmdBuf, std::function<void()> fnc){
			wrappedMutex.lock();
			wrappedCmds.emplace_back(cmdBuf, fnc);
			wrappedMutex.unlock();
		}

		void EndCommandQueue(SyncedCommandQueue* cmdQ) {
			syncQueueMutex.lock();
			synchronizedCommandBuffers.push_back(cmdQ);
			syncQueueMutex.unlock();
		}
	}
}