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
		PipelineSystem(PipelineID pipeID) : myID{pipeID} {}
#else
		PipelineSystem() {}
#endif
		virtual void createPipeLayout() = 0;
		virtual void createPipeline() = 0;

	public:
		static void setFrameInfo(FrameInfo frameInfo);
		static PipelineSystem* at(PipelineID pipeID);
		static void emplace(PipelineID pipeID, PipelineSystem* pipeSys) {
#ifdef _DEBUG
			if (pipelineSystem.find(pipeID) != pipelineSystem.end()) {
				throw std::runtime_error("attempting to emplace a pipe with an existing id");
			}
#endif
			pipeSys->construct();
			pipelineSystem.emplace(pipeID, pipeSys);
		}
		static void destruct();

		void bindPipeline();

		void bindModel(EWEModel* model);
		void bindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet);
		void bindTextureDescriptor(uint8_t descSlot, TextureDesc texID);

		void push(void* push);
		virtual void pushAndDraw(void* push);
		void drawModel();
		virtual void drawInstanced(EWEModel* model);

		virtual void construct() = 0;

	protected:
		std::unique_ptr<EWEPipeline> pipe{nullptr};
		VkPipelineLayout pipeLayout{};
		TextureDesc bindedTexture = TEXTURE_UNBINDED_DESC;
		//VkPipelineCache cache{VK_NULL_HANDLE};
		EWEModel* bindedModel = nullptr;
		VkShaderStageFlags pushStageFlags;
		uint32_t pushSize;
#ifdef _DEBUG
		PipelineID myID;
#endif

	};
}