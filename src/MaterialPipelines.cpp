#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

#include "EWEngine/Graphics/Texture/Image_Manager.h"

#include "EWEngine/Graphics/PushConstants.h"

namespace EWE {

	struct MaterialPipeLayoutInfo {
		VkPipelineLayout pipeLayout{ VK_NULL_HANDLE };
		uint32_t pushSize;
		VkShaderStageFlags pushStageFlags;

		void Push(void* pushData) {
			EWE_VK(vkCmdPushConstants, VK::Object->GetFrameBuffer(), pipeLayout, pushStageFlags, 0, pushSize, pushData);
		}
	};

	VkPipelineCache materialPipelineCache{ VK_NULL_HANDLE };
	VkPipelineCache instanceMaterialPipelineCache{ VK_NULL_HANDLE };
	VkPipelineCache skinPipelineCache{ VK_NULL_HANDLE };
	VkPipelineCache instanceSkinPipelineCache{ VK_NULL_HANDLE };
	std::array<MaterialPipeLayoutInfo, MATERIAL_PIPE_LAYOUT_COUNT> materialPipeLayout;
	std::array<EWEDescriptorSetLayout*, MATERIAL_PIPE_LAYOUT_COUNT> eDSLs;

	EWEDescriptorSetLayout* MaterialPipelines::GetDSL(uint16_t pipeLayoutIndex) {
#if EWE_DEBUG
		assert(eDSLs[pipeLayoutIndex] != VK_NULL_HANDLE);
#endif
		return eDSLs[pipeLayoutIndex];
	}
	EWEDescriptorSetLayout* MaterialPipelines::GetDSLFromFlags(MaterialFlags flags) {
		return GetDSL(MaterialPipelines::GetPipeLayoutIndex(flags));
	}

