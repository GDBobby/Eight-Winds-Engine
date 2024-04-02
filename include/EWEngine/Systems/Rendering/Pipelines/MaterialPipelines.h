#pragma once

#include <unordered_map>
#include "EWEngine/Data/EWE_Utils.h"
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Pipeline.h"

#ifndef DYNAMIC_PIPE_LAYOUT_COUNT
#define DYNAMIC_PIPE_LAYOUT_COUNT MAX_MATERIAL_TEXTURE_COUNT * 4
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

		void bindPipeline();

		void bindModel(EWEModel* model);
		void bindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet);
		void bindTextureDescriptor(uint8_t descSlot, TextureDesc texID);

		void push(void* push);
		void pushAndDraw(void* push);
		void drawModel();
		void drawInstanced(EWEModel* model);

	protected:
		uint16_t pipeLayoutIndex;
		EWEPipeline pipeline;
		EWEModel* bindedModel = nullptr;
		TextureDesc bindedTexture{ TEXTURE_UNBINDED_DESC };


		//static portion
    public:

		//pipelayout index is computed before passing in because the calling function is always using it as well
		static void initMaterialPipeLayout(uint16_t materialPipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced, bool hasBump);
		static MaterialPipelines* getMaterialPipe(MaterialFlags flags);
		static MaterialPipelines* getInstancedSkinMaterialPipe(uint16_t boneCount, MaterialFlags flags);

		static void initStaticVariables();
		static void cleanupStaticVariables();

		static MaterialPipelines* at(MaterialFlags flags);
		static MaterialPipelines* at(SkinInstanceKey skinInstanceKey);
		static MaterialPipelines* at(uint16_t boneCount, MaterialFlags flags);
		static void setFrameInfo(FrameInfo const& frameInfo);

	protected:
		static std::unordered_map<MaterialFlags, MaterialPipelines*> materialPipelines;
		static std::unordered_map<SkinInstanceKey, MaterialPipelines*> instancedBonePipelines;

		struct MaterialPipeLayoutInfo {
			VkPipelineLayout pipeLayout{ VK_NULL_HANDLE };
			size_t pushSize;
			VkShaderStageFlags pushStageFlags;

			void push(VkCommandBuffer cmdBuf, void* pushData) {
				vkCmdPushConstants(cmdBuf, pipeLayout, pushStageFlags, 0, pushSize, pushData);
			}
		};

		static MaterialPipeLayoutInfo materialPipeLayout[DYNAMIC_PIPE_LAYOUT_COUNT];

#ifdef _DEBUG
		static std::vector<MaterialFlags> bonePipeTracker;
		static std::vector<std::pair<uint16_t, MaterialFlags>> instancedBonePipeTracker;
		static MaterialPipelines* currentPipe;
#endif
		static uint8_t frameIndex;
		static VkCommandBuffer cmdBuf;

		static std::vector<VkDescriptorSetLayout> getPipeDSL(uint8_t textureCount, bool hasBones, bool instanced, bool hasBump);
		static VkPipelineCache materialPipelineCache;
		static VkPipelineCache skinPipelineCache;
		static VkPipelineCache instanceSkinPipelineCache;

		static void getPipeCache(bool hasBones, bool instanced, VkPipelineCache& outCache);
		static MaterialPipelines* createPipe(uint16_t pipeLayoutIndex, EWEPipeline::PipelineConfigInfo& pipelineConfig, bool hasBones, bool hasNormal, bool hasBumps, MaterialFlags flags);
	};
}

