#pragma once
#include <stdint.h>

#include "vulkan/vulkan.h"

typedef uint8_t MaterialFlags;
typedef uint16_t TextureID;
#ifndef TEXTURE_UNBINDED
#define TEXTURE_UNBINDED UINT16_MAX
#endif
#ifndef MAX_MATERIAL_TEXTURE_COUNT
#define MAX_MATERIAL_TEXTURE_COUNT 6
#endif

typedef uint16_t TransformID;
typedef uint16_t Compute_TextureID;
typedef uint32_t SkeletonID;
typedef uint32_t PipelineID;

struct MaterialTextureInfo {
	MaterialFlags materialFlags;
	TextureID textureID;
	//MaterialTextureInfo() {}
	MaterialTextureInfo() {}
	MaterialTextureInfo(MaterialFlags flags, TextureID texID) : materialFlags{ flags }, textureID{ texID } {}
};

enum MaterialAttributes : uint8_t {
	MaterialF_hasAO = 1,
	MaterialF_hasMetal = 2,
	MaterialF_hasRough = 4,
	MaterialF_hasNormal = 8,
	MaterialF_hasBump = 16,

	MaterialF_instanced = 64,
	MaterialF_hasBones = 128,


	//MaterialF_hasBones = 128, //removed from texture flags
};

struct FrameInfo {
	VkCommandBuffer cmdBuf;
	uint8_t index;
	FrameInfo(VkCommandBuffer cmdBuffer, uint8_t frameIndex) : cmdBuf{ cmdBuffer }, index{ frameIndex } {}
};