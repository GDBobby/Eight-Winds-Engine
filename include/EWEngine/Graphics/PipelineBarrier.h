#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"

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

		void AddBarrier(VkMemoryBarrier const& memoryBarrier) {
			memoryBarriers.push_back(memoryBarrier);
		}
		void AddBarrier(VkImageMemoryBarrier const& imageBarrier) {
			imageBarriers.push_back(imageBarrier);
		}
		void AddBarrier(VkBufferMemoryBarrier const& bufferBarrier) {
			bufferBarriers.push_back(bufferBarrier);
		}
		void SubmitBarrier(VkCommandBuffer cmdBuf);

		//the parameter object passed in is no longer usable, submitting both barriers will potentially lead to errors
		void Merge(PipelineBarrier const& other);
	};
}
