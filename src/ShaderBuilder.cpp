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

namespace EWE {
	std::unordered_map<std::thread::id, std::vector<std::string>> stringBuffer{};
	std::mutex stringMut{};
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

	void AddBindings(std::vector<const char*>& retVec, bool hasNormal, bool hasRough, bool hasMetal, bool hasAO, bool hasBumps, bool setTwo) {
		uint8_t currentBinding = hasBumps;

		//shaderString += firstHalfBinding[hasBones];
		retVec.push_back(FragmentShaderText::firstHalfBinding[setTwo].c_str());
		//shaderString += std::to_string(currentBinding);
		//retVec.emplace_back(std::to_string(currentBinding).c_str());
		std::thread::id thisThreadID = std::this_thread::get_id();
		stringMut.lock(); //im locking it here, and unlocking it at the end, so that a different thread emplacing doesn't move this vector
		std::vector<std::string>* threadBuffer;
		{
			auto stringBufferFind = stringBuffer.find(thisThreadID);
			if (stringBufferFind == stringBuffer.end()) {
				threadBuffer = &(stringBuffer.try_emplace(thisThreadID, std::vector<std::string>{}).first->second);
			}
			else {
				threadBuffer = &stringBufferFind->second;
			}
		}
		threadBuffer->resize(1 + hasNormal + hasRough + hasMetal + hasAO);
		threadBuffer->at(currentBinding - hasBumps) = std::to_string(currentBinding);
		retVec.push_back(threadBuffer->at(currentBinding - hasBumps).c_str());

#if EWE_DEBUG
		printf("~~~~~ CURRENT BINDING : %s \n", retVec.back());
#endif
		currentBinding++;
		//shaderString += secondHalfBinding;
		retVec.push_back(FragmentShaderText::secondHalfBinding.c_str());
		//shaderString += "albedoSampler;";
		retVec.push_back("albedoSampler;");


		if (hasNormal) {
			//shaderString += firstHalfBinding[hasBones];
			retVec.push_back(FragmentShaderText::firstHalfBinding[setTwo].c_str());
			//shaderString += std::to_string(currentBinding);
			threadBuffer->at(currentBinding - hasBumps) = std::to_string(currentBinding);
			retVec.push_back(threadBuffer->at(currentBinding - hasBumps).c_str());
			currentBinding++;
			//shaderString += secondHalfBinding;
			retVec.push_back(FragmentShaderText::secondHalfBinding.c_str());
			//shaderString += "normalSampler;";
			retVec.push_back("normalSampler;");
		}
		if (hasRough) {
			//shaderString += firstHalfBinding[hasBones];
			retVec.push_back(FragmentShaderText::firstHalfBinding[setTwo].c_str());
			//shaderString += std::to_string(currentBinding);
			threadBuffer->at(currentBinding - hasBumps) = std::to_string(currentBinding);
			retVec.push_back(threadBuffer->at(currentBinding - hasBumps).c_str());
			currentBinding++;
			//shaderString += secondHalfBinding;
			retVec.push_back(FragmentShaderText::secondHalfBinding.c_str());
			//shaderString += "roughSampler;";
			retVec.push_back("roughSampler;");
		}

		if (hasMetal) {
			//shaderString += firstHalfBinding[hasBones];
			retVec.push_back(FragmentShaderText::firstHalfBinding[setTwo].c_str());
			//shaderString += std::to_string(currentBinding);
			threadBuffer->at(currentBinding - hasBumps) = std::to_string(currentBinding);
			retVec.push_back(threadBuffer->at(currentBinding - hasBumps).c_str());
			currentBinding++;
			//shaderString += secondHalfBinding;
			retVec.push_back(FragmentShaderText::secondHalfBinding.c_str());
			//shaderString += "metalSampler;";
			retVec.push_back("metalSampler;");
		}
		if (hasAO) {
			//shaderString += firstHalfBinding[hasBones];
			retVec.push_back(FragmentShaderText::firstHalfBinding[setTwo].c_str());
			//shaderString += std::to_string(currentBinding);
			threadBuffer->at(currentBinding - hasBumps) = std::to_string(currentBinding);
			retVec.push_back(threadBuffer->at(currentBinding - hasBumps).c_str());
			currentBinding++;
			//shaderString += secondHalfBinding;
			retVec.push_back(FragmentShaderText::secondHalfBinding.c_str());
			//shaderString += "amOccSampler;";
			retVec.push_back("amOccSampler;");
		}
		stringMut.unlock();
	}