	void InitPipeDSL(uint16_t pipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced, bool hasBump) {
		//printf("get dynamic pipe desc set layout : %d \n", textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2)));

		//might be relevant to only build these once, not sure if its a big deal
		if (eDSLs[pipeLayoutIndex] != VK_NULL_HANDLE) {
			return;
		}

		EWEDescriptorSetLayout::Builder builder{};
		builder.AddGlobalBindings();
#if EWE_DEBUG
		printf("getting material PDSL - %d:%d:%d \n", textureCount, hasBones, instanced);
#endif
		if (hasBones && instanced) {
			builder.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
			builder.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		}
		else if (hasBones) {
			builder.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		}
		else if (instanced) {
			builder.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		}
		if (hasBump) {
			builder.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		else {
			builder.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		}

		eDSLs[pipeLayoutIndex] = builder.Build();
#if DEBUG_NAMING
		std::string materialDSLName = "material DSL[" + std::to_string(pipeLayoutIndex) + ']';
		printf("materialDSLName : %s\n", materialDSLName.c_str());
		DebugNaming::SetObjectName(*eDSLs[pipeLayoutIndex]->GetDescriptorSetLayout(), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, materialDSLName.c_str());
#endif
	}

	MaterialPipelines::MaterialPipelines(uint16_t pipeLayoutIndex, std::string const& vertFilepath, std::string const& fragFilepath, EWEPipeline::PipelineConfigInfo const& configInfo) : pipeLayoutIndex{ pipeLayoutIndex }, pipeline{ vertFilepath, fragFilepath, configInfo } {}

	MaterialPipelines::MaterialPipelines(uint16_t pipeLayoutIndex, VkShaderModule vertShaderModu, VkShaderModule fragShaderModu, EWEPipeline::PipelineConfigInfo const& configInfo) : pipeLayoutIndex{ pipeLayoutIndex }, pipeline{ vertShaderModu, fragShaderModu, configInfo } {}

	MaterialPipelines::MaterialPipelines(uint16_t pipeLayoutIndex, std::string const& vertFilePath, MaterialFlags flags, EWEPipeline::PipelineConfigInfo const& configInfo, bool hasBones) : pipeLayoutIndex{ pipeLayoutIndex }, pipeline{ vertFilePath, flags, configInfo, hasBones } {}

	MaterialPipelines::MaterialPipelines(uint16_t pipeLayoutIndex, uint16_t boneCount, MaterialFlags flags, EWEPipeline::PipelineConfigInfo const& configInfo) : pipeLayoutIndex{ pipeLayoutIndex }, pipeline{ boneCount, flags, configInfo } {}

	void GetPipeCache(bool hasBones, bool instanced, VkPipelineCache& outCache) {

		VkPipelineCache retCache;
		if (instanced) {
			if (hasBones) {
				retCache = instanceSkinPipelineCache;
			}
			else {
				//assert(hasBones);
				retCache = instanceMaterialPipelineCache;
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
		createInfo.pNext = nullptr;
		createInfo.initialDataSize = 0;
		createInfo.flags = 0;
		createInfo.pInitialData = nullptr;

		EWE_VK(vkCreatePipelineCache, VK::Object->vkDevice, &createInfo, nullptr, &retCache);
#if PIPELINE_DERIVATIVES
		pipelineConfig.basePipelineHandle = nullptr;
		pipelineConfig.basePipelineIndex = -1;
		pipelineConfig.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
		daddyPipeKey = newFlags;
#endif

		outCache = retCache;
		return;
	}


	void InitMaterialPipelineConfig(EWEPipeline::PipelineConfigInfo& pipelineConfig, MaterialFlags flags) {
		bool hasBones = flags & MaterialF_hasBones;
		bool instanced = flags & MaterialF_instanced;
#if EWE_DEBUG
		if (instanced) {
			//probably going to need a breakpoint here for debugging
		}
#endif
		//bool finalSlotBeforeNeedExpansion = MaterialFlags & 32;
		bool hasBumps = flags & MaterialF_hasBump;
		bool hasNormal = flags & MaterialF_hasNormal;
		bool hasRough = flags & MaterialF_hasRough;
		bool hasMetal = flags & MaterialF_hasMetal;
		bool hasAO = flags & MaterialF_hasAO;
		if (hasBones && hasBumps) {
			printf("ERROR: HAS BONES AND BUMP, NOT CURRENTLY SUPPORTED \n");
			assert(false);
		}

		uint8_t textureCount = hasNormal + hasRough + hasMetal + hasAO + hasBumps;
		uint16_t pipeLayoutIndex = textureCount + (MAX_MATERIAL_TEXTURE_COUNT * (hasBones + 2 * instanced));

#if EWE_DEBUG
		printf("textureCount, hasBones, instanced - %d:%d:%d \n", textureCount, hasBones, instanced);
		if (textureCount == 0) {
			//undesirable, but not quite a bug. only passing in an albedo texture is valid
			printf("material pipeline, flags textureCount is 0 \n");
		}
#endif

		MaterialPipelines::InitMaterialPipeLayout(pipeLayoutIndex, textureCount, hasBones, instanced, hasBumps);

		//printf("creating pipeline, dynamicShaderFinding, (key value:%d)-(bones:%d)-(normal:%d)-(rough:%d)-(metal:%d)-(ao:%d) \n", newFlags, hasBones, hasNormal, hasRough, hasMetal, hasAO );
		EWEPipeline::DefaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.pipelineLayout = materialPipeLayout[pipeLayoutIndex].pipeLayout;

		GetPipeCache(hasBones, instanced, pipelineConfig.cache);
	}

	MaterialPipelines::~MaterialPipelines() {
	}

	void MaterialPipelines::BindPipeline() {
		pipeline.Bind();
		bindedModel = nullptr;
	}
	void MaterialPipelines::BindModel(EWEModel* model) {
		bindedModel = model;
		bindedModel->Bind();
	}
	void MaterialPipelines::BindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet) {
		EWE_VK(vkCmdBindDescriptorSets, VK::Object->GetFrameBuffer(),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			materialPipeLayout[pipeLayoutIndex].pipeLayout,
			descSlot, 1,
			descSet,
			0, nullptr
		);
	}
	void MaterialPipelines::BindDescriptor(uint8_t descSlot, const VkDescriptorSet* descSet) {
		EWE_VK(vkCmdBindDescriptorSets, VK::Object->GetFrameBuffer(),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			materialPipeLayout[pipeLayoutIndex].pipeLayout,
			descSlot, 1,
			descSet,
			0, nullptr
		);
	}
	void MaterialPipelines::Push(void* push) {
		materialPipeLayout[pipeLayoutIndex].Push(push);
	}

	void MaterialPipelines::PushAndDraw(void* push) {
		materialPipeLayout[pipeLayoutIndex].Push(push);

#if EWE_DEBUG
		assert(bindedModel != nullptr && "failed model draw");
#endif
		bindedModel->Draw();
	}
	void MaterialPipelines::DrawModel() {
#if EWE_DEBUG
		assert(bindedModel != nullptr && "failed model draw");
#endif
		bindedModel->Draw();
	}
	void MaterialPipelines::DrawInstanced(EWEModel* model) {
		model->BindAndDrawInstance();
	}
	void MaterialPipelines::DrawInstanced(EWEModel* model, uint32_t instanceCount) {
		model->BindAndDrawInstance(instanceCount);
	}



//~~~~~~~~~~~~~~~~~~~ STATIC PORTION ~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if EWE_DEBUG
	MaterialPipelines* MaterialPipelines::currentPipe;
#endif

	std::unordered_map<MaterialFlags, MaterialPipelines*> MaterialPipelines::materialPipelines;
	std::unordered_map<SkinInstanceKey, MaterialPipelines*> MaterialPipelines::instancedBonePipelines;

	MaterialPipelines* MaterialPipelines::At(MaterialFlags flags) {
#if EWE_DEBUG
		currentPipe = materialPipelines.at(flags);
		return currentPipe;
#endif
		return materialPipelines.at(flags);
	}
	MaterialPipelines* MaterialPipelines::At(SkinInstanceKey skinInstanceKey) {
#if EWE_DEBUG
		currentPipe = instancedBonePipelines.at(skinInstanceKey);
		return currentPipe;
#endif
		return instancedBonePipelines.at(skinInstanceKey);
	}
	MaterialPipelines* MaterialPipelines::At(uint16_t boneCount, MaterialFlags flags) {
		SkinInstanceKey key{ boneCount, flags };
#if EWE_DEBUG
		currentPipe = instancedBonePipelines.at(key);
		return currentPipe;
#endif
		return instancedBonePipelines.at(key);
	}

	void MaterialPipelines::InitMaterialPipeLayout(uint16_t pipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced, bool hasBump) {
		//layouts
		//textureCount + (hasBones * MAX_MATERIAL_TEXTURE_COUNT) + (instanced * (MAX_MATERIAL_TEXTURE_COUNT * 2))
		if (materialPipeLayout[pipeLayoutIndex].pipeLayout == VK_NULL_HANDLE) {

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.pNext = nullptr;
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
					pushConstantRange.size = sizeof(ModelAndNormalPushData);
				}
				pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
				pipelineLayoutInfo.pushConstantRangeCount = 1;
			}

			InitPipeDSL(pipeLayoutIndex, textureCount, hasBones, instanced, hasBump);
			pipelineLayoutInfo.setLayoutCount = 1;
			pipelineLayoutInfo.pSetLayouts = eDSLs[pipeLayoutIndex]->GetDescriptorSetLayout();

#if EWE_DEBUG
			printf("creating material pipe layout with index : %d \n", pipeLayoutIndex);
#endif

			MaterialPipeLayoutInfo& pipeLayoutInfo = materialPipeLayout[pipeLayoutIndex];
			pipeLayoutInfo.pushSize = pushConstantRange.size;
			pipeLayoutInfo.pushStageFlags = pushConstantRange.stageFlags;

			EWE_VK(vkCreatePipelineLayout, VK::Object->vkDevice, &pipelineLayoutInfo, nullptr, &pipeLayoutInfo.pipeLayout);
		}
	}

