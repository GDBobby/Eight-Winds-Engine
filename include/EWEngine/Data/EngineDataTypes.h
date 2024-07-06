#pragma once
#include <stdint.h>
#include <cassert>

#include "vulkan/vulkan.h"

typedef uint8_t MaterialFlags;
typedef VkDescriptorSet TextureDesc;
#ifndef TEXTURE_UNBINDED_DESC
#define TEXTURE_UNBINDED_DESC VK_NULL_HANDLE
#endif
#ifndef MAX_MATERIAL_TEXTURE_COUNT
#define MAX_MATERIAL_TEXTURE_COUNT 6
#endif

typedef uint16_t TransformID;
typedef uint16_t Compute_TextureID;
typedef uint32_t SkeletonID;
typedef uint32_t PipelineID;

//struct MappedStagingBuffer {
//	VkBuffer buffer{ VK_NULL_HANDLE };
//	VkDeviceMemory memory{ VK_NULL_HANDLE };
//	void Free(VkDevice device) {
//#ifdef _DEBUG
//		assert(buffer != VK_NULL_HANDLE);
//#endif
//		vkUnmapMemory(device, memory);
//		vkDestroyBuffer(device, buffer, nullptr);
//		vkFreeMemory(device, memory, nullptr);
//	}
//};
struct StagingBuffer {
	VkBuffer buffer{ VK_NULL_HANDLE };
	VkDeviceMemory memory{ VK_NULL_HANDLE };
	void Free(VkDevice device) {
		if (buffer == VK_NULL_HANDLE) {
			return;
		}
		vkDestroyBuffer(device, buffer, nullptr);
		vkFreeMemory(device, memory, nullptr);
	}
};

struct MaterialTextureInfo {
	MaterialFlags materialFlags;
	TextureDesc texture;
	//MaterialTextureInfo() {}
	MaterialTextureInfo() {}
	MaterialTextureInfo(MaterialFlags flags, TextureDesc texID) : materialFlags{ flags }, texture{ texID } {}
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
	FrameInfo() {}
};