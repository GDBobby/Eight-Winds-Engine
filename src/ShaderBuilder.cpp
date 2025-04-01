#include "EWEngine/Data/ShaderBuilder.h"
#include "EWEngine/Data/FragmentShaderText.h"
#include "EWEngine/Data/VertexShaderText.h"
#include "EWEngine/resources/LoadingString.h"

#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <mutex>
//#include <chrono>

//open in vscode if having intellisense issues in Visual Studio
#define SHADER_VERSION_ID "1.0.1_"

namespace EWE {
	bool glslangInitialized = false;

	namespace ShaderBlock {
		void InitializeGlslang() {
			glslangInitialized = true;
			glslang::InitializeProcess();

		}
		void FinalizeGlslang() {
			glslangInitialized = false;
			glslang::FinalizeProcess();
		}
	} //namespace ShaderBlock

	void AddBindings(std::string& retBuf, bool hasAlbedo, bool hasNormal, bool hasRough, bool hasMetal, bool hasAO, bool hasBumps, bool hasBones, bool instanced) {
		uint8_t currentBinding = 2 + instanced + hasBones;

		if (instanced) {
			retBuf += FragmentShaderText::materialBufferInstancedPartOne;
			retBuf += FragmentShaderText::materialBufferInstancedPartTwo;
			retBuf += std::to_string(currentBinding);
			retBuf += FragmentShaderText::materialBufferInstancedPartThree;
		}
		else{
			retBuf += FragmentShaderText::firstHalfBinding;
			retBuf += std::to_string(currentBinding);
			for (auto const& tempStr : FragmentShaderText::MBOSecondHalf) {
				retBuf += tempStr;
			}
		}
		currentBinding++;

		if (hasAlbedo) {
			//shaderString += firstHalfBinding[hasBones];
			retBuf += FragmentShaderText::firstHalfBinding;
			//shaderString += std::to_string(currentBinding);
			//retVec.emplace_back(std::to_string(currentBinding).c_str());

			retBuf += std::to_string(currentBinding);
			retBuf += FragmentShaderText::secondHalfBinding;

			uint8_t texIndex = hasBumps;

			retBuf += "const int albedoIndex = ";
			retBuf += std::to_string(texIndex++);
			retBuf += ';';


			if (hasNormal) {
				retBuf += "const int normalIndex = ";
				retBuf += std::to_string(texIndex++);
				retBuf += ';';
			}
			if (hasRough) {
				retBuf += "const int roughIndex = ";
				retBuf += std::to_string(texIndex++);
				retBuf += ';';
			}

			if (hasMetal) {
				retBuf += "const int metalIndex = ";
				retBuf += std::to_string(texIndex++);
				retBuf += ';';
			}
			if (hasAO) {
				retBuf += "const int aoIndex = ";
				retBuf += std::to_string(texIndex++);
				retBuf += ';';
			}
		}
	}

