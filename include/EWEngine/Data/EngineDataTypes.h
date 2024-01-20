#pragma once
#include <stdint.h>

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
	MaterialTextureInfo(MaterialFlags flags, TextureID texID) : materialFlags{ flags }, textureID{ texID } {}
};

enum DynamicFlags : uint8_t {
	DynF_hasAO = 1,
	DynF_hasMetal = 2,
	DynF_hasRough = 4,
	DynF_hasNormal = 8,
	DynF_hasBump = 16,


	//DynF_hasBones = 128, //removed from texture flags
};