	MaterialPipelines* MaterialPipelines::GetMaterialPipe(MaterialFlags flags) {
		{
			const auto foundPipe = materialPipelines.find(flags);
			if (foundPipe != materialPipelines.end()) {
				return foundPipe->second;
			}
		}
		EWEPipeline::PipelineConfigInfo pipelineConfig;
		InitMaterialPipelineConfig(pipelineConfig, flags);

		MaterialPipelines* ret = CreatePipe(pipelineConfig, flags);
#if DEBUG_NAMING
		std::string pipeName = "material pipeline[";
		pipeName += std::to_string(flags) + ']';
		ret->pipeline.SetDebugName(pipeName);
#endif
		return ret;

		//printf("after dynamic shader finding \n");
	}

	MaterialPipelines* MaterialPipelines::GetMaterialPipe(MaterialFlags flags, uint16_t boneCount) {

		if (flags & MaterialF_hasBones) {

			SkinInstanceKey skinInstanceKey{ boneCount, flags };
			auto findRet = instancedBonePipelines.find(skinInstanceKey);
			if (findRet != instancedBonePipelines.end()) {
				return findRet->second;
			}
		}
		else {
			const auto foundPipe = materialPipelines.find(flags);
			if (foundPipe != materialPipelines.end()) {
				return foundPipe->second;
			}
		}
		EWEPipeline::PipelineConfigInfo pipelineConfig;
		InitMaterialPipelineConfig(pipelineConfig, flags);

		MaterialPipelines* ret;
		if (flags & MaterialF_hasBones) {

			//printf("boneVertex, flags:%d \n", newFlags);
			pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<boneVertex>();
			pipelineConfig.attributeDescriptions = boneVertex::GetAttributeDescriptions();

			const SkinInstanceKey key(boneCount, flags);

			uint16_t pipeLayoutIndex = GetPipeLayoutIndex(flags);

			ret = instancedBonePipelines.try_emplace(key, Construct<MaterialPipelines>({ pipeLayoutIndex, boneCount, flags, pipelineConfig })).first->second;

#if DEBUG_NAMING
			std::string pipeName = "material pipeline[";
			pipeName += std::to_string(flags) + ']';
			ret->pipeline.SetDebugName(pipeName);
#endif
			return ret;
		}
		else {
			const uint16_t entityCount = boneCount;
			assert(entityCount <= 1024 && "currently dont have the systems set up for a storage buffer in rigid instancing");

			MaterialPipelines* ret = CreatePipe(pipelineConfig, flags);

#if DEBUG_NAMING
			std::string pipeName = "material pipeline[";
			pipeName += std::to_string(flags) + ']';
			ret->pipeline.SetDebugName(pipeName);
#endif
			return ret;
		}
	}

