#include "EWEngine/Graphics/Pipeline.h"


#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Renderer.h"
#include "EWEngine/Graphics/PushConstants.h"


#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

#ifndef SHADER_DIR
#define SHADER_DIR "shaders\\"
#endif

namespace EWE {

	namespace Pipeline_Helper_Functions {
		std::vector<char> readFile(const std::string& filepath) {
			//#define ENGINE_DIR "..//shaders//"

			std::string enginePath = SHADER_DIR + filepath;

			//printf("readFile enginePath : %s \n", enginePath.c_str());

			std::ifstream shaderFile;
			shaderFile.open(enginePath, std::ios::binary);
			if(!shaderFile.is_open()){
				throw std::runtime_error("failed to open shader file");
			}
			shaderFile.seekg(0, std::ios::end);
			size_t fileSize = (size_t)shaderFile.tellg();
			if(fileSize == 0){
				throw std::runtime_error("shader file length is 0");
			}
			shaderFile.seekg(0, std::ios::beg);
			std::vector<char> returnVec(fileSize);
			shaderFile.read(returnVec.data(), fileSize);
			shaderFile.close();
			return returnVec;
		}
		void createShaderModule(EWEDevice& device, std::string const& file_path, VkShaderModule* shaderModule) {
			auto data = readFile(file_path);
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = data.size();
			//printf("template data size : %d \n", data.size());
			createInfo.pCode = (const uint32_t*)data.data();

			if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module");
			}
		}
		void createShaderModule(EWEDevice& device, const std::vector<uint32_t>& data, VkShaderModule* shaderModule) {
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = data.size() * 4;
			//printf("uint32_t data size : %d \n", data.size());
			createInfo.pCode = data.data();

			if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module");
			}
		}
		template <typename T>
		void createShaderModule(EWEDevice& device, const std::vector<T>& data, VkShaderModule* shaderModule) {
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = data.size() * sizeof(T);
			//printf("template data size : %d \n", data.size());
			createInfo.pCode = (const uint32_t*)data.data();

			if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module");
			}
		}
		void createShaderModule(EWEDevice& device, const void* data, size_t dataSize, VkShaderModule* shaderModule) {
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = dataSize;
			//printf("uint32_t data size : %d \n", data.size());
			createInfo.pCode = reinterpret_cast<const uint32_t*>(data);
			VkResult vkResult = vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule);
			if (vkResult != VK_SUCCESS) {
				printf("vkResult : %d \n", vkResult);
				throw std::runtime_error("failed to create shader module");
			}
		}
	}

	// ~~~~~~~ COMPUTE PIPELINE ~~~~~~~~~~~~~~~
	EWE_Compute_Pipeline EWE_Compute_Pipeline::createPipeline(EWEDevice& device, std::vector<VkDescriptorSetLayout> computeDSL, std::string compute_path) {
		EWE_Compute_Pipeline ret;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(computeDSL.size());
		pipelineLayoutInfo.pSetLayouts = computeDSL.data();

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &ret.pipe_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline layout!");
		}

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = ret.pipe_layout;

		std::string computePath = "compute/";
		computePath += compute_path;
		Pipeline_Helper_Functions::createShaderModule(device, computePath, &ret.shader);
		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = ret.shader;
		computeShaderStageInfo.pName = "main";
		pipelineInfo.stage = computeShaderStageInfo;
		vkCreateComputePipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ret.pipeline);
		return ret;
	}
	EWE_Compute_Pipeline EWE_Compute_Pipeline::createPipeline(EWEDevice& device, VkPipelineLayout pipe_layout, std::string compute_path) {
		EWE_Compute_Pipeline ret;
		ret.pipe_layout = pipe_layout;

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = ret.pipe_layout;
		Pipeline_Helper_Functions::createShaderModule(device, compute_path, &ret.shader);
		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = ret.shader;
		computeShaderStageInfo.pName = "main";
		pipelineInfo.stage = computeShaderStageInfo;
		vkCreateComputePipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ret.pipeline);
		return ret;
	}

	// ~~~~~~~~~~~~~~~~~~~~ END COMPUTE PIPELINE ~~~~~~~~~~~~~~~~~~~~~~

	std::map<std::string, VkShaderModule> EWEPipeline::shaderModuleMap;

	EWEPipeline::EWEPipeline(EWEDevice& device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo) : eweDevice{ device } {
		if (shaderModuleMap.find(vertFilepath) == shaderModuleMap.end()) {
			auto vertCode = Pipeline_Helper_Functions::readFile(vertFilepath);
			Pipeline_Helper_Functions::createShaderModule(eweDevice, vertCode, &vertShaderModule);
			shaderModuleMap[vertFilepath] = vertShaderModule;
		}
		else {
			vertShaderModule = shaderModuleMap[vertFilepath];
		}
		if (shaderModuleMap.find(fragFilepath) == shaderModuleMap.end()) {
			auto fragCode = Pipeline_Helper_Functions::readFile(fragFilepath);
			Pipeline_Helper_Functions::createShaderModule(eweDevice, fragCode, &fragShaderModule);
			shaderModuleMap[fragFilepath] = fragShaderModule;
		}
		else {
			fragShaderModule = shaderModuleMap[fragFilepath];
		}
		createGraphicsPipeline(configInfo);
	}
	EWEPipeline::EWEPipeline(EWEDevice& device, VkShaderModule vertShaderModu, VkShaderModule fragShaderModu, const PipelineConfigInfo& configInfo) : eweDevice{ device }, vertShaderModule{ vertShaderModu }, fragShaderModule{ fragShaderModu } {
		createGraphicsPipeline(configInfo);
	}

	EWEPipeline::EWEPipeline(EWEDevice& device, uint16_t boneCount, MaterialFlags flags, const PipelineConfigInfo& configInfo) : eweDevice{ device } {
		std::string vertPath = SHADER_DIR;
		//this is always instanced???
		bool hasNormal = (flags & DynF_hasNormal) > 0;
		if (hasNormal) {
			vertPath += "dynamic\\n" + std::to_string(boneCount) + ".vert.spv";
		}
		else {
			vertPath += "dynamic\\" + std::to_string(boneCount) + ".vert.spv";
		}
		if (shaderModuleMap.find(vertPath) == shaderModuleMap.end()) {
			printf("creating vertex shader - %d:%d \n", boneCount, flags);
			//auto vertCode = readFile(vertPath);
			Pipeline_Helper_Functions::createShaderModule(eweDevice, ShaderBlock::getVertexShader(hasNormal, boneCount, true), &vertShaderModule);
			shaderModuleMap[vertPath] = vertShaderModule;
		}
		else {
			vertShaderModule = shaderModuleMap[vertPath];
		}
		std::string fragPath = SHADER_DIR;
		fragPath += "dynamic\\" + std::to_string(flags) + "b.frag.spv";
		if (shaderModuleMap.find(fragPath) == shaderModuleMap.end()) {
			printf("creating fragment shader : %d \n", flags);
			Pipeline_Helper_Functions::createShaderModule(eweDevice, ShaderBlock::getFragmentShader(flags, true), &fragShaderModule);
			//fragPath = ENGINE_DIR + fragPath; //wtf
			shaderModuleMap[fragPath] = fragShaderModule;
		}
		else {
			fragShaderModule = shaderModuleMap[fragPath];
		}

		createGraphicsPipeline(configInfo);
	}

	EWEPipeline::EWEPipeline(EWEDevice& device, const std::string& vertFilePath, MaterialFlags flags, const PipelineConfigInfo& configInfo, bool hasBones) : eweDevice{ device } {

		if (shaderModuleMap.find(vertFilePath) == shaderModuleMap.end()) {
			auto vertCode = Pipeline_Helper_Functions::readFile(vertFilePath);
			Pipeline_Helper_Functions::createShaderModule(eweDevice, vertCode, &vertShaderModule);
			shaderModuleMap[vertFilePath] = vertShaderModule;
		}
		else {
			vertShaderModule = shaderModuleMap[vertFilePath];
		}
		std::string fragPath = SHADER_DIR;
		fragPath += "dynamic\\" + std::to_string(flags);
		if (hasBones) {
			fragPath += "b";
		}
		fragPath += ".frag.spv";
		if (shaderModuleMap.find(fragPath) == shaderModuleMap.end()) {
			Pipeline_Helper_Functions::createShaderModule(eweDevice, ShaderBlock::getFragmentShader(flags, hasBones), &fragShaderModule);
			fragPath = SHADER_DIR + fragPath;
			shaderModuleMap[fragPath] = fragShaderModule;
		}
		else {
			fragShaderModule = shaderModuleMap[fragPath];
		}

		createGraphicsPipeline(configInfo);
	}

	EWEPipeline::~EWEPipeline() {
		//MIGHT DESTROY THE SAME MODULE TWICE WHILE USING THE MAP COPY

		//vkDestroyShaderModule(eweDevice.device(), vertShaderModule, nullptr);
		//vkDestroyShaderModule(eweDevice.device(), fragShaderModule, nullptr);
		vkDestroyPipeline(eweDevice.device(), graphicsPipeline, nullptr);
	}

	void EWEPipeline::bind(VkCommandBuffer commandBuffer) {
		EWERenderer::bindGraphicsPipeline(commandBuffer, graphicsPipeline);
	}

	void EWEPipeline::createGraphicsPipeline(const PipelineConfigInfo& configInfo) {

		assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout provided in configInfo");
		//assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderPass provided in configInfo");

		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertShaderModule;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;


		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		auto& bindingDescriptions = configInfo.bindingDescriptions;
		auto& attributeDescriptions = configInfo.attributeDescriptions;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		if (attributeDescriptions.size() > 0) {
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		}
		else {
			vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		}
		if (bindingDescriptions.size() > 0) {
			vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		}
		else {
			vertexInputInfo.pVertexBindingDescriptions = nullptr;
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.pNext = &configInfo.pipelineRenderingInfo;
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
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
		if (vkCreateGraphicsPipelines(eweDevice.device(), configInfo.cache, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline");
		}


	}

	void EWEPipeline::enable2DConfig(PipelineConfigInfo& configInfo) {
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	}

	void EWEPipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {

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

		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
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

	void EWEPipeline::enableAlphaBlending(PipelineConfigInfo& configInfo) {
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

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PIPELINE MANAGER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	/*
	std::map<PipeLayout_Enum, VkPipelineLayout> PipelineManager::pipeLayouts;
	std::map<Pipeline_Enum, std::unique_ptr<EWEPipeline>> PipelineManager::pipelines;

	VkPipelineLayout PipelineManager::dynamicMaterialPipeLayout[DYNAMIC_PIPE_LAYOUT_COUNT];

	VkShaderModule PipelineManager::loadingVertShaderModule{ VK_NULL_HANDLE };
	VkShaderModule PipelineManager::loadingFragShaderModule{ VK_NULL_HANDLE };
	std::unique_ptr<EWEPipeline> PipelineManager::loadingPipeline{ nullptr };
#ifdef _DEBUG
	std::vector<uint8_t> PipelineManager::dynamicBonePipeTracker;
	std::vector<std::pair<uint16_t, MaterialFlags>> PipelineManager::dynamicInstancedPipeTracker;
#endif

	VkPipelineCache PipelineManager::materialPipelineCache = VK_NULL_HANDLE;
	VkPipelineCache PipelineManager::boneMaterialPipelineCache = VK_NULL_HANDLE;
	VkPipelineCache PipelineManager::instanceMaterialPipelineCache = VK_NULL_HANDLE;
	std::map<uint8_t, std::unique_ptr<EWEPipeline>> PipelineManager::dynamicMaterialPipeline;

	void PipelineManager::initDynamicPipeLayout(uint16_t dynamicPipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced, EWEDevice& device) {
		//layouts
		//textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2))
		if (dynamicMaterialPipeLayout[dynamicPipeLayoutIndex] == VK_NULL_HANDLE) {


			std::vector<VkDescriptorSetLayout>* tempDescLayout = DescriptorHandler::getDynamicPipeDescSetLayout(textureCount, hasBones, instanced, device);


			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			VkPushConstantRange pushConstantRange{};
			if (instanced) {
				pipelineLayoutInfo.pPushConstantRanges = nullptr;
				pipelineLayoutInfo.pushConstantRangeCount = 0;
			}
			else {
				pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				pushConstantRange.offset = 0;
				if (hasBones) {
					pushConstantRange.size = sizeof(PlayerPushConstantData);
				}
				else {
					pushConstantRange.size = sizeof(SimplePushConstantData);
				}
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
				pipelineLayoutInfo.pushConstantRangeCount = 1;
			}

			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDescLayout->size());
			pipelineLayoutInfo.pSetLayouts = tempDescLayout->data();

			printf("creating dynamic pipe layout with index : %d \n", textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2)));

			VkResult vkResult = vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &dynamicMaterialPipeLayout[textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2))]);
			if (vkResult != VK_SUCCESS) {
				printf("failed to create dynamic mat pipelayout layout [%d] \n", textureCount);
				throw std::runtime_error("Failed to create dynamic mat pipe layout \n");
			}
		}
	}

	void PipelineManager::updateMaterialPipe(MaterialFlags flags, VkPipelineRenderingCreateInfo const& pipeRenderInfo, EWEDevice& device) {
		bool hasBones = flags & 128;
		bool instanced = flags & 64; //curently creating an outside manager to deal with instanced skinned meshes
#ifdef _DEBUG
		if (hasBones || instanced) {
			printf("creating a material pipe with bones or instanced flag set, no longer supported \n");
			throw std::exception("creating a material pipe with bones or instanced flag set, no longer supported");
		}
#endif

		//bool finalSlotBeforeNeedExpansion = MaterialFlags & 32;
		bool hasBumps = flags & 16;
		bool hasNormal = flags & 8;
		bool hasRough = flags & 4;
		bool hasMetal = flags & 2;
		bool hasAO = flags & 1;

		uint8_t textureCount = hasNormal + hasRough + hasMetal + hasAO + hasBumps;
		uint16_t pipeLayoutIndex = textureCount;
		printf("textureCount, hasBones, instanced - %d:%d:%d \n", textureCount, hasBones, instanced);

#ifdef _DEBUG
		if (textureCount == 0) {
			//undesirable, but not quite a bug. only passing in an albedo texture is valid
			printf("material pipeline, flags textureCount is 0 wtf \n");
		}
#endif

		initDynamicPipeLayout(pipeLayoutIndex, textureCount, false, false, device);



		//printf("creating new pipeline, dynamicShaderFinding, (key value:%d)-(bones:%d)-(normal:%d)-(rough:%d)-(metal:%d)-(ao:%d) \n", newFlags, hasBones, hasNormal, hasRough, hasMetal, hasAO );
		if (dynamicMaterialPipeline.find(flags) == dynamicMaterialPipeline.end()) {
			EWEPipeline::PipelineConfigInfo pipelineConfig{};
			EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
			pipelineConfig.pipelineRenderingInfo = pipeRenderInfo;
			pipelineConfig.pipelineLayout = dynamicMaterialPipeLayout[pipeLayoutIndex];

			if (materialPipelineCache == VK_NULL_HANDLE) {
				VkPipelineCacheCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

				if (vkCreatePipelineCache(device.device(), &createInfo, nullptr, &materialPipelineCache) != VK_SUCCESS) {
					// handle error
					printf("failed to create material pipeline cache \n");
					throw std::runtime_error("failed to create material pipeline cache");
				}
				else {
					printf("material pipe line cache creating \n");
				}
#if PIPELINE_DERIVATIVES
				pipelineConfig.basePipelineHandle = nullptr;
				pipelineConfig.basePipelineIndex = -1;
				pipelineConfig.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
				daddyPipeKey = newFlags;
#endif
			}
#if PIPELINE_DERIVATIVES
			else {
				pipelineConfig.basePipelineHandle = dynamicMaterialPipeline[daddyPipeKey].get();
				pipelineConfig.basePipelineIndex = -1;
				pipelineConfig.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
			}
#endif


			pipelineConfig.cache = materialPipelineCache;
			if (hasBumps) {
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<AVertex>();
				pipelineConfig.attributeDescriptions = AVertex::getAttributeDescriptions();

				dynamicMaterialPipeline.emplace(flags, std::make_unique<EWEPipeline>(device, "material_bump.vert.spv", flags, pipelineConfig, false));
			}
			else if (hasNormal) {
				//printf("AVertex, flags:%d \n", newFlags);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<AVertex>();
				pipelineConfig.attributeDescriptions = AVertex::getAttributeDescriptions();
				dynamicMaterialPipeline.emplace(flags, std::make_unique<EWEPipeline>(device, "material_Tangent.vert.spv", flags, pipelineConfig, false));
			}
			else {
				//printf("AVertexNT, flags:%d \n", newFlags);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<AVertexNT>();
				pipelineConfig.attributeDescriptions = AVertexNT::getAttributeDescriptions();
				dynamicMaterialPipeline.emplace(flags, std::make_unique<EWEPipeline>(device, "material_nn.vert.spv", flags, pipelineConfig, false));
			}
			

			//printf("after dynamic shader finding \n");
		}
	}

	std::unique_ptr<EWEPipeline> PipelineManager::createInstancedRemote(MaterialFlags flags, uint16_t boneCount, VkPipelineRenderingCreateInfo const& pipeRenderInfo, EWEDevice& device) {

#ifdef _DEBUG
		for (int i = 0; i < dynamicInstancedPipeTracker.size(); i++) {
			if (flags == dynamicInstancedPipeTracker[i].second) {
				if (dynamicInstancedPipeTracker[i].first == boneCount) {
					printf("double created remote pipeline, throwing error \n");
					throw std::exception("double created remote pipeline");
				}
			}
		}
		dynamicInstancedPipeTracker.emplace_back(boneCount, flags);
#endif
		//bool finalSlotBeforeNeedExpansion = MaterialFlags & 32;
		bool hasBumps = flags & DynF_hasBump;
		bool hasNormal = flags & DynF_hasNormal;
		bool hasRough = flags & DynF_hasRough;
		bool hasMetal = flags & DynF_hasMetal;
		bool hasAO = flags & DynF_hasAO;

		uint8_t textureCount = hasNormal + hasRough + hasMetal + hasAO + hasBumps;

		if (hasBumps) {
			printf("HAS BONES AND BUMP, SHOULD NOT HAPPEN \n");
		}

		uint16_t pipeLayoutIndex = textureCount + (MAX_MATERIAL_TEXTURE_COUNT * 3);
		initDynamicPipeLayout(pipeLayoutIndex, textureCount, true, true, device);


		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.pipelineRenderingInfo = pipeRenderInfo;
		pipelineConfig.pipelineLayout = dynamicMaterialPipeLayout[pipeLayoutIndex];


		printf("initiating remote instanced pipeline : %d \n", flags);

		if (instanceMaterialPipelineCache == VK_NULL_HANDLE) {
			VkPipelineCacheCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			if (vkCreatePipelineCache(device.device(), &createInfo, nullptr, &instanceMaterialPipelineCache) != VK_SUCCESS) {
				// handle error
				printf("failed to create instance material pipeline cache \n");
				throw std::runtime_error("failed to create material pipeline cache");
			}
		}
		pipelineConfig.cache = instanceMaterialPipelineCache;

		//printf("boneVertex, flags:%d \n", newFlags);
		pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<boneVertex>();
		pipelineConfig.attributeDescriptions = boneVertex::getAttributeDescriptions();
		glslang::InitializeProcess();
		return std::make_unique<EWEPipeline>(device, boneCount, flags, pipelineConfig);
		glslang::FinalizeProcess();
		
	}

	std::unique_ptr<EWEPipeline> PipelineManager::createBoneRemote(MaterialFlags flags, VkPipelineRenderingCreateInfo const& pipeRenderInfo, EWEDevice& device) {
#ifdef _DEBUG
		for (int i = 0; i < dynamicBonePipeTracker.size(); i++) {
			if (flags == dynamicBonePipeTracker[i]) {
				printf("double created remote bone pipeline, throwing error \n");
				throw std::exception("double created remote bone pipeline");
			}
		}
		dynamicBonePipeTracker.emplace_back(flags);
		//remotePipelines.emplace_back(boneCount, flags);
#endif

		//bool finalSlotBeforeNeedExpansion = MaterialFlags & 32;
		bool hasBumps = flags & DynF_hasBump;
		bool hasNormal = flags & DynF_hasNormal;
		bool hasRough = flags & DynF_hasRough;
		bool hasMetal = flags & DynF_hasMetal;
		bool hasAO = flags & DynF_hasAO;

		uint8_t textureCount = hasNormal + hasRough + hasMetal + hasAO + hasBumps;

		if (hasBumps) {
			printf("HAS BONES AND BUMP, SHOULD NOT HAPPEN \n");
			throw std::runtime_error("currently not supporting skinned meshes with bump maps");
		}

		uint16_t pipeLayoutIndex = textureCount + MAX_MATERIAL_TEXTURE_COUNT;
		initDynamicPipeLayout(pipeLayoutIndex, textureCount, true, false, device);


		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.pipelineRenderingInfo = pipeRenderInfo;
		pipelineConfig.pipelineLayout = dynamicMaterialPipeLayout[pipeLayoutIndex];


		printf("initiating remote bone pipeline : %d \n", flags);

		if (boneMaterialPipelineCache == VK_NULL_HANDLE) {
			VkPipelineCacheCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			if (vkCreatePipelineCache(device.device(), &createInfo, nullptr, &boneMaterialPipelineCache) != VK_SUCCESS) {
				// handle error
				printf("failed to create bone material pipeline cache \n");
				throw std::runtime_error("failed to create bone material pipeline cache");
			}
		}
		pipelineConfig.cache = boneMaterialPipelineCache;

		glslang::InitializeProcess();
		if (hasNormal) {
			//printf("boneVertex, flags:%d \n", newFlags);
			pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<boneVertex>();
			pipelineConfig.attributeDescriptions = boneVertex::getAttributeDescriptions();
			return std::make_unique<EWEPipeline>(device, "bone_Tangent.vert.spv", flags, pipelineConfig, true);
		}
		else {
			//printf("boneVertexNT, flags:%d \n", newFlags);
			pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<boneVertexNoTangent>();
			pipelineConfig.attributeDescriptions = boneVertexNoTangent::getAttributeDescriptions();
			return std::make_unique<EWEPipeline>(device, "bone_NT.vert.spv", flags, pipelineConfig, true);
		}
		glslang::FinalizeProcess();

	}

	void PipelineManager::createLoadingPipeline(EWEDevice& device, VkPipelineRenderingCreateInfo const& pipeRenderInfo) {
		if (loadingPipeline.get() != nullptr) {
			printf("trying to recreate the loading pipeline \n");
			return;
		}

		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.pipelineRenderingInfo = pipeRenderInfo;
		pipelineConfig.pipelineLayout = getPipelineLayout(PL_loading, device);
		pipelineConfig.bindingDescriptions = LeafVertex::getBindingDescriptions();
		pipelineConfig.attributeDescriptions = LeafVertex::getAttributeDescriptions();

		printf("before loading vert shader \n");
		glslang::InitializeProcess();
		Pipeline_Helper_Functions::createShaderModule(device, ShaderBlock::getLoadingVertShader(), &loadingVertShaderModule);

		printf("before loading frag shader \n");
		Pipeline_Helper_Functions::createShaderModule(device, ShaderBlock::getLoadingFragShader(), &loadingFragShaderModule);
		printf("after loading creation shaders \n");
		glslang::FinalizeProcess();

		loadingPipeline = std::make_unique<EWEPipeline>(device, loadingVertShaderModule, loadingFragShaderModule, pipelineConfig);
	}

	VkPipelineLayout PipelineManager::getPipelineLayout(PipeLayout_Enum ple, EWEDevice& eweDevice) {
		//if (pipelineLayouts[ple] == VK_NULL_HANDLE) { //this initiates the layout, which may not be equal to VK_NULL_HANDLE at initiation (probably wont -> definitiely wont)
		//doing it like this because i have pipelayouts that are used in multiple pipelines
		if (pipeLayouts.find(ple) != pipeLayouts.end()) {
			return pipeLayouts[ple];
		}
		else {

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

			VkPushConstantRange pushConstantRange{};
			pushConstantRange.offset = 0;
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

#ifdef _DEBUG
			printf("finna create pipeline layout : %d \n", ple);
#endif
			switch (ple) {
#if DRAWING_POINTS
			case PL_pointLight: {
				pushConstantRange.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstantRange.size = sizeof(PointLightPushConstants);
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
				pipelineLayoutInfo.pushConstantRangeCount = 1;
				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_pointLight, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				if (vkCreatePipelineLayout(eweDevice.device(), &pipelineLayoutInfo, nullptr, &pipeLayouts[PL_pointLight]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create pipeline layout");
				}
				break;
			}
#endif
			case PL_spikyBall: {
				pushConstantRange.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstantRange.size = sizeof(ModelTimePushData);
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
				pipelineLayoutInfo.pushConstantRangeCount = 1;
				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_global, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				break;
			}
			case PL_grass: {
				pushConstantRange.size = sizeof(UVScrollingPushData);
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
				pipelineLayoutInfo.pushConstantRangeCount = 1;

				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_grass, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();

				break;
			}
			case PL_lightning: {

				pushConstantRange.size = sizeof(LightningPushConstants);
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_global, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				break;
			}
			case PL_skybox: {
				pipelineLayoutInfo.pushConstantRangeCount = 0;
				pipelineLayoutInfo.pPushConstantRanges = nullptr;
				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_textured, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				break;
			}
			case PL_textured: {
				pushConstantRange.size = sizeof(SimplePushConstantData);
				pipelineLayoutInfo.pushConstantRangeCount = 1;
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

				//pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_textured, eweDevice);
				//printf("tempDSL size : %d \n", tempDSL->size());
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				break;
			}

			case PL_boned: {
				pushConstantRange.size = sizeof(SimplePushConstantData);
				pipelineLayoutInfo.pushConstantRangeCount = 1;
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_boned, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				break;
			}
			case PL_2d: {
				pushConstantRange.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstantRange.size = sizeof(Simple2DPushConstantData);

				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_2d, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				//printf("before 2d PL \n");
				//printf("after 2d PL \n");
				break;
			}
			case PL_nineUI: {
				pushConstantRange.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstantRange.size = sizeof(NineUIPushConstantData);

				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_2d, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();

				//printf("before nine UI PL \n");
				//printf("after nine UI PL \n");
				break;
			}
			case PL_sprite: {
				pushConstantRange.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstantRange.size = sizeof(SpritePushConstantData);
				pipelineLayoutInfo.pushConstantRangeCount = 1;
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

				//std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_sprite, eweDevice);
				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_textured, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();

				break;
			}
			case PL_orbOverlay: {
				
				pushConstantRange.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstantRange.size = sizeof(HPContainerPushData);
				pipelineLayoutInfo.pushConstantRangeCount = 1;
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

				//std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_sprite, eweDevice);
				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_orbOverlay, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				//printf("before orb overlay PL \n");
				//printf("after orb overlay PL \n");
				break;
			}
			case PL_ExpBar: {

				pushConstantRange.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstantRange.size = sizeof(ExpBarPushData);
				pipelineLayoutInfo.pushConstantRangeCount = 1;
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

				//std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_sprite, eweDevice);
				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_2d, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				//printf("before orb overlay PL \n");
				//printf("after exp bar overlay PL \n");
				break;
			}
			case PL_castleHealth: {

				pushConstantRange.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstantRange.size = sizeof(CastleHealthPushData);
				pipelineLayoutInfo.pushConstantRangeCount = 1;
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

				//std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_sprite, eweDevice);
				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_2d, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				//printf("before orb overlay PL \n");
				//printf("after exp bar overlay PL \n");
				break;
			}

			case PL_visualEffect: {
				pushConstantRange.size = sizeof(ModelPushData);
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_visualEffect, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				break;
			}
			case PL_loading: {
				//pushConstantRange.size = 0;
				pipelineLayoutInfo.pushConstantRangeCount = 0;
				pipelineLayoutInfo.pPushConstantRanges = nullptr;

				std::vector<VkDescriptorSetLayout>* tempDSL = DescriptorHandler::getPipeDescSetLayout(PDSL_loading, eweDevice);
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL->size());
				pipelineLayoutInfo.pSetLayouts = tempDSL->data();
				break;
			}
			default: {
				printf("??? trying to create a pipeline layout that doesnt have support ??? \n");
				throw std::runtime_error("invalid pipeline layout creation");
				break; 
			}

			}

			if (vkCreatePipelineLayout(eweDevice.device(), &pipelineLayoutInfo, nullptr, &pipeLayouts[ple]) != VK_SUCCESS) {
				printf("failed to create %d pipe layout \n", ple);
				throw std::runtime_error("Failed to create pipe layout \n");
			}
#if false//def _DEBUG

			printf("pipeline layout was created : %d \n", ple);
#endif
			return pipeLayouts[ple];
		}
	}

	void PipelineManager::initPipelines(VkPipelineRenderingCreateInfo const& pipeRenderInfo, Pipeline_Enum pipeNeeded, EWEDevice& eweDevice) {

		if (pipelines.find(pipeNeeded) != pipelines.end()) {
			return;
		}
		else {
#ifdef false//_DEBUG
			printf("finna create pipeline : %d \n", pipeNeeded);
#endif

			EWEPipeline::PipelineConfigInfo pipelineConfig{};
			EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
			pipelineConfig.pipelineRenderingInfo = pipeRenderInfo;

			std::string vertString;
			std::string fragString;
			switch (pipeNeeded) {
			case Pipe_pointLight: {

				//printf("before point light pipeline \n");
				// next pipelin \/
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_pointLight, eweDevice);
				vertString = "point_light.vert.spv";
				fragString = "point_light.frag.spv";
				break;
			}
			case Pipe_spikyBall: {

				pipelineConfig.pipelineLayout = getPipelineLayout(PL_spikyBall, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<simpleVertex>();
				pipelineConfig.attributeDescriptions = simpleVertex::getAttributeDescriptions();

				vertString = "spikyball.vert.spv";
				fragString = "spikyball.frag.spv";

				break;
			}
			case Pipe_grass: {
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_grass, eweDevice);
				pipelineConfig.bindingDescriptions = GrassVertex::getBindingDescriptions();
				pipelineConfig.attributeDescriptions = GrassVertex::getAttributeDescriptions();

				vertString = "grassField.vert.spv";
				fragString = "grassField.frag.spv";
				break;
			}
			case Pipe_lightning: {
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_lightning, eweDevice);
				pipelineConfig.bindingDescriptions = {};
				pipelineConfig.attributeDescriptions = {};

				vertString = "lightning.vert.spv";
				fragString = "lightning.frag.spv";
				break;
			}
			case Pipe_skybox: {
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_skybox, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<skyVertex>();
				pipelineConfig.attributeDescriptions = skyVertex::getAttributeDescriptions();

				vertString = "skybox.vert.spv";
				fragString = "skybox.frag.spv";

				break;
			}
			case Pipe_textured: {
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_textured, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<Vertex>();
				pipelineConfig.attributeDescriptions = Vertex::getAttributeDescriptions();

				vertString = "texture_shader.vert.spv";
				fragString = "texture_shader.frag.spv";
				break;
			}
			case Pipe_2d: {

				EWEPipeline::enableAlphaBlending(pipelineConfig);
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_2d, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<VertexUI>();
				pipelineConfig.attributeDescriptions = VertexUI::getAttributeDescriptions();
				vertString = "2d.vert.spv";
				fragString = "2d.frag.spv";
				break;
			}
			case Pipe_NineUI: {
				EWEPipeline::enableAlphaBlending(pipelineConfig);
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_nineUI, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<VertexUI>();
				pipelineConfig.attributeDescriptions = VertexUI::getAttributeDescriptions();
				//maybe i should cache UI into here
				//printf("before nineui pipe \n");
				vertString = "NineUI.vert.spv";
				fragString = "NineUI.frag.spv";
				//printf("after nineui pipe \n");
				break;
			}
			case Pipe_alpha: {
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_textured, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<Vertex>();
				pipelineConfig.attributeDescriptions = Vertex::getAttributeDescriptions();
				EWEPipeline::enableAlphaBlending(pipelineConfig);
				vertString = "texture_alpha.vert.spv";
				fragString = "texture_alpha.frag.spv";
				break;
			}
			case Pipe_sprite: {

				pipelineConfig.pipelineLayout = getPipelineLayout(PL_sprite, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<AVertexNT>();
				pipelineConfig.attributeDescriptions = AVertexNT::getAttributeDescriptions();
				EWEPipeline::enableAlphaBlending(pipelineConfig);
				vertString = "sprite.vert.spv";
				fragString = "sprite.frag.spv";
				break;
			}
			case Pipe_orbOverlay: {

				pipelineConfig.pipelineLayout = getPipelineLayout(PL_orbOverlay, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<VertexUI>();
				pipelineConfig.attributeDescriptions = VertexUI::getAttributeDescriptions();
				EWEPipeline::enableAlphaBlending(pipelineConfig);
				//printf("before orb pipe \n");
				vertString = "HPContainer.vert.spv";
				fragString = "HPContainer.frag.spv";

				//printf("after orb pipe \n");
				break;
			}
			case Pipe_ExpBar: {

				pipelineConfig.pipelineLayout = getPipelineLayout(PL_ExpBar, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<VertexUI>();
				pipelineConfig.attributeDescriptions = VertexUI::getAttributeDescriptions();
				EWEPipeline::enableAlphaBlending(pipelineConfig);
				//printf("before orb pipe \n");
				vertString = "ExpContainer.vert.spv";
				fragString = "ExpContainer.frag.spv";
				//printf("after orb pipe \n");
				break;
			}
			case Pipe_castleHealth: {
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_castleHealth, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<VertexUI>();
				pipelineConfig.attributeDescriptions = VertexUI::getAttributeDescriptions();
				EWEPipeline::enableAlphaBlending(pipelineConfig);
				//printf("before orb pipe \n");
				vertString = "CastleHealth.vert.spv";
				fragString = "CastleHealth.frag.spv";
				//printf("after orb pipe \n");
				break;
			}

			case Pipe_visualEffect: {
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_visualEffect, eweDevice);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<EffectVertex>();
				pipelineConfig.attributeDescriptions = EffectVertex::getAttributeDescriptions();
				vertString = "visualEffect.vert.spv";
				fragString = "visualEffect.frag.spv";
				break;
			}

			case Pipe_grid: {
				pipelineConfig.pipelineLayout = getPipelineLayout(PL_textured, eweDevice);

				EWEPipeline::enableAlphaBlending(pipelineConfig);
				pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
				pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;

				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<Vertex>();
				pipelineConfig.attributeDescriptions = Vertex::getAttributeDescriptions();
				vertString = "texture_alpha.vert.spv";
				fragString = "texture_alpha.frag.spv";
				break;
			}
			default: {
				printf("trying to create a pipeline that doesnt have support??? \n");
				throw std::runtime_error("invalid pipeline construction");
			}
#if false//_DEBUG
						  printf("after creating pipe : %d \n", pipeNeeded);
#endif
			}



			pipelines.emplace(pipeNeeded, std::make_unique<EWEPipeline>(eweDevice, vertString, fragString, pipelineConfig));
		}
	}

	void PipelineManager::cleanupStaticVariables(EWEDevice& device) {

#if DECONSTRUCTION_DEBUG
		printf("begin deconstructing pipeline manager \n");
#endif
		for (int i = 0; i < DYNAMIC_PIPE_LAYOUT_COUNT; i++) {
			if (dynamicMaterialPipeLayout[i] != VK_NULL_HANDLE) {
				vkDestroyPipelineLayout(device.device(), dynamicMaterialPipeLayout[i], nullptr);
			}
		}
#ifdef _DEBUG
		for (int i = 0; i < PL_MAX_COUNT; i++) {
			if (pipeLayouts.find((PipeLayout_Enum)i) == pipeLayouts.end()) {
				printf("pipelayout : %d was never used! \n", i);
			}
		}
#endif
		for (auto iter = pipeLayouts.begin(); iter != pipeLayouts.end(); iter++) {
			vkDestroyPipelineLayout(device.device(), iter->second, nullptr);
		}
		pipeLayouts.clear();
		for (int i = 0; i < Pipe_MAX_COUNT; i++) {
			if (pipelines.find((Pipeline_Enum)i) == pipelines.end()) {
				printf("pipeline : %d was never used! \n", i);
			}
		}
		pipelines.clear();
		dynamicMaterialPipeline.clear();

		if (materialPipelineCache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(device.device(), materialPipelineCache, nullptr);
		}
		if (boneMaterialPipelineCache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(device.device(), boneMaterialPipelineCache, nullptr);
		}
		if (instanceMaterialPipelineCache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(device.device(), instanceMaterialPipelineCache, nullptr);
		}
		vkDestroyShaderModule(device.device(), loadingVertShaderModule, nullptr);
		vkDestroyShaderModule(device.device(), loadingFragShaderModule, nullptr);
		loadingPipeline.reset();
#if DECONSTRUCTION_DEBUG
		printf("end deconstructing pipeline manager \n");
#endif
	}
	*/
}