#include "EWEngine/Graphics/Pipeline.h"


#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Renderer.h"


#include <fstream>
#include <iostream>
#include <cassert>

#ifndef SHADER_DIR
#define SHADER_DIR "shaders/"
#endif

namespace EWE {

	namespace Pipeline_Helper_Functions {
		std::vector<char> ReadFile(const std::string& filepath) {
			//printf("reading shader file\n");

			//#define ENGINE_DIR "..//shaders//"

			std::string enginePath = SHADER_DIR + filepath;

			//printf("readFile enginePath : %s \n", enginePath.c_str());

			std::ifstream shaderFile{};
			shaderFile.open(enginePath, std::ios::binary);
//#if EWE_DEBUG
//			const std::string errorPrint = "failed to open shader : " + enginePath;
//			assert(shaderFile.is_open() && errorPrint.c_str());
//#endif
			if(!shaderFile.is_open()){
				printf("failed ot open shader file : %s\n", enginePath.c_str());
				assert(shaderFile.is_open() && "failed to open shader");
			}
			shaderFile.seekg(0, std::ios::end);
			std::size_t fileSize = static_cast<std::size_t>(shaderFile.tellg());
			assert(fileSize > 0 && "shader is empty");

			shaderFile.seekg(0, std::ios::beg);
			std::vector<char> returnVec(fileSize);
			shaderFile.read(returnVec.data(), fileSize);
			shaderFile.close();
			return returnVec;
		}
		void CreateShaderModule(std::string const& file_path, VkShaderModule* shaderModule) {
			auto data = ReadFile(file_path);
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.codeSize = data.size();
			//printf("template data size : %d \n", data.size());
			createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

			EWE_VK(vkCreateShaderModule, VK::Object->vkDevice, &createInfo, nullptr, shaderModule);
		}
		void CreateShaderModule(const std::vector<uint32_t>& data, VkShaderModule* shaderModule) {
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.codeSize = data.size() * 4;
			//printf("uint32_t data size : %d \n", data.size());
			createInfo.pCode = data.data();

			EWE_VK(vkCreateShaderModule, VK::Object->vkDevice, &createInfo, nullptr, shaderModule);
		}
		template <typename T>
		void CreateShaderModule(const std::vector<T>& data, VkShaderModule* shaderModule) {
#if EWE_DEBUG
			//printf("creating sahder module\n");
#endif
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.codeSize = data.size() * sizeof(T);
			//printf("template data size : %d \n", data.size());
			createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

			EWE_VK(vkCreateShaderModule, VK::Object->vkDevice, &createInfo, nullptr, shaderModule);
		}
		void CreateShaderModule(const void* data, std::size_t dataSize, VkShaderModule* shaderModule) {
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.codeSize = dataSize;
			createInfo.pCode = reinterpret_cast<const uint32_t*>(data);
			//printf("uint32_t data size : %d \n", data.size());
			EWE_VK(vkCreateShaderModule, VK::Object->vkDevice, &createInfo, nullptr, shaderModule);
		}
	}

	// ~~~~~~~ COMPUTE PIPELINE ~~~~~~~~~~~~~~~
	EWE_Compute_Pipeline EWE_Compute_Pipeline::CreatePipeline(std::vector<VkDescriptorSetLayout> computeDSL, std::string compute_path) {
		EWE_Compute_Pipeline ret;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(computeDSL.size());
		pipelineLayoutInfo.pSetLayouts = computeDSL.data();

		EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &ret.pipe_layout);
		

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = ret.pipe_layout;

