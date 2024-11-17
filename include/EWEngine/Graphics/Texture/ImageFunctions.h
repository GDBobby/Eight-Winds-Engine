#pragma once

#include "EWEngine/Graphics/Texture/Image.h"

namespace EWE {
	namespace Image {
#if USING_VMA
		void CreateImageWithInfo(const VkImageCreateInfo& imageCreateInfo, const VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation vkAlloc);
#else
		void CreateImageWithInfo(const VkImageCreateInfo& imageCreateInfo, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
#endif
		void CopyBufferToImage(CommandBuffer& cmdBuf, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height, uint32_t layerCount);


		//only for transfer -> graphics
		//not for graphics -> graphics 
		//not for transfer -> compute
		void GenerateMipMapsForMultipleImagesTransferQueue(CommandBuffer& cmdBuf, std::vector<ImageInfo>& imageInfo);
		void GenerateMipmaps(CommandBuffer& cmdBuf, ImageInfo& imageInfo, Queue::Enum srcQueue);

#if IMAGE_DEBUGGING
		void CreateImageCommands(ImageInfo& imageInfo, VkImageCreateInfo const& imageCreateInfo, StagingBuffer* stagingBuffer, Queue::Enum queue, bool mipmapping, std::string imageName);
#else
		void CreateImageCommands(ImageInfo& imageInfo, VkImageCreateInfo const& imageCreateInfo, StagingBuffer* stagingBuffer, Queue::Enum queue, bool mipmapping);
#endif

		[[nodiscard("this staging buffer needs to be handled outside of this function")]]
		StagingBuffer* StageImage(PixelPeek& pixelPeek);

		[[nodiscard("this staging buffer needs to be handled outside of this function")]]
		StagingBuffer* StageImage(std::vector<PixelPeek>& pixelPeek);


		VkImageSubresourceRange CreateSubresourceRange(ImageInfo const& imageInfo);


		ImageInfo CreateImage(std::string const& path, bool mipmap, Queue::Enum queue = Queue::transfer);
		ImageInfo CreateImage(PixelPeek& pixelPeek, bool mipmap, Queue::Enum queue = Queue::transfer);

		void Destroy(ImageInfo& imageInfo);
	}
}