#include "EWEngine/Graphics/TransferCommandManager.h"

namespace EWE {
	VkCommandBuffer SyncedCommandQueue::Pop() {
		assert(size > 0 && "popping from an empty queue?");
		const uint8_t retIndex = currentIndex;
		if (retIndex == size) {
			return VK_NULL_HANDLE;
		}
		else if (retIndex > TRANSFER_COMMAND_DEPTH_MAXIMUM) {
			assert(false && "popping too many command buffers from one sync queue");
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
	void SyncedCommandQueue::SetBarrier(PipelineBarrier const& pipeBarrier) {
		pipelineBarrier = pipeBarrier;
	}
	void SyncedCommandQueue::SetStagingBuffer(StagingBuffer const& stagingBuffer) {
		this->stagingBuffer = stagingBuffer;
	}

	void SyncedCommandQueue::GenerateMips(const bool generating) {
		generateMips = generating;
	}

	namespace TransferCommandManager {
		std::mutex syncQueueMutex{};
		std::mutex singleMutex{};

		std::queue<SyncedCommandQueue*> synchronizedCommandBuffers{};
		std::queue<VkCommandBuffer> commandBuffers{};

		SyncedCommandQueue* BeginCommandQueue() {
			return new SyncedCommandQueue();
		}
		void AddCommand(VkCommandBuffer cmdBuf) {
			singleMutex.lock();
			commandBuffers.push(cmdBuf);
			singleMutex.unlock();
		}
		VkCommandBuffer PullCommand() {
			singleMutex.lock();
			VkCommandBuffer ret = commandBuffers.front();
			commandBuffers.pop();
			singleMutex.unlock();
			return ret;
		}
		void EndCommandQueue(SyncedCommandQueue* cmdQ) {
			syncQueueMutex.lock();
			synchronizedCommandBuffers.push(cmdQ);
			syncQueueMutex.unlock();
		}
		SyncedCommandQueue* PullCommandQueue() {
			syncQueueMutex.lock();
			SyncedCommandQueue* ret = synchronizedCommandBuffers.front();
			synchronizedCommandBuffers.pop();
			syncQueueMutex.unlock();
			return ret;
		}
	}
}