		std::string computePath = "compute/";
		computePath += compute_path;
		Pipeline_Helper_Functions::CreateShaderModule(computePath, &ret.shader);
		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = ret.shader;
		computeShaderStageInfo.pName = "main";
		pipelineInfo.stage = computeShaderStageInfo;
		EWE_VK(vkCreateComputePipelines, VK::Object->vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ret.pipeline);
		return ret;
	}
	EWE_Compute_Pipeline EWE_Compute_Pipeline::CreatePipeline(VkPipelineLayout pipe_layout, std::string compute_path) {
		EWE_Compute_Pipeline ret;
		ret.pipe_layout = pipe_layout;

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = ret.pipe_layout;
		Pipeline_Helper_Functions::CreateShaderModule(compute_path, &ret.shader);
		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = ret.shader;
		computeShaderStageInfo.pName = "computeMain";
		pipelineInfo.stage = computeShaderStageInfo;
		EWE_VK(vkCreateComputePipelines, VK::Object->vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ret.pipeline);
		return ret;
	}

	// ~~~~~~~~~~~~~~~~~~~~ END COMPUTE PIPELINE ~~~~~~~~~~~~~~~~~~~~~~

	std::map<std::string, VkShaderModule> EWEPipeline::shaderModuleMap;
	VkPipelineRenderingCreateInfo* EWEPipeline::PipelineConfigInfo::pipelineRenderingInfoStatic;

	EWEPipeline::EWEPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo) {
		//printf("constructing ewe pipeline\n");


		const auto vertFind = shaderModuleMap.find(vertFilepath);
		if (vertFind == shaderModuleMap.end()) {
			auto vertCode = Pipeline_Helper_Functions::ReadFile(vertFilepath);
			Pipeline_Helper_Functions::CreateShaderModule(vertCode, &vertShaderModule);
			//printf("emplacing shader module vert to map\n");
			shaderModuleMap.try_emplace(vertFilepath, vertShaderModule);
		}
		else {
			vertShaderModule = vertFind->second;
		}

		const auto fragFind = shaderModuleMap.find(fragFilepath);
		if (fragFind == shaderModuleMap.end()) {
			auto fragCode = Pipeline_Helper_Functions::ReadFile(fragFilepath);
			Pipeline_Helper_Functions::CreateShaderModule(fragCode, &fragShaderModule);
			//printf("emplacing shader module frag to map\n");
			shaderModuleMap.try_emplace(fragFilepath, fragShaderModule);
		}
		else {
			fragShaderModule = fragFind->second;
		}
		//printf("creating graphics pipeline\n");
		CreateGraphicsPipeline(configInfo);
	}

	EWEPipeline::EWEPipeline(VkShaderModule vertShaderModu, VkShaderModule fragShaderModu, const PipelineConfigInfo& configInfo) : vertShaderModule{ vertShaderModu }, fragShaderModule{ fragShaderModu } {
		CreateGraphicsPipeline(configInfo);
	}

	EWEPipeline::EWEPipeline(uint16_t boneCount, MaterialFlags flags, const PipelineConfigInfo& configInfo) {
		std::string vertPath = SHADER_DIR;
		//this is always instanced???
		bool hasNormal = (flags & Material::Flags::Texture::Normal) > 0;
		if (hasNormal) {
			vertPath += "dynamic/n" + std::to_string(boneCount) + ".vert.spv";
		}
		else {
			vertPath += "dynamic/" + std::to_string(boneCount) + ".vert.spv";
		}
		const auto vertFind = shaderModuleMap.find(vertPath);
		if (vertFind == shaderModuleMap.end()) {
			printf("creating vertex shader - %d:%d \n", boneCount, flags);
			//auto vertCode = readFile(vertPath);
			Pipeline_Helper_Functions::CreateShaderModule(ShaderBlock::GetVertexShader(hasNormal, boneCount, true), &vertShaderModule);
			shaderModuleMap.try_emplace(vertPath, vertShaderModule);
		}
		else {
			vertShaderModule = vertFind->second;
		}
		std::string fragPath = SHADER_DIR;
		fragPath += "dynamic/" + std::to_string(flags) + "b.frag.spv";
		const auto fragFind = shaderModuleMap.find(fragPath);
		if (fragFind == shaderModuleMap.end()) {
			printf("creating fragment shader : %d \n", flags);
			Pipeline_Helper_Functions::CreateShaderModule(ShaderBlock::GetFragmentShader(flags), &fragShaderModule);
			shaderModuleMap.try_emplace(fragPath, fragShaderModule);
		}
		else {
			fragShaderModule = fragFind->second;
		}

		CreateGraphicsPipeline(configInfo);
	}

	EWEPipeline::EWEPipeline(std::string const& vertFilePath, MaterialFlags const flags, PipelineConfigInfo& configInfo) {

		{
			const auto vertModuleIter = shaderModuleMap.find(vertFilePath);
			if (vertModuleIter == shaderModuleMap.end()) {
				auto vertCode = Pipeline_Helper_Functions::ReadFile(vertFilePath);
				Pipeline_Helper_Functions::CreateShaderModule(vertCode, &vertShaderModule);
				shaderModuleMap.try_emplace(vertFilePath, vertShaderModule);
			}
			else {
				vertShaderModule = vertModuleIter->second;
			}
		}
#if DEBUGGING_MATERIAL_NORMALS
		const bool generatingNormals = flags & Material::Flags::GenerateNormals;
		if (generatingNormals) { //geometry stage
			configInfo.AddGeomShaderModule(flags);
		}
#endif
		{
#if DEBUGGING_MATERIAL_NORMALS
			std::string fragPath;// = SHADER_DIR;
			if (generatingNormals) {
				fragPath = "GenerateNormal.frag.spv";
			}
			else {
				fragPath = SHADER_DIR;
				fragPath += "dynamic/" + std::to_string(flags);
				if (flags & Material::Flags::Bones) {
					fragPath += "b";
				}
				fragPath += ".frag.spv";

			}
#else
			std::string fragPath = SHADER_DIR;
			fragPath += "dynamic/" + std::to_string(flags);
			if (flags & Material::Flags::Bones) {
				fragPath += "b";
			}
			fragPath += ".frag.spv";
#endif
			const auto fragModuleIter = shaderModuleMap.find(fragPath);
			if (fragModuleIter == shaderModuleMap.end()) {
#if DEBUGGING_MATERIAL_NORMALS
				if (generatingNormals) {
					auto fragCode = Pipeline_Helper_Functions::ReadFile(fragPath);
					Pipeline_Helper_Functions::CreateShaderModule(fragCode, &fragShaderModule);
					shaderModuleMap.try_emplace(fragPath, fragShaderModule);
				}
				else {
#endif
					Pipeline_Helper_Functions::CreateShaderModule(ShaderBlock::GetFragmentShader(flags), &fragShaderModule);
					//fragPath = SHADER_DIR + fragPath;
					shaderModuleMap.try_emplace(fragPath, fragShaderModule);
#if DEBUGGING_MATERIAL_NORMALS
				}
#endif

			}
			else {
				fragShaderModule = fragModuleIter->second;
			}
			
		}
		CreateGraphicsPipeline(configInfo);
	}

	EWEPipeline::~EWEPipeline() {
		EWE_VK(vkDestroyPipeline, VK::Object->vkDevice, graphicsPipeline, nullptr);
	}

	void EWEPipeline::Bind() {
		EWERenderer::BindGraphicsPipeline(graphicsPipeline);
	}