	std::vector<const char*> BuildFragmentShader(MaterialFlags flags, bool hasBones) {
		//printf("building fragment shader :%d \n", flags);
		//bool hasTangents = flags & 32; //if it has a normal map, it has tangents
		//bool hasBones = flags & 128;
		//bool instanced = MaterialFlags & 64;

		std::vector<const char*> retVec{};

		bool instanced = flags & 64;
		bool hasBumps = flags & 16;
		bool hasNormal = flags & 8;
		bool hasRough = flags & 4;
		bool hasMetal = flags & 2;
		bool hasAO = flags & 1;
		//building tangent frag
		//std::string shaderString;
		//shaderString += version;
		if (!hasBumps) {
			for (int i = 0; i < FragmentShaderText::fragNNEntry.size(); i++) {
				//shaderString += fragNNEntry[i];
				retVec.push_back(FragmentShaderText::fragNNEntry[i].c_str());
			}
			if (hasNormal) {
				//shaderString += "layout (location = 3) in vec3 fragTangentWorld;";
				retVec.push_back("layout (location = 3) in vec3 fragTangentWorld;");
			}
			//shaderString += fragExit;
			retVec.push_back(FragmentShaderText::fragExit.c_str());

			for (int i = 0; i < FragmentShaderText::dataBindings.size(); i++) {
				//shaderString += dataBindings[i];
				retVec.push_back(FragmentShaderText::dataBindings[i].c_str());
			}
			for (int i = 0; i < FragmentShaderText::functionBlock.size(); i++) {
				//shaderString += functionBlock[i];
				retVec.push_back(FragmentShaderText::functionBlock[i].c_str());
			}

			AddBindings(retVec, hasNormal, hasRough, hasMetal, hasAO, hasBumps, hasBones | instanced);

			if (hasNormal) {
				for (int i = 0; i < FragmentShaderText::calcNormalFunction.size(); i++) {
					//shaderString += calcNormalFunction[i];
					retVec.push_back(FragmentShaderText::calcNormalFunction[i].c_str());
				}
			}

			for (int i = 0; i < FragmentShaderText::mainEntryBlock[0].size(); i++) {
				//shaderString += mainEntryBlock[0][i];
				retVec.push_back(FragmentShaderText::mainEntryBlock[0][i].c_str());
			}
			for (int i = 0; i < FragmentShaderText::mainSecondBlockNN.size(); i++) {
				//shaderString += mainSecondBlockNN[i];
				retVec.push_back(FragmentShaderText::mainSecondBlockNN[i].c_str());
			}
			if (hasNormal) {
				//shaderString += "vec3 normal = calculateNormal();";
				retVec.push_back("vec3 normal = calculateNormal();");
			}
			else {
				//shaderString += "vec3 normal = normalize(fragNormalWorld);";
				retVec.push_back("vec3 normal = normalize(fragNormalWorld);");
			}
			if (hasRough) {
				//shaderString += "float roughness = texture(roughSampler, fragTexCoord).r;";
				retVec.push_back("float roughness = texture(roughSampler, fragTexCoord).r;");
			}
			else {
				//shaderString += "float roughness = 0.5;";
				retVec.push_back("float roughness = 0.5;");
			}
			if (hasMetal) {
				//shaderString += "float metal = texture(metalSampler, fragTexCoord).r;";
				retVec.push_back("float metal = texture(metalSampler, fragTexCoord).r;");
			}
			else {
				//shaderString += "float metal = 0.0;";
				retVec.push_back("float metal = 0.0;");
			}
			for (int i = 0; i < FragmentShaderText::mainThirdBlock.size(); i++) {
				//shaderString += mainThirdBlock[i];
				retVec.push_back(FragmentShaderText::mainThirdBlock[i].c_str());
			}
			for (int i = 0; i < FragmentShaderText::pointLightLoop.size(); i++) {
				//shaderString += pointLightLoop[i];
				retVec.push_back(FragmentShaderText::pointLightLoop[i].c_str());
			}
			for (int i = 0; i < FragmentShaderText::sunCalculation.size(); i++) {
				//shaderString += sunCalculation[i];
				retVec.push_back(FragmentShaderText::sunCalculation[i].c_str());
			}

			if (hasAO) {
				//shaderString += "vec3 ambient = vec3(0.05) * albedo * texture(amOccSampler, fragTexCoord).r;";
				retVec.push_back("vec3 ambient = vec3(0.05) * albedo * texture(amOccSampler, fragTexCoord).r;");
			}
			else {
				//shaderString += "vec3 ambient = vec3(0.05) * albedo;";
				retVec.push_back("vec3 ambient = vec3(0.05) * albedo;");
			}
			//shaderString += "vec3 color = ambient + Lo;";
			retVec.push_back("vec3 color = ambient + Lo;");
			//shaderString += "color /= (color + vec3(1.0));";
			retVec.push_back("color /= (color + vec3(1.0));");
			//shaderString += "color = pow(color, vec3(1.0/2.2));";
			retVec.push_back("color = pow(color, vec3(1.0/2.2));");
			//shaderString += "outColor = vec4(color, 1.0);}";
			retVec.push_back("outColor = vec4(color, 1.0);}");
		}
		else { //if hasBumps, mostly doing a second block because bumpmap changes the uv variable name from fragTexCoord to fragTexCoord
			for (int i = 0; i < FragmentShaderText::fragBumpEntry.size(); i++) {
				//shaderString += fragBumpEntry[i];
				retVec.push_back(FragmentShaderText::fragBumpEntry[i].c_str());
			}
			//shaderString += fragExit;
			retVec.push_back(FragmentShaderText::fragExit.c_str());
			for (int i = 0; i < FragmentShaderText::dataBindings.size(); i++) {
				//shaderString += dataBindings[i];
				retVec.push_back(FragmentShaderText::dataBindings[i].c_str());
			}
			for (int i = 0; i < FragmentShaderText::functionBlock.size(); i++) {
				//shaderString += functionBlock[i];
				retVec.push_back(FragmentShaderText::functionBlock[i].c_str());
			}
			//bump map should not have bones, but leaving it in regardless
			AddBindings(retVec, hasNormal, hasRough, hasMetal, hasAO, hasBumps, hasBones);

			for (int i = 0; i < FragmentShaderText::parallaxMapping.size(); i++) {
				//shaderString += parallaxMapping[i];
				retVec.push_back(FragmentShaderText::parallaxMapping[i].c_str());
			}
			//entering void main()
			for (int i = 0; i < FragmentShaderText::mainEntryBlock[1].size(); i++) {
				//shaderString += mainEntryBlock[1][i];
				retVec.push_back(FragmentShaderText::mainEntryBlock[1][i].c_str());
			}
			if (!hasNormal) {
				printf("BUMP FRAGMENT SHADER SHOULD ALWAYS HAVE NORMAL \n");
				printf("BUMP FRAGMENT SHADER SHOULD ALWAYS HAVE NORMAL \n");
				//shaderString += "vec3 surfaceNormal = normalize(fragNormalWorld);";
				retVec.push_back("vec3 surfaceNormal = normalize(fragNormalWorld);");
			}
			if (hasRough) {
				//shaderString += "float roughness = texture(roughSampler, fragTexCoord).r;";
				retVec.push_back("float roughness = texture(roughSampler, fragTexCoord).r;");
			}
			else {
				//shaderString += "float roughness = 0.5;";
				retVec.push_back("float roughness = 0.5;");
			}
			if (hasMetal) {
				//shaderString += "float metal = texture(metalSampler, fragTexCoord).r;";
				retVec.push_back("float metal = texture(metalSampler, fragTexCoord).r;");
			}
			else {
				//shaderString += "float metal = 0.0f;";
				retVec.push_back("float metal = 0.0;");
			}
			for (int i = 0; i < FragmentShaderText::mainThirdBlock.size(); i++) {
				//shaderString += mainThirdBlock[i];
				retVec.push_back(FragmentShaderText::mainThirdBlock[i].c_str());
			}

			for (int i = 0; i < FragmentShaderText::bumpSunCalculation.size(); i++) {
				//shaderString += bumpSunCalculation[i];
				retVec.push_back(FragmentShaderText::bumpSunCalculation[i].c_str());
			}

			if (hasAO) {
				//shaderString += "vec3 ambient = vec3(0.05) * albedo * texture(amOccSampler, fragTexCoord).r;";
				retVec.push_back("vec3 ambient = vec3(0.05) * albedo * texture(amOccSampler, fragTexCoord).r;");
			}
			else {
				//shaderString += "vec3 ambient = vec3(0.05) * albedo;";
				retVec.push_back("vec3 ambient = vec3(0.05) * albedo;");
			}
			//shaderString += "vec3 color = ambient + Lo;";
			retVec.push_back("vec3 color = ambient + Lo;");
			//shaderString += "color /= (color + vec3(1.0));";
			retVec.push_back("color /= (color + vec3(1.0));");
			//shaderString += "color = pow(color, vec3(1.0/2.2));";
			retVec.push_back("color = pow(color, vec3(1.0/2.2));");
			//shaderString += "outColor = vec4(color, 1.0);}";
			retVec.push_back("outColor = vec4(color, 1.0);}");
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
		//printf("inserting new lines : %d \n", shaderString.size());

		for (auto& cstr : retVec) {
			debugShader << cstr << '\n';
		}

		//debugShader.write(shaderString.c_str(), shaderString.length());
		//for (int i = 0; i < shaderString.size(); i++) {
		//	if ((shaderString[i] == ';') || (shaderString[i] == '{') || (shaderString[i] == '}')) {
		//		shaderString.insert(shaderString.begin() + i + 1, '\n');
		//		i++;
		//	}
		//}
		//debugShader << shaderString;
		debugShader.close();
#endif

		return retVec;
	}

	std::string BuildVertexShader(bool hasNormal, uint16_t boneCount, bool instanced, bool largeInstance) {
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
			if (!shader.parse(&resource, 450, false, messages)) {
				printf("shader parse failed \n");

				printf("info log - \n");
				printf("\t%s\n", shader.getInfoLog());
				printf("\ninfo debug log - \n");
				printf("\t%s\n", shader.getInfoDebugLog());

				return false;  // something didn't work
			}
			if (finalizeHere) {
				glslang::FinalizeProcess();
			}
			return true;
		}

		bool BuildFlaggedFrag(MaterialFlags flags, bool hasBones, std::vector<unsigned int>& spirv) { //shader stage ALWAYS frag?

			glslang::TShader shader(EShLangFragment);
			glslang::TProgram program{};
			TBuiltInResource Resources{};
			InitResources(Resources);

			// Enable SPIR-V and Vulkan rules when parsing GLSL
			EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo | EShMsgEnhanced | EShMsgCascadingErrors);
			const std::vector<const char*> fragString = BuildFragmentShader(flags, hasBones); //this won't allow the string to be destroyed right?
			printf("immediately after building shader string \n");

			//const char* ptrBuffer = tempString.c_str();
			//const char* const* shaderStrings = &ptrBuffer;
			shader.setStrings(fragString.data(), fragString.size());

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
				stringMut.lock();
				std::thread::id thisThreadID = std::this_thread::get_id();
				stringBuffer.at(thisThreadID).clear();
				stringMut.unlock();
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
				printf("writing to shader location : %s \n", shaderFileName.c_str());
				outShader.write((char*)spirv.data(), spirv.size() * sizeof(unsigned int));
				outShader.close();
			}
			else {
				printf("failed to save shader \n");
			}