	void MaterialPipelines::InitStaticVariables() {
		for (int i = 0; i < MATERIAL_PIPE_LAYOUT_COUNT; i++) {
			materialPipeLayout[i].pipeLayout = VK_NULL_HANDLE;
			eDSLs[i] = VK_NULL_HANDLE;
		}
	}

	void MaterialPipelines::CleanupStaticVariables() {

#if DECONSTRUCTION_DEBUG
		printf("begin deconstructing material pipelines \n");
#endif
		for(auto& pipe : materialPipelines) {

			Deconstruct(pipe.second);
		}
		materialPipelines.clear();
		for (auto& pipe : instancedBonePipelines) {
			Deconstruct(pipe.second);
		}
		instancedBonePipelines.clear();

		for (uint8_t i = 0; i < MATERIAL_PIPE_LAYOUT_COUNT; i++) {
			if (materialPipeLayout[i].pipeLayout != VK_NULL_HANDLE) {
				EWE_VK(vkDestroyPipelineLayout, VK::Object->vkDevice, materialPipeLayout[i].pipeLayout, nullptr);
			}
			if (eDSLs[i] != VK_NULL_HANDLE) {
				Deconstruct(eDSLs[i]);
			}
		}

		if (materialPipelineCache != VK_NULL_HANDLE) {
			EWE_VK(vkDestroyPipelineCache, VK::Object->vkDevice, materialPipelineCache, nullptr);
		}
		if (skinPipelineCache != VK_NULL_HANDLE) {
			EWE_VK(vkDestroyPipelineCache, VK::Object->vkDevice, skinPipelineCache, nullptr);
		}
		if (instanceSkinPipelineCache != VK_NULL_HANDLE) {
			EWE_VK(vkDestroyPipelineCache, VK::Object->vkDevice, instanceSkinPipelineCache, nullptr);
		}
		if (instanceMaterialPipelineCache != VK_NULL_HANDLE) {
			EWE_VK(vkDestroyPipelineCache, VK::Object->vkDevice, instanceMaterialPipelineCache, nullptr);
		}

#if DECONSTRUCTION_DEBUG
		printf("end deconstructing pipeline manager \n");
#endif
	}


