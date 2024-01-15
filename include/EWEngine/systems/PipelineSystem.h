#pragma once

#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/PushConstants.h"
#include <EWEngine/Data/EngineDataTypes.h>

#include <unordered_map>
#include <stdexcept>
#include <memory>

namespace EWE {
	class PipelineSystem {
	protected:
		static std::unordered_map<PipelineID, std::unique_ptr<PipelineSystem>> pipelineSystem;
		static uint8_t frameIndex;
		static VkCommandBuffer cmdBuf;
#ifdef _DEBUG
		static PipelineID currentPipe;
#endif
		PipelineSystem() {}
		virtual void createPipeLayout(EWEDevice& device) = 0;
		virtual void createPipeline(EWEDevice& device, VkPipelineRenderingCreateInfo const& pipeRenderInfo) = 0;

	public:
		static void setCmdIndexPair(std::pair<VkCommandBuffer, uint8_t> cmdIndexPair);
		static PipelineSystem* at(PipelineID pipeID);
		static void emplace(PipelineID pipeID, PipelineSystem* pipeSys) {
#ifdef _DEBUG
			if (pipelineSystem.find(pipeID) != pipelineSystem.end()) {
				throw std::runtime_error("attempting to emplace a pipe with an existing id");
			}
			pipeSys->myID = pipeID;
#endif
			pipelineSystem.emplace(pipeID, pipeSys);
		}
		static void destruct(EWEDevice& device);

		void bindPipeline();

		void bindModel(EWEModel* model);
		void bindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet);

		void push(void* push);
		virtual void pushAndDraw(void* push);
		void drawModel();
		virtual void drawInstanced(EWEModel* model);



	protected:
		std::unique_ptr<EWEPipeline> pipe{nullptr};
		VkPipelineLayout pipeLayout{};
		TextureID bindedTexture = TEXTURE_UNBINDED;
		VkPipelineCache cache{VK_NULL_HANDLE};
		EWEModel* bindedModel = nullptr;
		VkShaderStageFlags pushStageFlags;
		uint32_t pushSize;
#ifdef _DEBUG
		PipelineID myID;
#endif

	};
}