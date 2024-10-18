#include "EWEngine/Graphics/PipelineBarrier.h"

#include <iterator>

namespace EWE {
	void PipelineBarrier::Submit(VkCommandBuffer cmdBuf) const {
		vkCmdPipelineBarrier(cmdBuf,
			srcStageMask, dstStageMask,
			dependencyFlags,
			memoryBarriers.size(), memoryBarriers.data(),
			bufferBarriers.size(), bufferBarriers.data(),
			imageBarriers.size(), imageBarriers.data()
		);
	}
	//the parameter object passed in is no longer usable, submitting both barriers will potentially lead to errors
	void PipelineBarrier::Merge(PipelineBarrier const& other) {
		std::copy(other.memoryBarriers.begin(), other.memoryBarriers.end(), std::back_inserter(memoryBarriers));
		std::copy(other.bufferBarriers.begin(), other.bufferBarriers.end(), std::back_inserter(bufferBarriers));
		std::copy(other.imageBarriers.begin(), other.imageBarriers.end(), std::back_inserter(imageBarriers));
	}

	namespace Barrier {
		VkImageMemoryBarrier ChangeImageLayout(
			const VkImage image,
			const VkImageLayout oldImageLayout,
			const VkImageLayout newImageLayout,
			VkImageSubresourceRange const& subresourceRange
		) {
			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			if ((oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
				imageMemoryBarrier.srcAccessMask = 0;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			else if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			}
			else {
				assert(false && "unsupported layout transition");
			}

			return imageMemoryBarrier;
		}
	}//namespace Barrier
} //namespace EWE