	std::string BuildFragmentShader(MaterialFlags flags, bool hasBones) {

		std::string retBuf{};

		bool instanced = flags & Material::Instanced;
		bool hasBumps = flags & Material::Bump;
		bool hasNormal = flags & Material::Normal;
		bool hasRough = flags & Material::Rough;
		bool hasMetal = flags & Material::Metal;
		bool hasAO = flags & Material::AO;

		if (!hasBumps) {
			for (int i = 0; i < FragmentShaderText::fragNNEntry.size(); i++) {
				retBuf += FragmentShaderText::fragNNEntry[i];
			}
			if (hasNormal) {
				retBuf += "layout (location = 3) in vec3 fragTangentWorld;";
				if (instanced) {
					retBuf += "layout (location = 4) in float instanceIndex;";
				}
			}
			else if (instanced) {
				retBuf += "layout (location = 3) in float instanceIndex;";
			}
			retBuf += FragmentShaderText::fragExit;

			for (int i = 0; i < FragmentShaderText::dataBindings.size(); i++) {
				retBuf += FragmentShaderText::dataBindings[i];
			}
			for (int i = 0; i < FragmentShaderText::functionBlock.size(); i++) {
				retBuf += FragmentShaderText::functionBlock[i];
			}

			AddBindings(retBuf, flags != Material::no_texture, hasNormal, hasRough, hasMetal, hasAO, hasBumps, hasBones, instanced);

			if (hasNormal) {
				for (int i = 0; i < FragmentShaderText::calcNormalFunction.size(); i++) {
					retBuf += FragmentShaderText::calcNormalFunction[i];
				}
			}

			for (int i = 0; i < FragmentShaderText::mainEntryBlock[0].size(); i++) {
				retBuf += FragmentShaderText::mainEntryBlock[0][i];
			}
			for (int i = 0; i < FragmentShaderText::mainSecondBlockNN.size(); i++) {
				retBuf += FragmentShaderText::mainSecondBlockNN[i];
			}
			if (hasNormal) {
				retBuf += "vec3 normal = calculateNormal();";
			}
			else {
				retBuf += "vec3 normal = normalize(fragNormalWorld);";
			}

			bool initializedInstanceIndex = false;
			if (hasRough) {
				retBuf += "float roughness = texture(materialTextures, vec3(fragTexCoord, roughIndex)).r;";
			}
			else {
				if (instanced) {
					retBuf += "int instanceIndexInt = int(instanceIndex);";
					initializedInstanceIndex = true;
					retBuf += "float roughness = mbo[instanceIndexInt].rough;";
				}
				else {
					retBuf += "float roughness = mbo.rough;";
				}
			}
			if (hasMetal) {
				retBuf += "float metal = texture(materialTextures, vec3(fragTexCoord, metalIndex)).r;";
			}
			else {
				if (instanced) {
					if (!initializedInstanceIndex) {
						retBuf += "int instanceIndexInt = int(instanceIndex);";
					}
					retBuf += "float metal = mbo[instanceIndexInt].metal;";
				}
				else {
					retBuf += "float metal = mbo.metal;";
				}
			}
			for (int i = 0; i < FragmentShaderText::mainThirdBlock.size(); i++) {
				retBuf += FragmentShaderText::mainThirdBlock[i];
			}
			for (int i = 0; i < FragmentShaderText::pointLightLoop.size(); i++) {
				retBuf += FragmentShaderText::pointLightLoop[i];
			}
			for (int i = 0; i < FragmentShaderText::sunCalculation.size(); i++) {
				retBuf += FragmentShaderText::sunCalculation[i];
			}

			if (hasAO) {
				retBuf += "vec3 ambient = lbo.ambientColor.rgb * albedo * texture(materialTextures, vec3(fragTexCoord, aoIndex)).r;";
			}
			else {
				retBuf += "vec3 ambient = lbo.ambientColor.rgb * albedo;";
			}
			retBuf += "vec3 color = ambient + Lo;";
			retBuf += "color /= (color + vec3(1.0));";
			retBuf += "color = pow(color, vec3(1.0/2.2));";
			retBuf += "outColor = vec4(color, 1.0);}";
		}
		else { //if hasBumps, mostly doing a second block because bumpmap changes the uv variable name from fragTexCoord to fragTexCoord
			for (int i = 0; i < FragmentShaderText::fragBumpEntry.size(); i++) {
				retBuf += FragmentShaderText::fragBumpEntry[i];
			}

			retBuf += FragmentShaderText::fragExit;
			for (int i = 0; i < FragmentShaderText::dataBindings.size(); i++) {
				retBuf += FragmentShaderText::dataBindings[i];
			}
			for (int i = 0; i < FragmentShaderText::functionBlock.size(); i++) {
				retBuf += FragmentShaderText::functionBlock[i];
			}
			//bump map should not have bones, but leaving it in regardless
			AddBindings(retBuf, flags != Material::no_texture, hasNormal, hasRough, hasMetal, hasAO, hasBumps, hasBones, instanced);

			for (int i = 0; i < FragmentShaderText::parallaxMapping.size(); i++) {
				retBuf += FragmentShaderText::parallaxMapping[i];
			}
			//entering void main()
			for (int i = 0; i < FragmentShaderText::mainEntryBlock[1].size(); i++) {
				retBuf += FragmentShaderText::mainEntryBlock[1][i];
			}
			if (!hasNormal) {
				printf("BUMP FRAGMENT SHADER SHOULD ALWAYS HAVE NORMAL \n");
				printf("BUMP FRAGMENT SHADER SHOULD ALWAYS HAVE NORMAL \n");
				retBuf += "vec3 surfaceNormal = normalize(fragNormalWorld);";
			}
			bool initializedInstanceIndex;
			if (hasRough) {
				retBuf += "float roughness = texture(materialTextures, vec3(fragTexCoord, roughIndex)).r;";
			}
			else {
				if (instanced) {
					retBuf += "int instanceIndexInt = int(instanceIndex);";
					initializedInstanceIndex = true;
					retBuf += "float roughness = mbo[instanceIndexInt].rough;";
				}
				else {
					retBuf += "float roughness = mbo.rough;";
				}
			}
			if (hasMetal) {
				retBuf += "float metal = texture(materialTextures, vec3(fragTexCoord, metalIndex)).r;";
			}
			else {
				if (instanced) {
					if (!initializedInstanceIndex) {
						retBuf += "int instanceIndexInt = int(instanceIndex);";
					}
					retBuf += "float metal = mbo[instanceIndexInt].metal;";
				}
				else {
					retBuf += "float metal = mbo.metal;";
				}
			}
			for (int i = 0; i < FragmentShaderText::mainThirdBlock.size(); i++) {
				retBuf += FragmentShaderText::mainThirdBlock[i];
			}

			for (int i = 0; i < FragmentShaderText::bumpSunCalculation.size(); i++) {
				retBuf += FragmentShaderText::bumpSunCalculation[i];
			}

			if (hasAO) {
				retBuf += "vec3 ambient = lbo.ambientColor.rgb * albedo * texture(materialTextures, vec3(fragTexCoord, aoIndex)).r;";
			}
			else {
				retBuf += "vec3 ambient = lbo.ambientColor.rgb * albedo;";
			}
			retBuf += "vec3 color = ambient + Lo;";
			retBuf += "color /= (color + vec3(1.0));";
			//shaderString += "color = pow(color, vec3(1.0/2.2));";
			retBuf += "color = pow(color, vec3(1.0/2.2));";
			//shaderString += "outColor = vec4(color, 1.0);}";
			retBuf += "outColor = vec4(color, 1.0);}";
		}


#if DEBUGGING_SHADERS
		if (!std::filesystem::exists("shaders/materials/debugging")) {
			printf("debugging directory doesn't exist, creating \n");
			std::filesystem::create_directories("shaders/materials/debugging");
		}

		std::string debugShaderPath = "shaders/materials/debugging/D_";
		debugShaderPath += std::to_string(flags) + ".frag";
		std::ofstream debugShader{ debugShaderPath, std::ios::trunc };
		if (!debugShader.is_open()) {
			printf("COULD NOT OPEN OR FIND DEBUG SHADER FILE \n");
		}

		uint64_t last = 0;
		for(int i = 0; i < retBuf.size(); i++){
			if ((retBuf[i] == ';') || (retBuf[i] == '{') || (retBuf[i] == '}')) {
				debugShader << retBuf.substr(last, i - last + 1);
				last = i + 1;
				debugShader << '\n';
			}
		}

		debugShader.close();
#endif

		return retBuf;
	}

