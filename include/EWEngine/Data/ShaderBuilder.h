#pragma once
#include <glslang/SPIRV/GlslangToSpv.h>

#include <stdint.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <ios>

#include "EngineDataTypes.h"



#define USING_TANGENT_SPACE false
#define DEBUGGING_SHADERS true


#define SHADER_DYNAMIC_PATH "shaders\\dynamic\\"

class ShaderBlock {
public:

	static void BatchCreateFragmentShader(std::vector<MaterialFlags> flagVector);
	static std::vector<uint32_t> getFragmentShader(MaterialFlags flags, bool hasBones);
	static std::vector<uint32_t> getVertexShader(bool hasNormal, uint16_t boneCount, bool instanced, bool largeInstance = true);
	static std::vector<uint32_t> getLoadingVertShader();
	static std::vector<uint32_t> getLoadingFragShader();
private:
	struct SpirvHelper {
		static void InitResources(TBuiltInResource& Resources) {
			Resources.maxLights = 32;
			Resources.maxClipPlanes = 6;
			Resources.maxTextureUnits = 32;
			Resources.maxTextureCoords = 32;
			Resources.maxVertexAttribs = 64;
			Resources.maxVertexUniformComponents = 4096;
			Resources.maxVaryingFloats = 64;
			Resources.maxVertexTextureImageUnits = 32;
			Resources.maxCombinedTextureImageUnits = 80;
			Resources.maxTextureImageUnits = 32;
			Resources.maxFragmentUniformComponents = 4096;
			Resources.maxDrawBuffers = 32;
			Resources.maxVertexUniformVectors = 128;
			Resources.maxVaryingVectors = 8;
			Resources.maxFragmentUniformVectors = 16;
			Resources.maxVertexOutputVectors = 16;
			Resources.maxFragmentInputVectors = 15;
			Resources.minProgramTexelOffset = -8;
			Resources.maxProgramTexelOffset = 7;
			Resources.maxClipDistances = 8;
			Resources.maxComputeWorkGroupCountX = 65535;
			Resources.maxComputeWorkGroupCountY = 65535;
			Resources.maxComputeWorkGroupCountZ = 65535;
			Resources.maxComputeWorkGroupSizeX = 1024;
			Resources.maxComputeWorkGroupSizeY = 1024;
			Resources.maxComputeWorkGroupSizeZ = 64;
			Resources.maxComputeUniformComponents = 1024;
			Resources.maxComputeTextureImageUnits = 16;
			Resources.maxComputeImageUniforms = 8;
			Resources.maxComputeAtomicCounters = 8;
			Resources.maxComputeAtomicCounterBuffers = 1;
			Resources.maxVaryingComponents = 60;
			Resources.maxVertexOutputComponents = 64;
			Resources.maxGeometryInputComponents = 64;
			Resources.maxGeometryOutputComponents = 128;
			Resources.maxFragmentInputComponents = 128;
			Resources.maxImageUnits = 8;
			Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
			Resources.maxCombinedShaderOutputResources = 8;
			Resources.maxImageSamples = 0;
			Resources.maxVertexImageUniforms = 0;
			Resources.maxTessControlImageUniforms = 0;
			Resources.maxTessEvaluationImageUniforms = 0;
			Resources.maxGeometryImageUniforms = 0;
			Resources.maxFragmentImageUniforms = 8;
			Resources.maxCombinedImageUniforms = 8;
			Resources.maxGeometryTextureImageUnits = 16;
			Resources.maxGeometryOutputVertices = 256;
			Resources.maxGeometryTotalOutputComponents = 1024;
			Resources.maxGeometryUniformComponents = 1024;
			Resources.maxGeometryVaryingComponents = 64;
			Resources.maxTessControlInputComponents = 128;
			Resources.maxTessControlOutputComponents = 128;
			Resources.maxTessControlTextureImageUnits = 16;
			Resources.maxTessControlUniformComponents = 1024;
			Resources.maxTessControlTotalOutputComponents = 4096;
			Resources.maxTessEvaluationInputComponents = 128;
			Resources.maxTessEvaluationOutputComponents = 128;
			Resources.maxTessEvaluationTextureImageUnits = 16;
			Resources.maxTessEvaluationUniformComponents = 1024;
			Resources.maxTessPatchComponents = 120;
			Resources.maxPatchVertices = 32;
			Resources.maxTessGenLevel = 64;
			Resources.maxViewports = 16;
			Resources.maxVertexAtomicCounters = 0;
			Resources.maxTessControlAtomicCounters = 0;
			Resources.maxTessEvaluationAtomicCounters = 0;
			Resources.maxGeometryAtomicCounters = 0;
			Resources.maxFragmentAtomicCounters = 8;
			Resources.maxCombinedAtomicCounters = 8;
			Resources.maxAtomicCounterBindings = 1;
			Resources.maxVertexAtomicCounterBuffers = 0;
			Resources.maxTessControlAtomicCounterBuffers = 0;
			Resources.maxTessEvaluationAtomicCounterBuffers = 0;
			Resources.maxGeometryAtomicCounterBuffers = 0;
			Resources.maxFragmentAtomicCounterBuffers = 1;
			Resources.maxCombinedAtomicCounterBuffers = 1;
			Resources.maxAtomicCounterBufferSize = 16384;
			Resources.maxTransformFeedbackBuffers = 4;
			Resources.maxTransformFeedbackInterleavedComponents = 64;
			Resources.maxCullDistances = 8;
			Resources.maxCombinedClipAndCullDistances = 8;
			Resources.maxSamples = 4;
			Resources.maxMeshOutputVerticesNV = 256;
			Resources.maxMeshOutputPrimitivesNV = 512;
			Resources.maxMeshWorkGroupSizeX_NV = 32;
			Resources.maxMeshWorkGroupSizeY_NV = 1;
			Resources.maxMeshWorkGroupSizeZ_NV = 1;
			Resources.maxTaskWorkGroupSizeX_NV = 32;
			Resources.maxTaskWorkGroupSizeY_NV = 1;
			Resources.maxTaskWorkGroupSizeZ_NV = 1;
			Resources.maxMeshViewCountNV = 4;
			Resources.limits.nonInductiveForLoops = 1;
			Resources.limits.whileLoops = 1;
			Resources.limits.doWhileLoops = 1;
			Resources.limits.generalUniformIndexing = 1;
			Resources.limits.generalAttributeMatrixVectorIndexing = 1;
			Resources.limits.generalVaryingIndexing = 1;
			Resources.limits.generalSamplerIndexing = 1;
			Resources.limits.generalVariableIndexing = 1;
			Resources.limits.generalConstantMatrixVectorIndexing = 1;
		}

		static bool BuildFlaggedFrag(MaterialFlags flags, bool hasBones, std::vector<unsigned int>& spirv);
		static bool BuildFlaggedVert(bool hasNormal, uint16_t boneCount, bool instanced, std::vector<unsigned int>& spirv, bool largeInstance); //currently, ALWAYS has bones
		static bool LoadingVertSPV(std::vector<unsigned int>& spirv);
		static bool LoadingFragSPV(std::vector<unsigned int>& spirv);
	};
	static std::string buildFragmentShader(MaterialFlags flags, bool hasBones);

	static std::string buildVertexShader(bool hasNormal, uint16_t boneCount, bool instanced, bool largeInstance);

	static void addBindings(std::string& shaderString, bool hasNormal, bool hasRough, bool hasMetal, bool hasAO, bool hasBumps, bool hasBones);
};


