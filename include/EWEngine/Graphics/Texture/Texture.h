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

		void CreateTextureImage(PixelPeek& pixelPeek, bool mipmapping = true);
		void CreateTextureImage(VkCommandBuffer cmdBuf, PixelPeek& pixelPeek, bool mipmapping = true);

		void CreateTextureImageView();

		void CreateTextureSampler();

		void GenerateMipmaps(const VkFormat imageFormat, const int width, int height);
		void GenerateMipmaps(VkCommandBuffer cmdBuf, const VkFormat imageFormat, int width, int height);

		ImageQueueTransitionData GenerateTransitionData(uint32_t queueIndex){
			return ImageQueueTransitionData{image, mipLevels, arrayLayers, queueIndex};
		}

	public:
		VkDescriptorImageInfo* GetDescriptorImageInfo() {
			return &descriptorImageInfo;
		}
		ImageInfo(PixelPeek& pixelPeek, bool mipmap);
		ImageInfo(VkCommandBuffer cmdBuf, PixelPeek& pixelPeek, bool mipmap);
		ImageInfo(std::string const& path, bool mipmap);
		ImageInfo(VkCommandBuffer cmdBuf, std::string const& path, bool mipmap);
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