	std::string BuildVertexShader(bool hasNormal, uint16_t boneCount, bool instanced, bool largeInstance) {
		assert(false && "not currently setup");

		std::string shaderString;
		if (hasNormal) {
			for (int i = 0; i < VertexShaderText::vertexTangentInput.size(); i++) {
				shaderString += VertexShaderText::vertexTangentInput[i];
			}
			for (int i = 0; i < VertexShaderText::vertexTangentOutput.size(); i++) {
				shaderString += VertexShaderText::vertexTangentOutput[i];
			}
		}
		else {
			for (int i = 0; i < VertexShaderText::vertexNNInput.size(); i++) {
				shaderString += VertexShaderText::vertexNNInput[i];
			}
			for (int i = 0; i < VertexShaderText::vertexNNOutput.size(); i++) {
				shaderString += VertexShaderText::vertexNNOutput[i];
			}
		}
		for (int i = 0; i < VertexShaderText::vertexEntry.size(); i++) {
			shaderString += VertexShaderText::vertexEntry[i];
		}
		if (instanced) {


			if (largeInstance) {
				for (int i = 0; i < VertexShaderText::vertexInstanceBuffersFirstHalf.size(); i++) {
					shaderString += VertexShaderText::vertexInstanceBuffersFirstHalf[i];
				}
			}
			else {
				for (int i = 0; i < VertexShaderText::vertexSmallInstanceBuffersFirstHalf.size(); i++) {
					shaderString += VertexShaderText::vertexSmallInstanceBuffersFirstHalf[i];
				}
			}
			shaderString += std::to_string(boneCount) + ";";
			for (int i = 0; i < VertexShaderText::vertexInstanceBuffersSecondHalf.size(); i++) {
				shaderString += VertexShaderText::vertexInstanceBuffersSecondHalf[i];
			}
			if (hasNormal) {
				for (int i = 0; i < VertexShaderText::vertexTangentInstancingMainExit.size(); i++) {
					shaderString += VertexShaderText::vertexTangentInstancingMainExit[i];
				}
			}
			else {
				for (int i = 0; i < VertexShaderText::vertexNNInstancingMainExit.size(); i++) {
					shaderString += VertexShaderText::vertexNNInstancingMainExit[i];
				}
			}
		}
		else {
			for (int i = 0; i < VertexShaderText::vertexNoInstanceBuffers.size(); i++) {
				shaderString += VertexShaderText::vertexNoInstanceBuffers[i];
			}
			if (hasNormal) {
				for (int i = 0; i < VertexShaderText::vertexTangentNoInstancingMainExit.size(); i++) {
					shaderString += VertexShaderText::vertexTangentNoInstancingMainExit[i];
				}
			}
			else {
				for (int i = 0; i < VertexShaderText::vertexNNNoInstancingMainExit.size(); i++) {
					shaderString += VertexShaderText::vertexNNNoInstancingMainExit[i];
				}
			}
		}
#if DEBUGGING_SHADERS
		if (!std::filesystem::exists("shaders/debugging")) {
			printf("debugging directory doesn't exist, creating \n");
			std::filesystem::create_directories("shaders/debugging");
		}


		std::string debugShaderPath = "shaders/debugging/D_";
		debugShaderPath += std::to_string(boneCount) + ".vert";
		std::ofstream debugShader{ debugShaderPath, std::ios::trunc };
		if (!debugShader.is_open()) {
			printf("COULD NOT OPEN OR FIND DEBUG SHADER FILE \n");
		}
		//printf("inserting new lines : %d \n", shaderString.size());
		for (int i = 0; i < shaderString.size(); i++) {
			if ((shaderString[i] == ';') || (shaderString[i] == '{') || (shaderString[i] == '}')) {
				shaderString.insert(shaderString.begin() + i + 1, '\n');
				if (shaderString[i] == '}') {
					shaderString.insert(shaderString.begin() + i + 1, '\n');
					i++;
				}
				i++;
			}
		}
		//debugShader.write(shaderString.c_str(), shaderString.length());
		debugShader << shaderString;
		debugShader.close();
#endif


		return shaderString;
	}

