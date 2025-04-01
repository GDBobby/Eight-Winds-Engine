#pragma once

#include "vulkan/vulkan.h"

#include <stdint.h>
#include <cassert>
#include <string.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

//#define COORDINATE_SYSTEM CS_NegZNegY 

namespace EWE {
	typedef uint16_t MaterialFlags;
	namespace Material {
		enum Flags : MaterialFlags {
			AO = 1,
			Metal = 1 << 1,
			Rough = 1 << 2,
			Normal = 1 << 3,
			Bump = 1 << 4,

			Instanced = 1 << 13,
			Bones = 1 << 14,
			no_texture = 1 << 15,
		};
	}
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
	typedef uint8_t SceneKey;


	struct MaterialBuffer {
		glm::vec4 albedo;
		float rough;
		float metal;
		//sub surface scattering
		//depth
		//transparency
		//emission
		//specular
		//specular tint
		//sheen
		//clear coat
	};
	struct MaterialInfo {
		MaterialFlags materialFlags;
		ImageID imageID;
		MaterialBuffer* materialBuffer;
		MaterialInfo() {}
		MaterialInfo(MaterialFlags flags, ImageID imageID) : materialFlags{ flags }, imageID{ imageID }, materialBuffer{ nullptr } {}
		MaterialInfo(MaterialFlags flags, ImageID imageID, MaterialBuffer* matBuffer) : materialFlags{ flags }, imageID{ imageID }, materialBuffer{ matBuffer } {}
		bool operator==(MaterialInfo const& other) const {
			return (materialFlags == other.materialFlags) && (imageID == other.imageID);
		}
	};


	struct Matrix3ForGLSL {
		std::array<glm::vec4, 3> columns{ glm::vec4{0.f}, glm::vec4{0.f}, glm::vec4{0.f, 0.f, 1.f, 0.f} };
		Matrix3ForGLSL() {}
		Matrix3ForGLSL(glm::mat3 const& inMat);
		
	};
} //namespace EWE