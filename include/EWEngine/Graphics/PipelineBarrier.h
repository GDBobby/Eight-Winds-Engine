#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"

#include <vector>

namespace EWE {
    struct PipelineBarrier {
        VkPipelineStageFlagBits srcStageMask;
        VkPipelineStageFlagBits dstStageMask;
        VkDependencyFlags dependencyFlags;
        std::vector<VkMemoryBarrier> memoryBarriers;
        std::vector<VkImageMemoryBarrier> imageBarriers;
        std::vector<VkBufferMemoryBarrier> bufferBarriers;

		PipelineBarrier();
		PipelineBarrier(PipelineBarrier& copySource) noexcept;
		PipelineBarrier& operator=(PipelineBarrier& copySource);
		PipelineBarrier(PipelineBarrier&& moveSource) noexcept;
		PipelineBarrier& operator=(PipelineBarrier&& moveSource);

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
		void Submit(VkCommandBuffer cmdBuf) const;

		//the parameter object passed in is no longer usable, submitting both barriers will potentially lead to errors
		void Merge(PipelineBarrier const& other);

		static void SimplifyVector(std::vector<PipelineBarrier>& barriers);
	};
	namespace Barrier {
		//this only changes the src/dst access mask
		VkImageMemoryBarrier ChangeImageLayout(
			const VkImage image, 
			const VkImageLayout oldImageLayout, 
			const VkImageLayout newImageLayout, 
			VkImageSubresourceRange const& subresourceRange
		);
		VkImageMemoryBarrier TransitionImageLayout(VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint8_t layerCount = 1);
		void TransitionImageLayoutWithBarrier(VkCommandBuffer cmdBuf, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImage& image, VkImageLayout srcLayout, VkImageLayout dstLayout, uint32_t mipLevels, uint8_t layerCount);
	} //namespace Barrier

	//indirectly tied to barriers
	struct MipParamPack {
		VkImage image;
		uint8_t mipLevels;
		uint32_t width;
		uint32_t height;
		//add depth and array layers here later if necessary
		MipParamPack(VkImage image, uint8_t mipLevels, uint32_t width, uint32_t height) : image{ image }, mipLevels{ mipLevels }, width{ width }, height{ height } {}
	};
	namespace Image {
		//only for transfer -> graphics
		//not for graphics -> graphics 
		//not for transfer -> compute
		void GenerateMipMapsForMultipleImages(VkCommandBuffer cmdBuf, std::vector<MipParamPack>& mipParamPack);
		void GenerateMipmaps(VkCommandBuffer cmdBuf, MipParamPack mipParamPack);
		void GenerateMipmaps(VkCommandBuffer cmdBuf, VkImage image, uint8_t mipLevels, uint32_t width, uint32_t height);
	}
} //namespace EWE
