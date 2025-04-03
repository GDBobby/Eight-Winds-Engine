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
#ifndef MAX_MATERIAL_TEXTURE_COUNT
#define MAX_MATERIAL_TEXTURE_COUNT 6
#endif
	namespace Material {
		enum Flags : MaterialFlags {
			//add albedo here eventually, as 1. then it's possible to create a material without albedo but with metal/rough and others
			Albedo = 1,
			Metal = 1 << 1,
			Rough = 1 << 2,
			Normal = 1 << 3,
			Bump = 1 << 4,
			AO = 1 << 5,

			Instanced = 1 << 13,
			Bones = 1 << 14,
		};

		static constexpr uint8_t GetTextureCount(const MaterialFlags flags) {
			const bool hasBumps = flags & Material::Bump;
			const bool hasNormal = flags & Material::Normal;
			const bool hasRough = flags & Material::Rough;
			const bool hasMetal = flags & Material::Metal;
			const bool hasAO = flags & Material::AO;
			const bool hasAlbedo = flags & Material::Albedo;
			//assert(!(hasBones && hasBumps));

			return hasAlbedo + hasNormal + hasRough + hasMetal + hasAO + hasBumps;
		}

		static constexpr uint16_t GetPipeLayoutIndex(const MaterialFlags flags) {
			const bool hasBones = flags & Material::Bones;
			const bool instanced = flags & Material::Instanced;
			//assert(!(hasBones && hasBumps));

			const uint8_t textureCount = GetTextureCount(flags);
			return textureCount + (MAX_MATERIAL_TEXTURE_COUNT * (hasBones + (2 * instanced)));
		}


	}
	//typedef VkDescriptorSet TextureDesc;
#ifndef IMAGE_INVALID
#define IMAGE_INVALID UINT64_MAX
#endif

	typedef uint16_t TransformID;
	typedef uint16_t Compute_TextureID;
	typedef uint32_t SkeletonID;
	typedef uint32_t PipelineID;
	typedef uint64_t ModelID;
	typedef uint64_t ImageID;
	typedef uint8_t SceneKey;


	struct MaterialBuffer {
		glm::vec3 albedo;
		float rough;
		float metal;
		glm::vec3 p_padding; //no do not use
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
		MaterialInfo() {}
		MaterialInfo(MaterialFlags flags, ImageID imageID) : materialFlags{ flags }, imageID{ imageID } {}
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