	MaterialPipelines* MaterialPipelines::CreatePipe(EWEPipeline::PipelineConfigInfo& pipelineConfig, MaterialFlags flags) {

		uint16_t pipeLayoutIndex = GetPipeLayoutIndex(flags);

		const bool hasBones = flags & MaterialF_hasBones;
		const bool hasNormal = flags & MaterialF_hasNormal;
		const bool hasBumps = flags & MaterialF_hasBump;
		const bool instanced = flags & MaterialF_instanced;

		if (hasBones) {
			if (hasNormal) {
				//printf("boneVertex, flags:%d \n", newFlags);
				pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<boneVertex>();
				pipelineConfig.attributeDescriptions = boneVertex::GetAttributeDescriptions();
				return materialPipelines.try_emplace(flags, Construct<MaterialPipelines>({ pipeLayoutIndex, "bone_Tangent.vert.spv", flags, pipelineConfig, true })).first->second;

			}
			else {
				//printf("boneVertexNT, flags:%d \n", newFlags);
				pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<boneVertexNoTangent>();
				pipelineConfig.attributeDescriptions = boneVertexNoTangent::GetAttributeDescriptions();
				return materialPipelines.try_emplace(flags, Construct<MaterialPipelines>({ pipeLayoutIndex, "bone_NT.vert.spv", flags, pipelineConfig, true })).first->second;
			}
		}
		else {
			if (instanced) {

				if (hasBumps) {
					assert(false && "not supported yet");
				}
				else if (hasNormal) {
					pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<Vertex>();
					pipelineConfig.attributeDescriptions = Vertex::GetAttributeDescriptions();
					return materialPipelines.try_emplace(flags, Construct<MaterialPipelines>({ pipeLayoutIndex, "material_tangent_instance.vert.spv", flags, pipelineConfig, false })).first->second;
				}
				else {
					pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexNT>();
					pipelineConfig.attributeDescriptions = VertexNT::GetAttributeDescriptions();
					return materialPipelines.try_emplace(flags, Construct<MaterialPipelines>({ pipeLayoutIndex, "material_nn_instance.vert.spv", flags, pipelineConfig, false })).first->second;
				}
			}
			else {
				if (hasBumps) {
					pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<Vertex>();
					pipelineConfig.attributeDescriptions = Vertex::GetAttributeDescriptions();
					return materialPipelines.try_emplace(flags, Construct<MaterialPipelines>({ pipeLayoutIndex, "material_bump.vert.spv", flags, pipelineConfig, false })).first->second;
				}
				else if (hasNormal) {
					//printf("AVertex, flags:%d \n", newFlags);
					pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<Vertex>();
					pipelineConfig.attributeDescriptions = Vertex::GetAttributeDescriptions();
					return materialPipelines.try_emplace(flags, Construct<MaterialPipelines>({ pipeLayoutIndex, "material_Tangent.vert.spv", flags, pipelineConfig, false })).first->second;
				}
				else {
					//printf("AVertexNT, flags:%d \n", newFlags);
					pipelineConfig.bindingDescriptions = EWEModel::GetBindingDescriptions<VertexNT>();
					pipelineConfig.attributeDescriptions = VertexNT::GetAttributeDescriptions();
					return materialPipelines.try_emplace(flags, Construct<MaterialPipelines>({ pipeLayoutIndex, "material_nn.vert.spv", flags, pipelineConfig, false })).first->second;
				}
			}
		}
		//in a release build, tag this unreachable
		assert(false);
		return nullptr;
	}
}