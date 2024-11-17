#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"
#include "EWEngine/Graphics/Descriptors.h"
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