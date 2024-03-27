#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

#include "EWEngine/Graphics/Texture/Texture_Manager.h"

#include "EWEngine/Graphics/PushConstants.h"

namespace EWE {

	MaterialPipelines::MaterialPipelines(uint16_t pipeLayoutIndex, std::string const& vertFilepath, std::string const& fragFilepath, EWEPipeline::PipelineConfigInfo const& configInfo) : pipeLayoutIndex{ pipeLayoutIndex }, pipeline{ vertFilepath, fragFilepath, configInfo } {}

	MaterialPipelines::MaterialPipelines(uint16_t pipeLayoutIndex, VkShaderModule vertShaderModu, VkShaderModule fragShaderModu, EWEPipeline::PipelineConfigInfo const& configInfo) : pipeLayoutIndex{ pipeLayoutIndex }, pipeline{ vertShaderModu, fragShaderModu, configInfo } {}

	MaterialPipelines::MaterialPipelines(uint16_t pipeLayoutIndex, std::string const& vertFilePath, MaterialFlags flags, EWEPipeline::PipelineConfigInfo const& configInfo, bool hasBones) : pipeLayoutIndex{ pipeLayoutIndex }, pipeline{ vertFilePath, flags, configInfo, hasBones } {}

	MaterialPipelines::MaterialPipelines(uint16_t pipeLayoutIndex, uint16_t boneCount, MaterialFlags flags, EWEPipeline::PipelineConfigInfo const& configInfo) : pipeLayoutIndex{ pipeLayoutIndex }, pipeline{ boneCount, flags, configInfo } {}




	MaterialPipelines::~MaterialPipelines() {
	}

	void MaterialPipelines::bindPipeline() {
		pipeline.bind(cmdBuf);
		bindedTexture = TEXTURE_UNBINDED_DESC;
		bindedModel = nullptr;
	}
	void MaterialPipelines::bindModel(EWEModel* model) {
		bindedModel = model;
		bindedModel->bind(cmdBuf);
	}
	void MaterialPipelines::bindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet) {
		vkCmdBindDescriptorSets(cmdBuf,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			materialPipeLayout[pipeLayoutIndex].pipeLayout,
			descSlot, 1,
			descSet,
			0, nullptr
		);
	}

	void MaterialPipelines::bindTextureDescriptor(uint8_t descSlot, TextureDesc texID) {
		if (bindedTexture != texID) {
			bindDescriptor(descSlot, &texID);
			bindedTexture = texID;
		}
	}
	void MaterialPipelines::push(void* push) {
		materialPipeLayout[pipeLayoutIndex].push(cmdBuf, push);
	}

	void MaterialPipelines::pushAndDraw(void* push) {
		materialPipeLayout[pipeLayoutIndex].push(cmdBuf, push);

#ifdef _DEBUG
		if (bindedModel == nullptr) {
			printf("failed model draw \n");
			throw std::runtime_error("attempting to draw a model while none is binded");
		}
#endif
		bindedModel->draw(cmdBuf);
	}
	void MaterialPipelines::drawModel() {
#ifdef _DEBUG
		if (bindedModel == nullptr) {
			printf("failed model draw \n");
			throw std::runtime_error("attempting to draw a model while none is binded");
		}
#endif
		bindedModel->draw(cmdBuf);
	}
	void MaterialPipelines::drawInstanced(EWEModel* model) {
		model->BindAndDrawInstance(cmdBuf);
	}



//~~~~~~~~~~~~~~~~~~~ STATIC PORTION ~~~~~~~~~~~~~~~~~~~~~~~~~~~

	uint8_t MaterialPipelines::frameIndex;
	VkCommandBuffer MaterialPipelines::cmdBuf;
#ifdef _DEBUG
	MaterialPipelines* MaterialPipelines::currentPipe;
