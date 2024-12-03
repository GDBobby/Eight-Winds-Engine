#include "EWEngine/Graphics/TransferCommandManager.h"


#include <cassert>
#include <mutex>
#include <iterator>

namespace EWE {

	namespace TransferCommandManager {
		std::mutex callbackMutex{};
		TransferCommandCallbacks commandCallbacks;


		bool Empty(){
#if EWE_DEBUG
			std::lock_guard<std::mutex> lock(callbackMutex);
			if (commandCallbacks.commands.size() == 0) {
				assert(commandCallbacks.images.size() == 0);
				assert(commandCallbacks.pipeBarriers.size() == 0);
				assert(commandCallbacks.stagingBuffers.size() == 0);
			}
#endif
			return commandCallbacks.commands.size() == 0;
		}

		TransferCommandCallbacks PrepareSubmit(){
			std::lock_guard<std::mutex> guard{ callbackMutex };

			return commandCallbacks;
		}


		void AddCommand(CommandBuffer& cmdBuf) {
			VK::Object->poolMutex[Queue::transfer].lock();
			EWE_VK(vkEndCommandBuffer, cmdBuf);
			VK::Object->poolMutex[Queue::transfer].unlock();
			callbackMutex.lock();

			commandCallbacks.commands.push_back(&cmdBuf);
		}
		void AddPropertyToCommand(StagingBuffer* stagingBuffer) {
			commandCallbacks.stagingBuffers.push_back(stagingBuffer);
		}
		void AddPropertyToCommand(PipelineBarrier& pipeBarrier) {
			commandCallbacks.pipeBarriers.push_back(std::move(pipeBarrier));
		}
		void AddPropertyToCommand(VkImageLayout* imageLayout) {
			commandCallbacks.imageLayouts.push_back(imageLayout);
		}
		void AddPropertyToCommand(ImageInfo* imageInfo) {
			commandCallbacks.images.push_back(imageInfo);
		}
		void FinalizeCommand() {
			callbackMutex.unlock();
		}

	} //namespace TransferCommandManager
} //namespace EWE