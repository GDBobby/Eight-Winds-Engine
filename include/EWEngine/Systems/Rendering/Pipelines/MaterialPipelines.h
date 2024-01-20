#pragma once

#include <unordered_map>
#include "EWEngine/Systems/PipelineSystem.h"

#ifndef DYNAMIC_PIPE_LAYOUT_COUNT
#define DYNAMIC_PIPE_LAYOUT_COUNT 24
#endif

namespace EWE {
	struct BonePipeTrackerKey {
			uint16_t boneCount;
			MaterialFlags flags;
	};




	class MaterialPipelines : public PipelineSystem {
    public:
		static std::unordered_map<MaterialFlags, std::unique_ptr<EWEPipeline>> materialPipelines;

#ifdef _DEBUG
		static std::vector<MaterialFlags> bonePipeTracker;
		static std::vector<std::pair<uint16_t, MaterialFlags>> instancedBonePipeTracker;
#endif
		static VkPipelineLayout materialPipeLayout[DYNAMIC_PIPE_LAYOUT_COUNT];

		//pipelayout index is computed before passing in because the calling function is always using it as well
		static void initMaterialPipeLayout(uint16_t materialPipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced, EWEDevice& device, bool hasBump);
		static void constructMaterialPipe(MaterialFlags flags, VkPipelineRenderingCreateInfo const& pipeRenderInfo, EWEDevice& device);
		static void constructInstancedMaterial(MaterialFlags flags, uint16_t boneCount, VkPipelineRenderingCreateInfo const& pipeRenderInfo, EWEDevice& device);

		static void initStaticVariables();
		static void cleanupStaticVariables(EWEDevice& device);


		static void destruct(EWEDevice& device);

		void bindPipeline();

		void bindModel(EWEModel* model);
		void bindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet);

		void push(void* push);
		virtual void pushAndDraw(void* push);
		void drawModel();
		virtual void drawInstanced(EWEModel* model);

	protected:
		static std::vector<VkDescriptorSetLayout> getPipeDSL(uint8_t textureCount, bool hasBones, bool instanced, EWEDevice& device, bool hasBump);
		static VkPipelineCache materialPipelineCache;
		static VkPipelineCache boneMaterialPipelineCache;
		static VkPipelineCache instanceMaterialPipelineCache;
	};
}