	namespace SpirvHelper {
		void InitResources(TBuiltInResource& Resources) {
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

		bool ParseShader_DEBUG(TBuiltInResource& resource, EShMessages& messages, glslang::TShader& shader) {
#if EWE_DEBUG
			printf("parsing shader \n");
#endif
			/*
			* failure here COULD mean the glslang compiler is not initiated
			* debug steps -
			* ensure glslang compiler is iniated

			* if it is, ensure the shader code is correctly written
			* ^ do this by going in the debug folder
				1. add #version 450 to the beginning of the file
				2. run the compiler manually
			  this will give further warnings

			** i really hate how little detail glslang gives
			*/

			bool finalizeHere = false;
			if (!glslangInitialized) {
				glslang::InitializeProcess();
				finalizeHere = true;
			}

			const bool parseRet = shader.parse(&resource, 450, false, messages);
			printf("shader parse DEBUG : %d \n", parseRet);

			printf("info log - \n");

			printf("\t%s\n", shader.getInfoLog());
			printf("\ninfo debug log - \n");
			printf("\t%s\n", shader.getInfoDebugLog());
			if (finalizeHere) {
				glslang::FinalizeProcess();
			}
			return parseRet;
		}

		bool ParseShader(TBuiltInResource& resource, EShMessages& messages, glslang::TShader& shader) {
#if EWE_DEBUG
			printf("parsing shader \n");
#endif
			/*
			* failure here COULD mean the glslang compiler is not initiated
		    * debug steps - 
			* ensure glslang compiler is iniated
			
			* if it is, ensure the shader code is correctly written
			* ^ do this by going in the debug folder
				1. add #version 450 to the beginning of the file
				2. run the compiler manually 
			  this will give further warnings
			
			** i really hate how little detail glslang gives
			*/

			bool finalizeHere = false;
			if (!glslangInitialized) {
				glslang::InitializeProcess();
				finalizeHere = true;
			}
			const bool parseRet = shader.parse(&resource, 450, false, messages);
			if (!parseRet) {
				printf("shader parse failed \n");

				printf("info log - \n");

#if EWE_DEBUG
				const char* infoLog = shader.getInfoLog();
				//ERROR: 0:1
#endif

				printf("\t%s\n", shader.getInfoLog());
				printf("\ninfo debug log - \n");
				printf("\t%s\n", shader.getInfoDebugLog());
			}
			if (finalizeHere) {
				glslang::FinalizeProcess();
			}
			return parseRet;
		}

		void BuildFlaggedFrag_DEBUG(std::string& debugContents) {

			glslang::TShader shader(EShLangFragment);
			glslang::TProgram program{};
			TBuiltInResource Resources{};
			InitResources(Resources);

			// Enable SPIR-V and Vulkan rules when parsing GLSL
			EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo | EShMsgEnhanced | EShMsgCascadingErrors);

			const char* ptrBuffer = debugContents.c_str();
			const char* const* shaderStrings = &ptrBuffer;
			shader.setStrings(shaderStrings, 1);

			ParseShader_DEBUG(Resources, messages, shader);
		}

