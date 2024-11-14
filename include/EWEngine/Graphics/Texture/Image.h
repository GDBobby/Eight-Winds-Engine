#pragma once
#include "EWEngine/Graphics/VulkanHeader.h"

#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Descriptors.h"
#include "EWEngine/Graphics/Device_Buffer.h"
#include "EWEngine/Data/EWE_Utils.h"


#include <unordered_map>
#include <map>

/*
~EWETexture() {
	printf("texture destruction \n");
	//why was this not already here
	// nvm taking it back out, can't figure out how to stop textures from getting moved in the vectors
	destroy();
}
*/

#define SUPPORTED_STAGE_COUNT 9

namespace EWE {
	namespace Image {
#if USING_VMA
		void CreateImageWithInfo(const VkImageCreateInfo& imageCreateInfo, const VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation vkAlloc);
#else
		void CreateImageWithInfo(const VkImageCreateInfo& imageCreateInfo, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
#endif
	}


	struct PixelPeek {
		void* pixels{ nullptr };
		int width;
		int height;
		int channels;

#if DEBUG_NAMING
		const std::string debugName;
#endif

		PixelPeek() {}
		PixelPeek(std::string const& path);
		//dont currrently care about desired channels, but maybe one day
	};
	struct ImageInfo {
		VkImageLayout imageLayout;
		VkImage image;
#if USING_VMA
		VmaAllocation memory;
#else
		VkDeviceMemory memory;
#endif
		VkImageView imageView;
		VkSampler sampler;
		uint8_t mipLevels{ 1 };
		uint8_t arrayLayers{1};
		VkDescriptorImageInfo descriptorImageInfo;

		VkImageSubresourceRange CreateSubresourceRange();

		[[nodiscard("this staging buffer needs to be handled outside of this function")]]
		static StagingBuffer* StageImage(PixelPeek& pixelPeek);

		[[nodiscard("this staging buffer needs to be handled outside of this function")]]
		static StagingBuffer* StageImage(std::vector<PixelPeek>& pixelPeek);

		void CreateTextureImage(Queue::Enum queue, PixelPeek& pixelPeek, bool mipmapping = true);
#if IMAGE_DEBUGGING
		void CreateImageCommands(VkImageCreateInfo const& imageCreateInfo, StagingBuffer* stagingBuffer, Queue::Enum queue, bool mipmapping, std::string imageName);
#else
		void CreateImageCommands(VkImageCreateInfo const& imageCreateInfo, StagingBuffer* stagingBuffer, Queue::Enum queue, bool mipmapping);
#endif

		void CreateTextureImageView();

		void CreateTextureSampler();

		//(&ImageInfo::GenerateMipmaps, &imageInfo, format, width, height, srcQueue)
		void GenerateMipmaps(const VkFormat imageFormat, uint32_t width, uint32_t height, Queue::Enum srcQueue);

		//ImageQueueTransitionData GenerateTransitionData(uint32_t queueIndex, StagingBuffer stagingBuffer){
		//	return ImageQueueTransitionData{image, mipLevels, arrayLayers, queueIndex, stagingBuffer};
		//}
	private: 
		void GenerateMipmaps(VkCommandBuffer cmdBuf, const VkFormat imageFormat, uint32_t width, uint32_t height, Queue::Enum srcQueue);

	public:
		VkDescriptorImageInfo* GetDescriptorImageInfo() {
			return &descriptorImageInfo;
		}
		ImageInfo(PixelPeek& pixelPeek, bool mipmap, Queue::Enum queue = Queue::transfer);
		ImageInfo(std::string const& path, bool mipmap, Queue::Enum queue = Queue::transfer);

		void Initialize(std::string const& path, bool mipmap, Queue::Enum queue);
		void Initialize(PixelPeek& pixelPeek, bool mipmap, Queue::Enum queue);

		ImageInfo() {}
		void Destroy();
	};


	struct TextureDSLInfo {

		uint8_t stageCounts[SUPPORTED_STAGE_COUNT] = { 0,0,0,0, 0,0,0,0, 0 };
		void SetStageTextureCount(VkShaderStageFlags stageFlag, uint8_t textureCount);

		EWEDescriptorSetLayout* BuildDSL();

		TextureDSLInfo() {}
		EWEDescriptorSetLayout* GetDescSetLayout();

		static std::unordered_map<TextureDSLInfo, EWEDescriptorSetLayout*> descSetLayouts;
		static EWEDescriptorSetLayout* GetSimpleDSL(VkShaderStageFlags stageFlag);

		bool operator==(TextureDSLInfo const& other) const {
			for (int i = 0; i < SUPPORTED_STAGE_COUNT; i++) {
				if (stageCounts[i] != other.stageCounts[i]) {
					return false;
				}
			}
			return true;
		}
	};
} //namespace EWE

template<>
struct std::hash<EWE::TextureDSLInfo> {
	size_t operator()(EWE::TextureDSLInfo const& dslToHash) const {
		size_t seed = 0;

		for (uint8_t i = 0; i < SUPPORTED_STAGE_COUNT; i++) {
			EWE::HashCombine(seed, dslToHash.stageCounts[i]);
		}
		return seed;
	}
};