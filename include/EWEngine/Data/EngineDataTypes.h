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

//CS_ForwardUp
#define CS_PosXPosY 0
#define CS_NegZNegY 1
#ifndef COORDINATE_SYSTEM
//i dont know how to set this up better
#define COORDINATE_SYSTEM CS_NegZNegY 
#endif

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
	typedef uint8_t SceneKey;


	struct MaterialInfo {
		MaterialFlags materialFlags;
		ImageID imageID;
		MaterialInfo() {}
		MaterialInfo(MaterialFlags flags, ImageID imageID) : materialFlags{ flags }, imageID{ imageID } {}
		bool operator==(MaterialInfo const& other) const {
			return (materialFlags == other.materialFlags) && (imageID == other.imageID);
		}
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


	struct Matrix3ForGLSL {
		std::array<glm::vec4, 3> columns{ glm::vec4{0.f}, glm::vec4{0.f}, glm::vec4{0.f, 0.f, 1.f, 0.f} };
		Matrix3ForGLSL() {}
		Matrix3ForGLSL(glm::mat3 const& inMat);
		
	};
} //namespace EWE