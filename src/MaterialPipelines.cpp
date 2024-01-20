#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

#include "EWEngine/Graphics/Textures/Texture_Manager.h"

namespace EWE {

	VkPipelineCache MaterialPipelines::materialPipelineCache{ VK_NULL_HANDLE };
	VkPipelineCache MaterialPipelines::boneMaterialPipelineCache{ VK_NULL_HANDLE };
	VkPipelineCache MaterialPipelines::instanceMaterialPipelineCache{ VK_NULL_HANDLE };
	VkPipelineLayout MaterialPipelines::materialPipeLayout[DYNAMIC_PIPE_LAYOUT_COUNT];
	std::unordered_map<uint8_t, std::unique_ptr<EWEPipeline>> MaterialPipelines::materialPipelines;


	void MaterialPipelines::initMaterialPipeLayout(uint16_t dynamicPipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced, EWEDevice& device, bool hasBump) {
		//layouts
		//textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2))
		if (materialPipeLayout[dynamicPipeLayoutIndex] == VK_NULL_HANDLE) {


			std::vector<VkDescriptorSetLayout> tempDSL{ getPipeDSL(textureCount, hasBones, instanced, device, hasBump)};


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

			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL.size());
			pipelineLayoutInfo.pSetLayouts = tempDSL.data();

			printf("creating dynamic pipe layout with index : %d \n", textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2)));

			VkResult vkResult = vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &materialPipeLayout[textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2))]);
			if (vkResult != VK_SUCCESS) {
				printf("failed to create dynamic mat pipelayout layout [%d] \n", textureCount);
				throw std::runtime_error("Failed to create dynamic mat pipe layout \n");
			}
		}
	}

	void MaterialPipelines::constructMaterialPipe(MaterialFlags flags, VkPipelineRenderingCreateInfo const& pipeRenderInfo, EWEDevice& device) {
		bool hasBones = flags & 128;
		bool instanced = flags & 64; //curently creating an outside manager to deal with instanced skinned meshes
#ifdef _DEBUG
		if (instanced) {
			printf("creating a material pipe with bones or instanced flag set, no longer supported \n");
			throw std::exception("creating a material pipe with instanced flag set, which needs to be created using constructInstancedMaterial");
		}
#endif

		//bool finalSlotBeforeNeedExpansion = MaterialFlags & 32;
		bool hasBumps = flags & 16;
		bool hasNormal = flags & 8;
		bool hasRough = flags & 4;
		bool hasMetal = flags & 2;
		bool hasAO = flags & 1;

		uint8_t textureCount = hasNormal + hasRough + hasMetal + hasAO + hasBumps;
		uint16_t pipeLayoutIndex = textureCount + (MAX_MATERIAL_TEXTURE_COUNT * hasBones);
		printf("textureCount, hasBones, instanced - %d:%d:%d \n", textureCount, hasBones, instanced);

#ifdef _DEBUG
		if (textureCount == 0) {
			//undesirable, but not quite a bug. only passing in an albedo texture is valid
			printf("material pipeline, flags textureCount is 0 wtf \n");
		}
#endif

		
		initMaterialPipeLayout(pipeLayoutIndex, textureCount, false, false, device, hasBumps);



		//printf("creating new pipeline, dynamicShaderFinding, (key value:%d)-(bones:%d)-(normal:%d)-(rough:%d)-(metal:%d)-(ao:%d) \n", newFlags, hasBones, hasNormal, hasRough, hasMetal, hasAO );
		if (!materialPipelines.contains(flags)) {
			EWEPipeline::PipelineConfigInfo pipelineConfig{};
			EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
			pipelineConfig.pipelineRenderingInfo = pipeRenderInfo;
			pipelineConfig.pipelineLayout = materialPipeLayout[pipeLayoutIndex];

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

				materialPipelines.emplace(flags, std::make_unique<EWEPipeline>(device, "material_bump.vert.spv", flags, pipelineConfig, false));
			}
			else if (hasNormal) {
				//printf("AVertex, flags:%d \n", newFlags);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<AVertex>();
				pipelineConfig.attributeDescriptions = AVertex::getAttributeDescriptions();
				materialPipelines.emplace(flags, std::make_unique<EWEPipeline>(device, "material_Tangent.vert.spv", flags, pipelineConfig, false));
			}
			else {
				//printf("AVertexNT, flags:%d \n", newFlags);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<AVertexNT>();
				pipelineConfig.attributeDescriptions = AVertexNT::getAttributeDescriptions();
				materialPipelines.emplace(flags, std::make_unique<EWEPipeline>(device, "material_nn.vert.spv", flags, pipelineConfig, false));
			}


			//printf("after dynamic shader finding \n");
		}
	}

	void MaterialPipelines::initStaticVariables() {
		for (int i = 0; i < DYNAMIC_PIPE_LAYOUT_COUNT; i++) {
			materialPipeLayout[i] = VK_NULL_HANDLE;
		}
	}

	void MaterialPipelines::cleanupStaticVariables(EWEDevice& device) {

#if DECONSTRUCTION_DEBUG
		printf("begin deconstructing pipeline manager \n");
#endif
		for (int i = 0; i < DYNAMIC_PIPE_LAYOUT_COUNT; i++) {
			if (materialPipeLayout[i] != VK_NULL_HANDLE) {
				vkDestroyPipelineLayout(device.device(), materialPipeLayout[i], nullptr);
			}
		}
		materialPipelines.clear();

		if (materialPipelineCache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(device.device(), materialPipelineCache, nullptr);
		}
		if (boneMaterialPipelineCache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(device.device(), boneMaterialPipelineCache, nullptr);
		}
		if (instanceMaterialPipelineCache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(device.device(), instanceMaterialPipelineCache, nullptr);
		}

#if DECONSTRUCTION_DEBUG
		printf("end deconstructing pipeline manager \n");
#endif
	}

	std::vector<VkDescriptorSetLayout> MaterialPipelines::getPipeDSL(uint8_t textureCount, bool hasBones, bool instanced, EWEDevice& device, bool hasBump) {
		//printf("get dynamic pipe desc set layout : %d \n", textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2)));

		std::vector<VkDescriptorSetLayout> returnLayouts;

		returnLayouts.push_back(DescriptorHandler::getDescSetLayout(LDSL_global, device));
#ifdef _DEBUG
		printf("getting dynamic PDSL - %d:%d:%d \n", textureCount, hasBones, instanced);
#endif
		if (hasBones && instanced) {
			returnLayouts.push_back(DescriptorHandler::getDescSetLayout(LDSL_largeInstance, device));

		}
		else if (hasBones) {
			returnLayouts.push_back(DescriptorHandler::getDescSetLayout(LDSL_boned, device));
		}
		else if (instanced) {
			printf("currrently not supporting instancing without bones, THROWING ERROR \n");
			throw std::runtime_error("instanced but doesn't have bones?");
		}
		TextureDSLInfo dslInfo{};
		if (hasBump) {
			dslInfo.setStageTextureCount(VK_SHADER_STAGE_VERTEX_BIT, 1);
		}
		dslInfo.setStageTextureCount(VK_SHADER_STAGE_FRAGMENT_BIT, textureCount - hasBump + 1); //+1 for the albedo, which isnt incldued in textureCount
		returnLayouts.push_back(dslInfo.getDescSetLayout(device)->getDescriptorSetLayout());
		//if (instanced) {
			//printf("returning instanced PDSL size : %d \n", dynamicMaterialPipeDescSetLayouts[textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2))].size());
		//}
		return returnLayouts;
	}

	void MaterialPipelines::constructInstancedMaterial(MaterialFlags flags, uint16_t boneCount, VkPipelineRenderingCreateInfo const& pipeRenderInfo, EWEDevice& device) {

#ifdef _DEBUG
		for (int i = 0; i < instancedBonePipeTracker.size(); i++) {
			if (flags == instancedBonePipeTracker[i].second) {
				if (instancedBonePipeTracker[i].first == boneCount) {
					printf("double created instanced pipeline, throwing error \n");
					throw std::exception("double created instanced pipeline");
				}
			}
		}
		instancedBonePipeTracker.emplace_back(boneCount, flags);
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
		initMaterialPipeLayout(pipeLayoutIndex, textureCount, true, true, device, hasBumps);


		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.pipelineRenderingInfo = pipeRenderInfo;
		pipelineConfig.pipelineLayout = materialPipeLayout[pipeLayoutIndex];


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
}