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

		static StagingBuffer StageImage(PixelPeek& pixelPeek);
		static StagingBuffer StageImage(std::vector<PixelPeek>& pixelPeek);

		[[nodiscard("this staging buffer needs to be handled outside of this function")]] 
		StagingBuffer CreateTextureImage(Queue::Enum whichQueue, PixelPeek& pixelPeek, bool mipmapping = true);

		void CreateTextureImageView();

		void CreateTextureSampler();

		void GenerateMipmaps(Queue::Enum whichQueue, const VkFormat imageFormat, int width, int height);

		ImageQueueTransitionData GenerateTransitionData(uint32_t queueIndex){
			return ImageQueueTransitionData{image, mipLevels, arrayLayers, queueIndex};
		}
	private: 
		void GenerateMipmaps(VkCommandBuffer cmdBuf, const VkFormat imageFormat, int width, int height);

	public:
		VkDescriptorImageInfo* GetDescriptorImageInfo() {
			return &descriptorImageInfo;
		}
		ImageInfo(PixelPeek& pixelPeek, bool mipmap, Queue::Enum whichQueue = Queue::transfer);
		ImageInfo(std::string const& path, bool mipmap, Queue::Enum whichQueue = Queue::transfer);

		[[nodiscard("this StagingBuffer needs to be handled outside of this function")]]
		StagingBuffer Initialize(PixelPeek& pixelPeek, bool mipmap, Queue::Enum whichQueue);

		[[nodiscard("this StagingBuffer needs to be handled outside of this function")]]
		StagingBuffer Initialize(std::string const& path, bool mipmap, Queue::Enum whichQueue);
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
}

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