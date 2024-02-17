#pragma once
//#include <glslang/SPIRV/GlslangToSpv.h>


#include <notvulkan/glslang/glslang/Public/ShaderLang.h>
#include <notvulkan/glslang/glslang/Include/intermediate.h>
//we're specifically not using the glslang included by Vulkan to avoid build conflicts. Specifically, 
#include <notvulkan/glslang/SPIRV/GlslangToSpv.h>

#include <stdint.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <ios>

#include "EngineDataTypes.h"



#define USING_TANGENT_SPACE false
#define DEBUGGING_SHADERS true


#define SHADER_DYNAMIC_PATH "shaders/dynamic/"

class ShaderBlock {
public:

	//static void BatchCreateFragmentShader(std::vector<MaterialFlags> flagVector);
	static std::vector<uint32_t> getFragmentShader(MaterialFlags flags, bool hasBones);
	static std::vector<uint32_t> getVertexShader(bool hasNormal, uint16_t boneCount, bool instanced, bool largeInstance = true);
	static std::vector<uint32_t> getLoadingVertShader();
	static std::vector<uint32_t> getLoadingFragShader();
private:
	struct SpirvHelper {
		static void InitResources(TBuiltInResource& Resources);

		static bool BuildFlaggedFrag(MaterialFlags flags, bool hasBones, std::vector<unsigned int>& spirv);
		static bool BuildFlaggedVert(bool hasNormal, uint16_t boneCount, bool instanced, std::vector<unsigned int>& spirv, bool largeInstance); //currently, ALWAYS has bones
		static bool LoadingVertSPV(std::vector<unsigned int>& spirv);
		static bool LoadingFragSPV(std::vector<unsigned int>& spirv);
	};
	static std::vector<const char*> buildFragmentShader(MaterialFlags flags, bool hasBones);

	static std::string buildVertexShader(bool hasNormal, uint16_t boneCount, bool instanced, bool largeInstance);

	static void addBindings(std::vector<const char*>& retVec, bool hasNormal, bool hasRough, bool hasMetal, bool hasAO, bool hasBumps, bool hasBones);

	static void getNumberAsCString(std::vector<const char*>& retVec, uint8_t number);
	static char* buffers;
	static uint8_t buffersUsed;
};