#endif

	VkPipelineCache MaterialPipelines::materialPipelineCache{ VK_NULL_HANDLE };
	VkPipelineCache MaterialPipelines::skinPipelineCache{ VK_NULL_HANDLE };
	VkPipelineCache MaterialPipelines::instanceSkinPipelineCache{ VK_NULL_HANDLE };
	MaterialPipelines::MaterialPipeLayoutInfo MaterialPipelines::materialPipeLayout[DYNAMIC_PIPE_LAYOUT_COUNT];
	std::unordered_map<MaterialFlags, MaterialPipelines*> MaterialPipelines::materialPipelines;
	std::unordered_map<SkinInstanceKey, MaterialPipelines*> MaterialPipelines::instancedBonePipelines;

	MaterialPipelines* MaterialPipelines::at(MaterialFlags flags) {
#ifdef _DEBUG
		currentPipe = materialPipelines.at(flags);
		return currentPipe;
#endif
		return materialPipelines.at(flags);
	}
	MaterialPipelines* MaterialPipelines::at(SkinInstanceKey skinInstanceKey) {
#ifdef _DEBUG
		currentPipe = instancedBonePipelines.at(skinInstanceKey);
		return currentPipe;
#endif
		return instancedBonePipelines.at(skinInstanceKey);
	}
	MaterialPipelines* MaterialPipelines::at(uint16_t boneCount, MaterialFlags flags) {
		SkinInstanceKey key{ boneCount, flags };
#ifdef _DEBUG
		currentPipe = instancedBonePipelines.at(key);
		return currentPipe;
#endif
		return instancedBonePipelines.at(key);
	}

	void MaterialPipelines::setFrameInfo(FrameInfo const& frameInfo) {
		cmdBuf = frameInfo.cmdBuf;
		frameIndex = frameInfo.index;
	}

	void MaterialPipelines::initMaterialPipeLayout(uint16_t dynamicPipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced, bool hasBump) {
		//layouts
		//textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2))
		if (materialPipeLayout[dynamicPipeLayoutIndex].pipeLayout == VK_NULL_HANDLE) {




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

			std::vector<VkDescriptorSetLayout> tempDSL{ getPipeDSL(textureCount, hasBones, instanced, hasBump) };
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(tempDSL.size());
			pipelineLayoutInfo.pSetLayouts = tempDSL.data();

			printf("creating dynamic pipe layout with index : %d \n", textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2)));

			MaterialPipeLayoutInfo& pipeLayoutInfo = materialPipeLayout[textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2))];
			pipeLayoutInfo.pushSize = pushConstantRange.size;
			pipeLayoutInfo.pushStageFlags = pushConstantRange.stageFlags;

			EWE_VK_ASSERT(vkCreatePipelineLayout(EWEDevice::GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipeLayoutInfo.pipeLayout));
		}
	}

	MaterialPipelines* MaterialPipelines::getMaterialPipe(MaterialFlags flags) {
		{
			auto foundPipe = materialPipelines.find(flags);
			if (foundPipe != materialPipelines.end()) {
				return foundPipe->second;
			}
		}


		bool hasBones = flags & MaterialF_hasBones;
		bool instanced = flags & MaterialF_instanced; //curently creating an outside manager to deal with instanced skinned meshes
#ifdef _DEBUG
		if (instanced) {
			printf("creating a material pipe with bones or instanced flag set, no longer supported \n");
			throw std::exception("creating a material pipe with instanced flag set, which needs to be created using constructInstancedMaterial");
		}
#endif

		//bool finalSlotBeforeNeedExpansion = MaterialFlags & 32;
		bool hasBumps = flags & MaterialF_hasBump;
		bool hasNormal = flags & MaterialF_hasNormal;
		bool hasRough = flags & MaterialF_hasRough;
		bool hasMetal = flags & MaterialF_hasMetal;
		bool hasAO = flags & MaterialF_hasAO;

		uint8_t textureCount = hasNormal + hasRough + hasMetal + hasAO + hasBumps;
		uint16_t pipeLayoutIndex = textureCount + (MAX_MATERIAL_TEXTURE_COUNT * hasBones);
		printf("textureCount, hasBones, instanced - %d:%d:%d \n", textureCount, hasBones, instanced);

#ifdef _DEBUG
		if (textureCount == 0) {
			//undesirable, but not quite a bug. only passing in an albedo texture is valid
			printf("material pipeline, flags textureCount is 0 \n");
		}
#endif

		initMaterialPipeLayout(pipeLayoutIndex, textureCount, hasBones, instanced, hasBumps);

		//printf("creating pipeline, dynamicShaderFinding, (key value:%d)-(bones:%d)-(normal:%d)-(rough:%d)-(metal:%d)-(ao:%d) \n", newFlags, hasBones, hasNormal, hasRough, hasMetal, hasAO );
		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.pipelineLayout = materialPipeLayout[pipeLayoutIndex].pipeLayout;

		getPipeCache(hasBones, instanced, pipelineConfig.cache);
	
		return createPipe(pipeLayoutIndex, pipelineConfig, hasBones, hasNormal, hasBumps, flags);

		//printf("after dynamic shader finding \n");
	}

	void MaterialPipelines::initStaticVariables() {
		for (int i = 0; i < DYNAMIC_PIPE_LAYOUT_COUNT; i++) {
			materialPipeLayout[i].pipeLayout = VK_NULL_HANDLE;
		}
	}

	void MaterialPipelines::cleanupStaticVariables() {

#if DECONSTRUCTION_DEBUG
		printf("begin deconstructing material pipelines \n");
#endif
		for(auto& pipe : materialPipelines) {
			pipe.second->~MaterialPipelines();
			ewe_free(pipe.second);
		}
		materialPipelines.clear();
		for (auto& pipe : instancedBonePipelines) {
			pipe.second->~MaterialPipelines();
			ewe_free(pipe.second);
		}
		instancedBonePipelines.clear();

		for (auto& plInfo : materialPipeLayout) {
			if (plInfo.pipeLayout != VK_NULL_HANDLE) {
				vkDestroyPipelineLayout(EWEDevice::GetVkDevice(), plInfo.pipeLayout, nullptr);
			}
		}

		if (materialPipelineCache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(EWEDevice::GetVkDevice(), materialPipelineCache, nullptr);
		}
		if (skinPipelineCache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(EWEDevice::GetVkDevice(), skinPipelineCache, nullptr);
		}
		if (instanceSkinPipelineCache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(EWEDevice::GetVkDevice(), instanceSkinPipelineCache, nullptr);
		}

#if DECONSTRUCTION_DEBUG
		printf("end deconstructing pipeline manager \n");
#endif
	}

	std::vector<VkDescriptorSetLayout> MaterialPipelines::getPipeDSL(uint8_t textureCount, bool hasBones, bool instanced, bool hasBump) {
		//printf("get dynamic pipe desc set layout : %d \n", textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2)));

		std::vector<VkDescriptorSetLayout> returnLayouts{};

		returnLayouts.push_back(DescriptorHandler::getDescSetLayout(LDSL_global));
#ifdef _DEBUG
		printf("getting dynamic PDSL - %d:%d:%d \n", textureCount, hasBones, instanced);
#endif
		if (hasBones && instanced) {
			returnLayouts.push_back(DescriptorHandler::getDescSetLayout(LDSL_largeInstance));

		}
		else if (hasBones) {
			returnLayouts.push_back(DescriptorHandler::getDescSetLayout(LDSL_boned));
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
		returnLayouts.push_back(dslInfo.getDescSetLayout()->getDescriptorSetLayout());
		//if (instanced) {
			//printf("returning instanced PDSL size : %d \n", dynamicMaterialPipeDescSetLayouts[textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2))].size());
		//}
		return returnLayouts;
	}

	MaterialPipelines* MaterialPipelines::getInstancedSkinMaterialPipe(uint16_t boneCount, MaterialFlags flags) {

		SkinInstanceKey skinInstanceKey{ boneCount, flags };
		if(instancedBonePipelines.contains(skinInstanceKey)){
			//printf("creating instanced skin pipeline \n");
			return instancedBonePipelines.at(skinInstanceKey);
		}


		//bool finalSlotBeforeNeedExpansion = MaterialFlags & 32;
		bool hasBumps = flags & MaterialF_hasBump;
		bool hasNormal = flags & MaterialF_hasNormal;
		bool hasRough = flags & MaterialF_hasRough;
		bool hasMetal = flags & MaterialF_hasMetal;
		bool hasAO = flags & MaterialF_hasAO;

		uint8_t textureCount = hasNormal + hasRough + hasMetal + hasAO + hasBumps;

		if (hasBumps) {
			printf("HAS BONES AND BUMP, SHOULD NOT HAPPEN \n");
		}

		uint16_t pipeLayoutIndex = textureCount + (MAX_MATERIAL_TEXTURE_COUNT * 3);
		initMaterialPipeLayout(pipeLayoutIndex, textureCount, true, true, hasBumps);


		EWEPipeline::PipelineConfigInfo pipelineConfig{};
		EWEPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.pipelineLayout = materialPipeLayout[pipeLayoutIndex].pipeLayout;


		printf("initiating remote instanced pipeline : %d \n", flags);

		if (instanceSkinPipelineCache == VK_NULL_HANDLE) {
			VkPipelineCacheCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			if (vkCreatePipelineCache(EWEDevice::GetVkDevice(), &createInfo, nullptr, &instanceSkinPipelineCache) != VK_SUCCESS) {
				// handle error
				printf("failed to create instance skinned material pipeline cache \n");
				throw std::runtime_error("failed to create instanced skin material pipeline cache");
			}
		}
		pipelineConfig.cache = instanceSkinPipelineCache;

		//printf("boneVertex, flags:%d \n", newFlags);
		pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<boneVertex>();
		pipelineConfig.attributeDescriptions = boneVertex::getAttributeDescriptions();
		glslang::InitializeProcess();
		
		SkinInstanceKey key(boneCount, flags);

		auto ret = instancedBonePipelines.try_emplace(key, ConstructSingular<MaterialPipelines>(ewe_call_trace, pipeLayoutIndex, boneCount, flags, pipelineConfig)).first->second;

		glslang::FinalizeProcess();
		return ret;
	}

	void MaterialPipelines::getPipeCache(bool hasBones, bool instanced, VkPipelineCache& outCache) {

		VkPipelineCache retCache;
		if (instanced) {
			if (hasBones) {
				retCache = instanceSkinPipelineCache;
			}
			else {
				printf("instancing without skin not yet implemented \n");
				throw std::runtime_error("invalid material pipe cache");
				//retCache = instanceMaterialPipelineCache;
			}
		}
		else if (hasBones) {
			retCache = skinPipelineCache;
		}
		else {
			retCache = materialPipelineCache;
		}

		if (retCache != VK_NULL_HANDLE) {
			outCache = retCache;
#if PIPELINE_DERIVATIVES
			pipelineConfig.basePipelineHandle = dynamicMaterialPipeline[daddyPipeKey].get();
			pipelineConfig.basePipelineIndex = -1;
			pipelineConfig.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
#endif
			return;
		}
		VkPipelineCacheCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		if (vkCreatePipelineCache(EWEDevice::GetVkDevice(), &createInfo, nullptr, &retCache) != VK_SUCCESS) {
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

		outCache = retCache;
		return;
	}

	MaterialPipelines* MaterialPipelines::createPipe(uint16_t pipeLayoutIndex, EWEPipeline::PipelineConfigInfo& pipelineConfig, bool hasBones, bool hasNormal, bool hasBumps, MaterialFlags flags) {
		if (hasBones) {
			if (hasNormal) {
				//printf("boneVertex, flags:%d \n", newFlags);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<boneVertex>();
				pipelineConfig.attributeDescriptions = boneVertex::getAttributeDescriptions();
				return materialPipelines.try_emplace(flags, ConstructSingular<MaterialPipelines>(ewe_call_trace, pipeLayoutIndex, "bone_Tangent.vert.spv", flags, pipelineConfig, true)).first->second;

			}
			else {
				//printf("boneVertexNT, flags:%d \n", newFlags);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<boneVertexNoTangent>();
				pipelineConfig.attributeDescriptions = boneVertexNoTangent::getAttributeDescriptions();
				return materialPipelines.try_emplace(flags, ConstructSingular<MaterialPipelines>(ewe_call_trace, pipeLayoutIndex, "bone_NT.vert.spv", flags, pipelineConfig, true)).first->second;
			}
		}
		else {
			if (hasBumps) {
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<Vertex>();
				pipelineConfig.attributeDescriptions = Vertex::getAttributeDescriptions();
				return materialPipelines.try_emplace(flags, ConstructSingular<MaterialPipelines>(ewe_call_trace, pipeLayoutIndex, "material_bump.vert.spv", flags, pipelineConfig, false)).first->second;
			}
			else if (hasNormal) {
				//printf("AVertex, flags:%d \n", newFlags);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<Vertex>();
				pipelineConfig.attributeDescriptions = Vertex::getAttributeDescriptions();
				return materialPipelines.try_emplace(flags, ConstructSingular<MaterialPipelines>(ewe_call_trace, pipeLayoutIndex, "material_Tangent.vert.spv", flags, pipelineConfig, false)).first->second;
			}
			else {
				//printf("AVertexNT, flags:%d \n", newFlags);
				pipelineConfig.bindingDescriptions = EWEModel::getBindingDescriptions<VertexNT>();
				pipelineConfig.attributeDescriptions = VertexNT::getAttributeDescriptions();
				return materialPipelines.try_emplace(flags, ConstructSingular<MaterialPipelines>(ewe_call_trace, pipeLayoutIndex, "material_nn.vert.spv", flags, pipelineConfig, false)).first->second;
			}
		}
	}
}