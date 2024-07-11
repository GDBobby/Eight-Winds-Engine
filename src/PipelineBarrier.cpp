#include "EWEngine/Graphics/PipelineBarrier.h"

#include <iterator>

namespace EWE {
	void PipelineBarrier::SubmitBarrier(VkCommandBuffer cmdBuf) {
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
}