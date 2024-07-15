#pragma once
#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Descriptors.h"
#include "EWEngine/Graphics/Device_Buffer.h"
#include "EWEngine/Data/EngineDataTypes.h"
#include "EWEngine/Data/EWE_Utils.h"

#include <stb/stb_image.h>

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
		void CreateImageWithInfo(const VkImageCreateInfo& imageInfo, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	}


	struct PixelPeek {
		stbi_uc* pixels{ nullptr };
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
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkSampler sampler;
		uint8_t mipLevels{ 1 };
		uint8_t arrayLayers{1};
		VkDescriptorImageInfo descriptorImageInfo;

		VkImageSubresourceRange CreateSubresourceRange() {
			VkImageSubresourceRange subresourceRange{};

			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = mipLevels;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.layerCount = arrayLayers;

			return subresourceRange;
		}

		[[nodiscard("this staging buffer needs to be handled outside of this function")]]
		static StagingBuffer* StageImage(PixelPeek& pixelPeek);

		[[nodiscard("this staging buffer needs to be handled outside of this function")]]
		static StagingBuffer* StageImage(std::vector<PixelPeek>& pixelPeek);

		void CreateTextureImage(Queue::Enum queue, PixelPeek& pixelPeek, bool mipmapping = true);
		void CreateImageCommands(VkImageCreateInfo const& imageCreateInfo, StagingBuffer* stagingBuffer, Queue::Enum queue, bool mipmapping);

		void CreateTextureImageView();

		void CreateTextureSampler();

		//(&ImageInfo::GenerateMipmaps, &imageInfo, format, width, height, srcQueue)
		void GenerateMipmaps(const VkFormat imageFormat, int width, int height, Queue::Enum srcQueue);
		static void GenerateMipmaps(VkImage image, uint8_t mipLevels, const VkFormat imageFormat, int width, int height, Queue::Enum srcQueue);

		//ImageQueueTransitionData GenerateTransitionData(uint32_t queueIndex, StagingBuffer stagingBuffer){
		//	return ImageQueueTransitionData{image, mipLevels, arrayLayers, queueIndex, stagingBuffer};
		//}
	private: 
		void GenerateMipmaps(VkCommandBuffer cmdBuf, const VkFormat imageFormat, int width, int height, Queue::Enum srcQueue);

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