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

#include <EWEngine/Graphics/Preprocessor.h>

#include "EngineDataTypes.h"



#define USING_TANGENT_SPACE false
#define DEBUGGING_SHADERS true


#define SHADER_DYNAMIC_PATH "shaders/materials/"

namespace EWE {
	namespace ShaderBlock {

		void InitializeGlslang();
		void FinalizeGlslang();

		//static void BatchCreateFragmentShader(std::vector<MaterialFlags> flagVector);
		std::vector<uint32_t> GetFragmentShader(MaterialFlags flags, bool hasBones);
		std::vector<uint32_t> GetVertexShader(bool hasNormal, uint16_t boneCount, bool instanced, bool largeInstance = true);
		std::vector<uint32_t> GetLoadingVertShader();
		std::vector<uint32_t> GetLoadingFragShader();
	};
} //namespace EWE