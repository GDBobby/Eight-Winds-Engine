#pragma once

#include "EWEngine/Data/EWE_Utils.h"
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Pipeline.h"

#include <unordered_map>

#ifndef MATERIAL_PIPE_LAYOUT_COUNT
//can be thought of as a multidimensional array, with a size of [Material::Atributes::Vertex::COUNT][Material::Attributes::Texture::SIZE]
#define MATERIAL_PIPE_LAYOUT_COUNT (Material::Attributes::Texture::SIZE << (Material::Attributes::Other::COUNT - Material::Attributes::Texture::SIZE))
#endif

namespace EWE {
	struct SkinInstanceKey {
		uint16_t boneCount;
		MaterialFlags matFlags;

		SkinInstanceKey(uint16_t boneCount, MaterialFlags flags) : boneCount{ boneCount }, matFlags{ flags } {}

		bool operator==(SkinInstanceKey const& other) const {
			return (boneCount == other.boneCount) && (matFlags == other.matFlags);
		}
	};
}
template<>
struct std::hash<EWE::SkinInstanceKey> {
	size_t operator()(EWE::SkinInstanceKey const& bptKey) const {
		size_t seed = 0;
		EWE::HashCombine(seed, static_cast<uint32_t>(bptKey.boneCount), static_cast<uint32_t>(bptKey.matFlags));
		return seed;
	}
};

namespace EWE{
	class MaterialPipelines {
		//object portion
	public:
		//	EWEPipeline(std::string const& vertFilepath, std::string const& fragFilepath, PipelineConfigInfo const& configInfo);
		//	EWEPipeline(VkShaderModule vertShaderModu, VkShaderModule fragShaderModu, PipelineConfigInfo const& configInfo);
		//	EWEPipeline(std::string const& vertFilePath, MaterialFlags flags, PipelineConfigInfo const& configInfo, bool hasBones);
		//	EWEPipeline(uint16_t boneCount, MaterialFlags flags, PipelineConfigInfo const& configInfo);
		MaterialPipelines(uint16_t pipeLayoutIndex, std::string const& vertFilepath, std::string const& fragFilepath, EWEPipeline::PipelineConfigInfo const& configInfo);
		MaterialPipelines(uint16_t pipeLayoutIndex, VkShaderModule vertShaderModu, VkShaderModule fragShaderModu, EWEPipeline::PipelineConfigInfo const& configInfo);
		MaterialPipelines(uint16_t pipeLayoutIndex, std::string const& vertFilePath, MaterialFlags flags, EWEPipeline::PipelineConfigInfo& configInfo);
		MaterialPipelines(uint16_t pipeLayoutIndex, uint16_t boneCount, MaterialFlags flags, EWEPipeline::PipelineConfigInfo const& configInfo);

		~MaterialPipelines();

		void BindPipeline();

		void BindModel(EWEModel* model);
		void BindDescriptor(const uint8_t descSlot, VkDescriptorSet* descSet);
		void BindDescriptor(const uint8_t descSlot, const VkDescriptorSet* descSet);

		void Push(void* push);
		void PushAndDraw(void* push);
		void DrawModel();
		void DrawInstanced(EWEModel* model);
		void DrawInstanced(EWEModel* model, uint32_t instanceCount);

	protected:
		uint16_t pipeLayoutIndex;
		EWEPipeline pipeline;
		EWEModel* bindedModel = nullptr;
		VkDescriptorSet bindedDescriptor{ VK_NULL_HANDLE };


		//static portion
    public:

		//pipelayout index is computed before passing in because the calling function is always using it as well
		static void InitMaterialPipeLayout(uint16_t materialPipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced, bool hasBump, bool generatingNormals);
		static MaterialPipelines* GetMaterialPipe(MaterialFlags flags);

		//this is pretty shitty, entitycount could alos be bonecount if it has bones and not instancing or something. not even sure if it can be instanced and boned
		static MaterialPipelines* GetMaterialPipe(MaterialFlags flags, uint16_t entityCount);

		static void InitStaticVariables();
		static void CleanupStaticVariables();

		static MaterialPipelines* At(MaterialFlags flags);
		static MaterialPipelines* At(SkinInstanceKey skinInstanceKey);
		static MaterialPipelines* At(uint16_t boneCount, MaterialFlags flags);

		static EWEDescriptorSetLayout* GetDSL(uint16_t pipeLayoutIndex);
		static EWEDescriptorSetLayout* GetDSLFromFlags(MaterialFlags flags);

	protected:
		static std::unordered_map<MaterialFlags, MaterialPipelines*> materialPipelines;
		static std::unordered_map<SkinInstanceKey, MaterialPipelines*> instancedBonePipelines;

#if EWE_DEBUG
		static std::vector<MaterialFlags> bonePipeTracker;
		static std::vector<std::pair<uint16_t, MaterialFlags>> instancedBonePipeTracker;
		static MaterialPipelines* currentPipe;
#endif
		static MaterialPipelines* CreatePipe(EWEPipeline::PipelineConfigInfo& pipelineConfig, MaterialFlags flags);
	};
}