#if DEBUG_NAMING
	void EWEPipeline::SetDebugName(std::string const& name) {
		DebugNaming::SetObjectName(graphicsPipeline, VK_OBJECT_TYPE_PIPELINE, name.c_str());
	}
#endif

	void EWEPipeline::CreateGraphicsPipeline(const PipelineConfigInfo& configInfo) {

		assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout provided in configInfo");
		//assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderPass provided in configInfo");

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages{2 + static_cast<std::size_t>(configInfo.geomShaderModule != VK_NULL_HANDLE)};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertShaderModule;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;

		if (configInfo.geomShaderModule != VK_NULL_HANDLE) {
			auto& geomStage = shaderStages[1];
			geomStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			geomStage.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			geomStage.module = configInfo.geomShaderModule;
			geomStage.pName = "main";
			geomStage.flags = 0;
			geomStage.pNext = nullptr;
			geomStage.pSpecializationInfo = nullptr;
		}


		
		auto& fragStage = shaderStages[1 + static_cast<std::size_t>(configInfo.geomShaderModule != VK_NULL_HANDLE)];
		fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragStage.module = fragShaderModule;
		fragStage.pName = "main";
		fragStage.flags = 0;
		fragStage.pNext = nullptr;
		fragStage.pSpecializationInfo = nullptr;

		auto& bindingDescriptions = configInfo.bindingDescriptions;
		auto& attributeDescriptions = configInfo.attributeDescriptions;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.pNext = &configInfo.pipelineRenderingInfo;
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &configInfo.viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;

		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.subpass = configInfo.subpass;
#if PIPELINE_DERIVATIVES
		pipelineInfo.basePipelineIndex = configInfo.basePipelineIndex;
		if (configInfo.basePipelineHandle != nullptr) {
			pipelineInfo.basePipelineHandle = configInfo.basePipelineHandle->graphicsPipeline;
		}
		else {
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		}
		pipelineInfo.flags = configInfo.flags;
#endif
		EWE_VK(vkCreateGraphicsPipelines, VK::Object->vkDevice, configInfo.cache, 1, &pipelineInfo, nullptr, &graphicsPipeline);
	}

	void EWEPipeline::Enable2DConfig(PipelineConfigInfo& configInfo) {
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	}

	void EWEPipeline::DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {

		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		configInfo.viewportInfo.viewportCount = 1;
		configInfo.viewportInfo.pViewports = nullptr;
		configInfo.viewportInfo.scissorCount = 1;
		configInfo.viewportInfo.pScissors = nullptr;

		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		//configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
		configInfo.rasterizationInfo.lineWidth = 1.0f;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
		configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

		configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
		configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.front = {};  // Optional
		configInfo.depthStencilInfo.back = {};   // Optional

		configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
		configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
		configInfo.dynamicStateInfo.flags = 0;

		configInfo.pipelineRenderingInfo = *PipelineConfigInfo::pipelineRenderingInfoStatic;
		/*
		std::vector<VkVertexInputBindingDescription> bindingDescription(1);
		bindingDescription[0].binding = 0;                            // Binding index
		bindingDescription[0].stride = 0;                             // No per-vertex data
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Input rate

		// Define the vertex input attribute description
		std::vector<VkVertexInputAttributeDescription> attributeDescription(1);
		attributeDescription[0].binding = 0;        // Binding index (should match the binding description)
		attributeDescription[0].location = 0;       // Location in the vertex shader
		attributeDescription[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Format of the attribute data
		attributeDescription[0].offset = 0;

		configInfo.bindingDescriptions = bindingDescription;
		configInfo.attributeDescriptions = attributeDescription;
		*/
	}

	void EWEPipeline::EnableAlphaBlending(PipelineConfigInfo& configInfo) {
		configInfo.colorBlendAttachment.blendEnable = VK_TRUE;

		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

		//may want to change these 3 later
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	void EWEPipeline::PipelineConfigInfo::AddGeomShaderModule(std::string const& geomFilepath) {

		const auto geomFind = shaderModuleMap.find(geomFilepath);
		if (geomFind == shaderModuleMap.end()) {
			auto geomCode = Pipeline_Helper_Functions::ReadFile(geomFilepath);
			Pipeline_Helper_Functions::CreateShaderModule(geomCode, &geomShaderModule);
			shaderModuleMap.try_emplace(geomFilepath, geomShaderModule);

		}
		else {
			geomShaderModule = geomFind->second;
		}
	}
	void EWEPipeline::PipelineConfigInfo::AddGeomShaderModule(const MaterialFlags flags) {
		std::string geomFilepath = SHADER_DIR;
		geomFilepath += "dynamic/" + std::to_string(flags);
		if (flags & Material::Flags::Bones) {
			geomFilepath += "b";
		}
		geomFilepath += ".geom.spv";

		const auto geomFind = shaderModuleMap.find(geomFilepath);
		if (geomFind == shaderModuleMap.end()) {
			Pipeline_Helper_Functions::CreateShaderModule(ShaderBlock::GetGeometryShader(flags), &geomShaderModule);
			//fragPath = SHADER_DIR + fragPath;
			shaderModuleMap.try_emplace(geomFilepath, geomShaderModule);

		}
		else {
			geomShaderModule = geomFind->second;
		}
	}
}