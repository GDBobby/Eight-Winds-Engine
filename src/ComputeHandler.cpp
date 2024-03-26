/*
#include "Compute/ComputeHandler.h"

namespace EWE {
	ComputeHandler::ComputeHandler() : syncHub{ SyncHub::getSyncHubInstance()} {
		createCommandBuffers();


	}

	VkCommandBuffer ComputeHandler::beginCommandBuffer() {
		computeStarted = !computeStarted;
		assert(computeStarted);
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			std::cout << "failed to begin compute command buffer " << std::endl;
			throw std::runtime_error("failed to begin compute command buffer!");
		}

		return commandBuffer;
	}

	void ComputeHandler::endCommandBuffer() {

		assert(computeStarted);
		{
			VkResult vkResult = vkEndCommandBuffer(commandBuffer);
			if (vkResult != VK_SUCCESS) {
				std::cout << "failed to record command buffer: " << vkResult << std::endl;
				throw std::runtime_error("failed to record command buffer");
			}
		}

		{
			std::cout << "immediately before queue submit in compute handler \n";
			VkResult vkResult = vkQueueSubmit(device.computeQueue(), 1, &computeSubmitInfo, VK_NULL_HANDLE);
			std::cout << "immediately after queue submit in compute handler \n";
			if (vkResult != VK_SUCCESS) {
				std::cout << "failed to submit compute command buffer: " << vkResult << std::endl;
				throw std::runtime_error("failed to submit compute command buffer");
			}
		}
		vkResetCommandBuffer(commandBuffer, 0);
		computeStarted = false;
	}
}
*/