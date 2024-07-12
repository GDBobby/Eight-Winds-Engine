#include "EWEngine/Graphics/TransferCommandManager.h"

#include <iterator>

namespace EWE {
	void CommandCallbacks::PushBack(CommandWithCallback const& cmdCb) {
		commands.push_back(cmdCb.cmdBuf);
		if (cmdCb.callback != nullptr) {
			callbacks.push_back(cmdCb.callback);
		}
		if (cmdCb.graphicsCallback != nullptr) {
			graphicsCallbacks.push_back(cmdCb.graphicsCallback);
		}
	}

	std::function<void()> CommandCallbacks::CombineCallbacks(VkDevice vkDevice, VkCommandPool transferPool) {
		for (uint16_t i = 0; i < callbacks.size(); i++) {
			if (callbacks[i] == nullptr) {
				callbacks.erase(callbacks.begin() + i);
				i--;
			}
		}
		if (callbacks.size() == 0) {
			return nullptr;
		}

		return [cbs = std::move(callbacks), cmds = this->commands, transferPool, vkDevice] {

			vkFreeCommandBuffers(vkDevice, transferPool, static_cast<uint32_t>(cmds.size()), cmds.data());

			for (auto const& cb : cbs) {
				cb();
			}
		};
	}
	bool CommandCallbacks::CleanGraphicsCallbacks() {
		for (uint16_t i = 0; i < graphicsCallbacks.size(); i++) {
			if (graphicsCallbacks[i] == nullptr) {
				graphicsCallbacks.erase(graphicsCallbacks.begin() + i);
				i--;
			}
		}
		return graphicsCallbacks.size() > 0;
	}
	std::function<void()> CommandCallbacks::CombineGraphicsCallbacks() {
		return [cbs = std::move(graphicsCallbacks)] {
			for (auto const& cb : cbs) {
				cb();
			}
		};
	}
#if SYNC_QUEUE
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
#endif

	namespace TransferCommandManager {
		std::mutex callbackMutex{};

#if SYNC_QUEUE
		std::mutex syncQueueMutex{};
		std::vector<SyncedCommandQueue*> synchronizedCommandBuffers{};
#endif

		std::mutex singleMutex{};
		std::vector<VkCommandBuffer> commandBuffers{};

		std::mutex wrappedMutex{};
		std::vector<CommandWithCallback> wrappedCmds{};

		bool Empty(){
			return 
#if SYNC_QUEUE
				(synchronizedCommandBuffers.size() == 0) && 
#endif
				(commandBuffers.size() == 0) && (wrappedCmds.size() == 0);
		}
#if SYNC_QUEUE
		SyncedCommandQueue* BeginCommandQueue() {
			return new SyncedCommandQueue();
		}
#endif

		CommandCallbacks PrepareSubmit(){
			CommandCallbacks cmdCbs{};

			cmdCbs.commands.reserve(
#if SYNC_QUEUE
				synchronizedCommandBuffers.size() + 
#endif
				wrappedCmds.size() + commandBuffers.size());
#if SYNC_QUEUE
			syncQueueMutex.lock();
			for(uint8_t i = 0; i < synchronizedCommandBuffers.size(); i++){
				VkCommandBuffer cmdBuf = synchronizedCommandBuffers[i]->Pop();
				cmdCbs.commands.push_back(cmdBuf);
				if (synchronizedCommandBuffers[i]->lastSignaled != nullptr) {
					bool foundDuplicate = false;
					for (uint8_t j = 0; j < i; j++) {
						foundDuplicate |= synchronizedCommandBuffers[i] == synchronizedCommandBuffers[j];
					}
					if (!foundDuplicate) {
						cmdCbs.waitSemaphores.push_back(synchronizedCommandBuffers[i]->lastSignaled);
					}
					synchronizedCommandBuffers[i]->lastSignaled = nullptr;
				}

				if(synchronizedCommandBuffers[i]->Finished()){
					cmdCbs.callbacks.push_back(std::move(synchronizedCommandBuffers[i]->callback));
					if(synchronizedCommandBuffers[i]->stagingBuffer.buffer != VK_NULL_HANDLE){
						cmdCbs.stagingBuffers.push_back(std::move(synchronizedCommandBuffers[i]->stagingBuffer));
					}
					delete synchronizedCommandBuffers[i];
					synchronizedCommandBuffers.erase(synchronizedCommandBuffers.begin() + i);
					i--;
				}
				else {
					cmdCbs.signalSemaphores.push_back(&synchronizedCommandBuffers[i]->lastSignaled);
				}
			}
			syncQueueMutex.unlock();
#endif

			singleMutex.lock();
			std::copy(commandBuffers.begin(), commandBuffers.end(), std::back_inserter(cmdCbs.commands));
			commandBuffers.clear();
			singleMutex.unlock();

			wrappedMutex.lock();
			for(auto& wrappedCmd : wrappedCmds){
				cmdCbs.PushBack(wrappedCmd);
			}
			wrappedCmds.clear();
			wrappedMutex.unlock();

			return cmdCbs;
		}
		void AddCommand(VkCommandBuffer cmdBuf) {
			singleMutex.lock();
			commandBuffers.push_back(cmdBuf);
			singleMutex.unlock();
		}

		void AddCommand(CommandWithCallback cmdCb){
			wrappedMutex.lock();
			wrappedCmds.push_back(cmdCb);
			wrappedMutex.unlock();
		}
#if SYNC_QUEUE
		void EndCommandQueue(SyncedCommandQueue* cmdQ) {
#ifdef _DEBUG
			assert(cmdQ != nullptr && "failed to initialize this queue");
#endif
			syncQueueMutex.lock();
			synchronizedCommandBuffers.push_back(cmdQ);
			syncQueueMutex.unlock();
		}
#endif
	} //namespace TransferCommandManager
} //namespace EWE