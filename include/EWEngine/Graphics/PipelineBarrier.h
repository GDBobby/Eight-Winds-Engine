#pragma once

#include "vulkan/vulkan.h"

#include <vector>

namespace EWE {
    struct PipelineBarrier {
        VkPipelineStageFlagBits srcStageMask{};
        VkPipelineStageFlagBits dstStageMask{};
        VkDependencyFlags dependencyFlags{0};
        std::vector<VkMemoryBarrier> memoryBarriers{};
        std::vector<VkImageMemoryBarrier> imageBarriers{};
        std::vector<VkBufferMemoryBarrier> bufferBarriers{};

		bool Empty() const {
			return (memoryBarriers.size() + imageBarriers.size() + bufferBarriers.size()) == 0;
		}

		void AddBarrier(VkMemoryBarrier& memoryBarrier) {
			memoryBarriers.push_back(memoryBarrier);
		}
		void AddBarrier(VkImageMemoryBarrier& imageBarrier) {
			imageBarriers.push_back(imageBarrier);
		}
		void AddBarrier(VkBufferMemoryBarrier& bufferBarrier) {
			bufferBarriers.push_back(bufferBarrier);
		}
		void SubmitBarrier(VkCommandBuffer cmdBuf) {
			vkCmdPipelineBarrier(cmdBuf,
				srcStageMask, dstStageMask,
				dependencyFlags,
				memoryBarriers.size(), memoryBarriers.data(),
				bufferBarriers.size(), bufferBarriers.data(),
				imageBarriers.size(), imageBarriers.data()
			);
		}
	};
}