		bool BuildFlaggedFrag(MaterialFlags flags, bool hasBones, std::vector<unsigned int>& spirv) { //shader stage ALWAYS frag?

			glslang::TShader shader(EShLangFragment);
			glslang::TProgram program{};
			TBuiltInResource Resources{};
			InitResources(Resources);

			// Enable SPIR-V and Vulkan rules when parsing GLSL
			EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo | EShMsgEnhanced | EShMsgCascadingErrors);
			std::string fragString = BuildFragmentShader(flags, hasBones);
#if EWE_DEBUG
			printf("immediately after building shader string \n");
#endif

			const char* ptrBuffer = fragString.c_str();
			const char* const* shaderStrings = &ptrBuffer;
			shader.setStrings(shaderStrings, 1);

			if (!ParseShader(Resources, messages, shader)) {
				return false;
			}

			program.addShader(&shader);

			//
			// Program-level processing...
			//
			//printf("linking program \n"); //what is this doing? just processing? processing what?
			if (!program.link(messages)) {
				puts(shader.getInfoLog());
				puts(shader.getInfoDebugLog());
				fflush(stdout);
				return false;
			}
			//printf("compiling \n");
			glslang::GlslangToSpv(*program.getIntermediate(EShLangFragment), spirv);
			std::string shaderFileName = SHADER_DYNAMIC_PATH;
			shaderFileName += std::to_string(flags);
			if (hasBones) {
				shaderFileName += "b";
			}
			shaderFileName += ".frag.spv";
			std::ofstream outShader{ shaderFileName, std::ios::binary };

			if (outShader.is_open()) {
#if EWE_DEBUG
				printf("writing to shader location : %s \n", shaderFileName.c_str());
#endif
				outShader.write((char*)spirv.data(), spirv.size() * sizeof(unsigned int));
				outShader.close();
			}
			else {
				printf("failed to save shader \n");
			}

			return true;
		}

		bool LoadingVertSPV(std::vector<unsigned int>& spirv) {
			glslang::TShader shader(EShLangVertex);
			glslang::TProgram program;
			const char* shaderStrings[1];
			TBuiltInResource Resources{};
			InitResources(Resources);

			// Enable SPIR-V and Vulkan rules when parsing GLSL
			EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo | EShMsgEnhanced);
			std::string tempString;
			for (auto& line : EWE::LoadingVertString) {
				printf("line of loading vert : %s \n", line.c_str());
				tempString += line;
			}
			shaderStrings[0] = tempString.c_str();
			shader.setStrings(shaderStrings, 1);

			if (!ParseShader(Resources, messages, shader)) {
				return false;
			}

			printf("adding shader to program? \n");
			program.addShader(&shader);

