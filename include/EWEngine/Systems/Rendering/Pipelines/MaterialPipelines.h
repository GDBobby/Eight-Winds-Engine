#pragma once

#include <unordered_map>
#include "EWEngine/Data/EWE_Utils.h"
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Pipeline.h"

#ifndef MATERIAL_PIPE_LAYOUT_COUNT
#define MATERIAL_PIPE_LAYOUT_COUNT MAX_MATERIAL_TEXTURE_COUNT * 4
//can be thought of as a multidimensional array, with a size of [4][MAX_MATERIAL_TEXTURE_COUNT]
//[0][x] is a material without bones or instancing
//[1][x] is a material with bones but no instancing
//[2][x] is a material with instancing
//[3][x] is a material with instancing and bones

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
		MaterialPipelines(uint16_t pipeLayoutIndex, std::string const& vertFilePath, MaterialFlags flags, EWEPipeline::PipelineConfigInfo const& configInfo, bool hasBones);
		MaterialPipelines(uint16_t pipeLayoutIndex, uint16_t boneCount, MaterialFlags flags, EWEPipeline::PipelineConfigInfo const& configInfo);

		~MaterialPipelines();

		void BindPipeline();

		void BindModel(EWEModel* model);
		void BindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet);
		void BindDescriptor(uint8_t descSlot, const VkDescriptorSet* descSet);

		void Push(void* push);
		void PushAndDraw(void* push);
		void DrawModel();
		void DrawInstanced(EWEModel* model);
		void DrawInstanced(EWEModel* model, uint32_t instanceCount);

		static constexpr uint16_t GetPipeLayoutIndex(const MaterialFlags flags) {
			const bool hasBones = flags & MaterialF_hasBones;
			const bool instanced = flags & MaterialF_instanced;
			const bool hasBumps = flags & MaterialF_hasBump;
			const bool hasNormal = flags & MaterialF_hasNormal;
			const bool hasRough = flags & MaterialF_hasRough;
			const bool hasMetal = flags & MaterialF_hasMetal;
			const bool hasAO = flags & MaterialF_hasAO;
			//assert(!(hasBones && hasBumps));

			const uint8_t textureCount = hasNormal + hasRough + hasMetal + hasAO + hasBumps;
#if EWE_DEBUG
			const uint16_t pipeLayoutIndex = textureCount + (MAX_MATERIAL_TEXTURE_COUNT * (hasBones + (2 * instanced)));
			//printf("textureCount, hasBones, instanced - %d:%d:%d \n", textureCount, hasBones, instanced);
			return pipeLayoutIndex;
#else
			return textureCount + (MAX_MATERIAL_TEXTURE_COUNT * (hasBones + (2 * instanced)));
#endif
		}

	protected:
		uint16_t pipeLayoutIndex;
		EWEPipeline pipeline;
		EWEModel* bindedModel = nullptr;
		VkDescriptorSet bindedDescriptor{ VK_NULL_HANDLE };


		//static portion
    public:

		//pipelayout index is computed before passing in because the calling function is always using it as well
		static void InitMaterialPipeLayout(uint16_t materialPipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced, bool hasBump);
		static MaterialPipelines* GetMaterialPipe(MaterialFlags flags);
		static MaterialPipelines* GetMaterialPipe(MaterialFlags flags, uint16_t boneCount);

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

