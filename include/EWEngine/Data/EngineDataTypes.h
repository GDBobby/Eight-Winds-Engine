#pragma once

#include "vulkan/vulkan.h"

#include <stdint.h>
#include <cassert>
#include <string.h>

namespace EWE {
	typedef uint8_t MaterialFlags;
	//typedef VkDescriptorSet TextureDesc;
#ifndef IMAGE_INVALID
#define IMAGE_INVALID UINT64_MAX
#endif
#ifndef MAX_MATERIAL_TEXTURE_COUNT
#define MAX_MATERIAL_TEXTURE_COUNT 6
#endif

	typedef uint16_t TransformID;
	typedef uint16_t Compute_TextureID;
	typedef uint32_t SkeletonID;
	typedef uint32_t PipelineID;
	typedef uint64_t ModelID;
	typedef uint64_t ImageID;
	typedef uint8_t MaterialCount;


	struct MaterialInfo {
		MaterialFlags materialFlags;
		ImageID imageID;
		MaterialInfo() {}
		MaterialInfo(MaterialFlags flags, ImageID imageID) : materialFlags{ flags }, imageID{ imageID } {}
	};

	namespace Material {
		enum Flags : MaterialCount {
			AO = 1,
			Metal = 1 << 1,
			Rough = 1 << 2,
			Normal = 1 << 3,
			Bump = 1 << 4,

			Instanced = 1 << 6,
			Bones = 1 << 7,
		};
	}
	//replacing MaterialAttrributes with Material::Flags
	enum MaterialAttributes : MaterialCount {
		MaterialF_hasAO = 1,
		MaterialF_hasMetal = 2,
		MaterialF_hasRough = 4,
		MaterialF_hasNormal = 8,
		MaterialF_hasBump = 16,

		MaterialF_instanced = 64,
		MaterialF_hasBones = 128,


		//MaterialF_hasBones = 128, //removed from texture flags
	};
} //namespace EWE