			//
			// Program-level processing...
			//
			printf("linking program \n"); //what is this doing? just processing? processing what?
			if (!program.link(messages)) {
				puts(shader.getInfoLog());
				puts(shader.getInfoDebugLog());
				fflush(stdout);
				return false;
			}
			//printf("compiling \n");
			glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex), spirv);
			std::string shaderFileName = SHADER_DYNAMIC_PATH;
			shaderFileName += "loading.vert.spv";


			std::ofstream outShader{ shaderFileName, std::ios::binary };

			if (outShader.is_open()) {
				printf("writing to shader location : %s \n", shaderFileName.c_str());
				outShader.write((char*)spirv.data(), spirv.size() * sizeof(unsigned int));
				outShader.close();
			}
			else {
				printf("failed to save shader \n");
			}

			return true;
		}
		bool LoadingFragSPV(std::vector<unsigned int>& spirv) { //shader stage ALWAYS frag?
			glslang::TShader shader(EShLangFragment);
			glslang::TProgram program;
			const char* shaderStrings[1];
			TBuiltInResource Resources{};
			InitResources(Resources);

			// Enable SPIR-V and Vulkan rules when parsing GLSL
			EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo | EShMsgEnhanced);
			std::string tempString;
			for (auto& line : EWE::LoadingFragString) {
				tempString += line;
			}
			shaderStrings[0] = tempString.c_str();
			shader.setStrings(shaderStrings, 1);

			if (!ParseShader(Resources, messages, shader)) {
				return false;
			}

			program.addShader(&shader);

			if (!program.link(messages)) {
				puts(shader.getInfoLog());
				puts(shader.getInfoDebugLog());
				fflush(stdout);
				return false;
			}
			//printf("compiling \n");
			glslang::GlslangToSpv(*program.getIntermediate(EShLangFragment), spirv);
			std::string shaderFileName = SHADER_DYNAMIC_PATH;
			shaderFileName += "loading.frag.spv";
			std::ofstream outShader{ shaderFileName, std::ios::binary };

			if (outShader.is_open()) {
				printf("writing to shader location : %s \n", shaderFileName.c_str());
				outShader.write((char*)spirv.data(), spirv.size() * sizeof(unsigned int));
				outShader.close();
			}
			else {
				printf("failed to save shader \n");
			}

			return true;
		}

		bool BuildFlaggedVert(bool hasNormal, uint16_t boneCount, bool instanced, std::vector<unsigned int>& spirv, bool largeInstance) { //shader stage ALWAYS frag?
			glslang::TShader shader(EShLangVertex);
			glslang::TProgram program;
			const char* shaderStrings[1];
			TBuiltInResource Resources{};
			InitResources(Resources);

			// Enable SPIR-V and Vulkan rules when parsing GLSL
			EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo | EShMsgEnhanced);
			const std::string tempString = BuildVertexShader(hasNormal, boneCount, instanced, largeInstance); //this is a reference right?
			shaderStrings[0] = tempString.c_str();
			shader.setStrings(shaderStrings, 1);

			if (!ParseShader(Resources, messages, shader)) {
				return false;
			}

			program.addShader(&shader);

			//
			// Program-level processing...
			//
			//printf("linking program \n"); //what is this doing? just processing? processing what?
			if (!program.link(messages)) {
				puts(shader.getInfoLog());
				puts(shader.getInfoDebugLog());
				fflush(stdout);
				return false;
			}
			//printf("compiling \n");
			glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex), spirv);
			std::string shaderFileName = SHADER_DYNAMIC_PATH;
			if (hasNormal) {
				shaderFileName += 'n';
			}
			shaderFileName += std::to_string(boneCount) + ".vert.spv";


			std::ofstream outShader{ shaderFileName, std::ios::binary };

			if (outShader.is_open()) {
				printf("writing to shader location : %s \n", shaderFileName.c_str());
				outShader.write((char*)spirv.data(), spirv.size() * sizeof(unsigned int));
				outShader.close();
			}
			else {
				printf("failed to save shader \n");
			}

			return true;
		}


	} //namespace SpirvHelper

	namespace ShaderBlock {


		std::vector<uint32_t> GetLoadingVertShader() {


			std::string subPath = "shaders/loading.vert.spv";
			printf("subPath : %s \n", subPath.c_str());
			if (std::filesystem::exists(subPath)) {
				printf("reading loading vertex shader from file\n");
				std::ifstream inShader{ subPath, std::ios::binary };
				if (!inShader.is_open()) {
					printf("failed to open an existing file? \n");
					return {};
				}
				// Get the file size
				inShader.seekg(0, std::ios::end);
				std::streampos fileSize = inShader.tellg();
				inShader.seekg(0, std::ios::beg);

				// Calculate the number of uint32_t elements in the file
				std::size_t numElements = fileSize / sizeof(uint32_t);

				// Create a vector to store the file contents
				std::vector<uint32_t> shaderCodeSpirV(numElements);

				// Read the file data into the vector
				inShader.read(reinterpret_cast<char*>(shaderCodeSpirV.data()), fileSize);

				inShader.close();
				return shaderCodeSpirV;
			}
			else if (!std::filesystem::exists(SHADER_DYNAMIC_PATH)) {
				printf("shader dynamic path doesn't exist : %s\n", SHADER_DYNAMIC_PATH);
				std::filesystem::create_directory(SHADER_DYNAMIC_PATH);
			}
			std::vector<uint32_t> shaderCodeSpirV;
			if (SpirvHelper::LoadingVertSPV(shaderCodeSpirV)) {
				//printf("compiled shader to spv successfully \n");
			}
			else {
				assert(false && "failed to compile loading vert shader");
				throw std::runtime_error("failed to compile shader");
			}

			return shaderCodeSpirV;
		}
		std::vector<uint32_t> GetLoadingFragShader() {
			std::string subPath = "shaders/loading.frag.spv";
			printf("subPath : %s \n", subPath.c_str());
			if (std::filesystem::exists(subPath)) {
				printf("reading loading frag shader from file\n");
				std::ifstream inShader{ subPath, std::ios::binary };
				if (!inShader.is_open()) {
					printf("failed to open an existing file? \n");
					return {};
				}
				// Get the file size
				inShader.seekg(0, std::ios::end);
				std::streampos fileSize = inShader.tellg();
				inShader.seekg(0, std::ios::beg);

				// Calculate the number of uint32_t elements in the file
				std::size_t numElements = fileSize / sizeof(uint32_t);

				// Create a vector to store the file contents
				std::vector<uint32_t> shaderCodeSpirV(numElements);

				// Read the file data into the vector
				inShader.read(reinterpret_cast<char*>(shaderCodeSpirV.data()), fileSize);

				inShader.close();
				return shaderCodeSpirV;
			}
			else if (!std::filesystem::exists(SHADER_DYNAMIC_PATH)) {
				printf("shader dynamic path doesn't exist : %s\n", SHADER_DYNAMIC_PATH);
				std::filesystem::create_directory(SHADER_DYNAMIC_PATH);
			}
			std::vector<uint32_t> shaderCodeSpirV;
			if (SpirvHelper::LoadingFragSPV(shaderCodeSpirV)) {
				//printf("compiled shader to spv successfully \n");
			}
			else {
				assert(false && "failed to compile loading frag shader");
				throw std::runtime_error("failed to compile shader");
				//throw std run time error
			}

			return shaderCodeSpirV;


		}

		std::vector<uint32_t> GetFragmentShader(MaterialFlags flags, bool hasBones) {
			/*
			auto print_msg_to_printf = [](spv_message_level_t, const char*, const spv_position_t&, const char* m) {
					printf("\t SPIRV Validator : error: %s \n", m);
			};
			spvtools::SpirvTools spirv_tools(SPV_ENV_VULKAN_1_0);
			spirv_tools.SetMessageConsumer(print_msg_to_printf);
			*/
			//printf("gettingg fragment shader : %d \n", flags);
			std::string subPath = SHADER_DYNAMIC_PATH;
			subPath += SHADER_VERSION_ID;
			subPath += std::to_string(flags);
			if (hasBones) {
				subPath += "b";
			}
			subPath += ".frag.spv";
			printf("subPath : %s \n", subPath.c_str());
			if (std::filesystem::exists(subPath)) {
				printf("reading shader from file : %ud \n", flags);
				std::ifstream inShader{ subPath, std::ios::binary };
				if (!inShader.is_open()) {
					printf("failed to open an existing file? \n");
					return {};
				}
				// Get the file size
				inShader.seekg(0, std::ios::end);
				std::streampos fileSize = inShader.tellg();
				inShader.seekg(0, std::ios::beg);

				// Calculate the number of uint32_t elements in the file
				std::size_t numElements = fileSize / sizeof(uint32_t);

				// Create a vector to store the file contents
				std::vector<uint32_t> shaderCodeSpirV(numElements);

				// Read the file data into the vector
				inShader.read(reinterpret_cast<char*>(shaderCodeSpirV.data()), fileSize);

				inShader.close();
				return shaderCodeSpirV;
			}
			std::vector<uint32_t> shaderCodeSpirV;

			if (SpirvHelper::BuildFlaggedFrag(flags, hasBones, shaderCodeSpirV)) {
				//printf("compiled shader to spv successfully \n");
			}
			else {
#if DEBUGGING_SHADERS
				//output it with lines separated so i can debug faster and dont have to run a separate debugger. currently, in the first run, everything is squished to one line
				//it shouldve been saved in debug build

				std::string debugFileName = "shaders/materials/debugging/D_";
				debugFileName += std::to_string(flags);
				debugFileName += ".frag";
				std::ifstream savedDebuggingFile{debugFileName, std::ios::binary};
				assert(savedDebuggingFile.is_open());
				{
					std::istream is(savedDebuggingFile.rdbuf());
					std::ostringstream ss;
					ss << is.rdbuf();
					std::string debugFileContents = ss.str();
					SpirvHelper::BuildFlaggedFrag_DEBUG(debugFileContents);
				}
				

#endif

				printf("failed to compile shader : %d \n", flags);
				assert(false && "failed to compile frag shader");
				throw std::runtime_error("failed to compile shader");
			}
			/*
			uint8_t paddingNeeded = 4 - shaderCodeSpirV.size() % 4;
			printf("padding needed, size - %d:%d \n", paddingNeeded, shaderCodeSpirV.size());
			for(int i = 0; i < paddingNeeded; i++) {
				shaderCodeSpirV.push_back(0);
			}
			*/

			return shaderCodeSpirV;


		}

		std::vector<uint32_t> GetVertexShader(bool hasNormal, uint16_t boneCount, bool instanced, bool largeInstance) {


			std::string subPath = SHADER_DYNAMIC_PATH;
			subPath += SHADER_VERSION_ID;
			if (hasNormal) {
				subPath += 'n';
			}

			subPath += std::to_string(boneCount) + ".vert.spv";
			printf("subPath : %s \n", subPath.c_str());
			if (std::filesystem::exists(subPath)) {
				printf("reading vertex shader from file : %d:%ud \n", hasNormal, boneCount);
				std::ifstream inShader{ subPath, std::ios::binary };
				if (!inShader.is_open()) {
					printf("failed to open an existing file? \n");
					return {};
				}
				// Get the file size
				inShader.seekg(0, std::ios::end);
				std::streampos fileSize = inShader.tellg();
				inShader.seekg(0, std::ios::beg);

				// Calculate the number of uint32_t elements in the file
				std::size_t numElements = fileSize / sizeof(uint32_t);

				// Create a vector to store the file contents
				std::vector<uint32_t> shaderCodeSpirV(numElements);

				// Read the file data into the vector
				inShader.read(reinterpret_cast<char*>(shaderCodeSpirV.data()), fileSize);

				inShader.close();
				return shaderCodeSpirV;
			}
			std::vector<uint32_t> shaderCodeSpirV;
			if (SpirvHelper::BuildFlaggedVert(hasNormal, boneCount, instanced, shaderCodeSpirV, largeInstance)) {
				//printf("compiled shader to spv successfully \n");
			}
			else {
				printf("failed to compile vertex shader : %d:%d \n", hasNormal, boneCount);
				assert(false && "failed to compile vert shader");
				throw std::runtime_error("failed to compile shader");
			}
			/*
			uint8_t paddingNeeded = 4 - shaderCodeSpirV.size() % 4;
			printf("padding needed, size - %d:%d \n", paddingNeeded, shaderCodeSpirV.size());
			for(int i = 0; i < paddingNeeded; i++) {
				shaderCodeSpirV.push_back(0);
			}
			*/

			return shaderCodeSpirV;


		}


	}//namespace ShaderBlock
} //namespace EWE