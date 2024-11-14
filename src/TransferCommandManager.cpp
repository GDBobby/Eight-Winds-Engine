#include "EWEngine/Graphics/TransferCommandManager.h"


#include <cassert>
#include <mutex>
#include <iterator>

namespace EWE {

	namespace TransferCommandManager {
		std::mutex callbackMutex{};
		CommandCallbacks commandCallbacks;


		bool Empty(){
#if EWE_DEBUG
			std::lock_guard<std::mutex> lock(callbackMutex);
			if (commandCallbacks.commands.size() == 0) {
				assert(commandCallbacks.mipParamPacks.size() == 0);
				assert(commandCallbacks.pipeBarriers.size() == 0);
				assert(commandCallbacks.stagingBuffers.size() == 0);
			}
#endif
			return commandCallbacks.commands.size() == 0;
		}

		CommandCallbacks PrepareSubmit(){
			std::lock_guard<std::mutex> guard{ callbackMutex };

			return commandCallbacks;
		}


		void AddCommand(CommandBufferData& cmdBuf) {
			EWE_VK(vkEndCommandBuffer, cmdBuf.cmdBuf);
			callbackMutex.lock();

			commandCallbacks.commands.push_back(&cmdBuf);
		}
		void AddPropertyToCommand(StagingBuffer* stagingBuffer) {
			commandCallbacks.stagingBuffers.push_back(stagingBuffer);
		}
		void AddPropertyToCommand(PipelineBarrier& pipeBarrier) {
			commandCallbacks.pipeBarriers.push_back(std::move(pipeBarrier));
		}
		void AddPropertyToCommand(VkImage image, uint8_t mipLevels, uint32_t width, uint32_t height) {
			commandCallbacks.mipParamPacks.emplace_back(image, mipLevels, width, height);
		}
		void FinalizeCommand() {
			callbackMutex.unlock();
		}

	} //namespace TransferCommandManager
} //namespace EWE