			stringMut.lock();
			std::thread::id thisThreadID = std::this_thread::get_id();
			stringBuffer.at(thisThreadID).clear();
			stringMut.unlock();
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


	/*
	void ShaderBlock::BatchCreateFragmentShader(std::vector<MaterialFlags> flagVector) {
		std::vector<std::pair<uint8_t, std::string>> needShader;
		for (int i = 0; i < flagVector.size(); i++) {
			std::string subPath = SHADER_PATH;
			subPath += std::to_string(flagVector[i]) + ".frag.spv";
			if (!std::filesystem::exists(subPath)) {
				needShader.push_back({ flagVector[i], subPath });
			}
		}
		spvtools::SpirvTools spirv_tools(SPV_ENV_VULKAN_1_0);
		for (int i = 0; i < needShader.size(); i++) {
			std::vector<uint32_t> spirv;
			if (!spirv_tools.Assemble(buildFragmentShader(needShader[i].first), &spirv)) {
				printf("Failed to assemble GLSL to SPIR-V \n");
				//std throw
			}
			std::ofstream file(needShader[i].second, std::ios::binary);
			if (!file) {
				printf("Failed to open file for writing \n");
				//std throw
			}
			file.write(reinterpret_cast<const char*>(spirv.data()), spirv.size() * sizeof(uint32_t));
			file.close();
		}
	}
		*/

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
				printf("failed to compile loading vertex shader\n");
				//throw std run time error
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
				printf("failed to compile loading frag shader \n");
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
				printf("failed to compile shader : %d \n", flags);
				throw std::runtime_error("failed to compile shader");
				//throw std run time error
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
				//throw std run time error
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