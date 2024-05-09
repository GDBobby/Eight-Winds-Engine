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
		static std::unordered_map<PipelineID, PipelineSystem*> pipelineSystem;
		static uint8_t frameIndex;
		static VkCommandBuffer cmdBuf;
#ifdef _DEBUG
		static PipelineID currentPipe;
		PipelineSystem(PipelineID pipeID) : myID{pipeID} {}
#else
		PipelineSystem() {}
#endif
		~PipelineSystem() {}
		virtual void CreatePipeLayout() = 0;
		virtual void CreatePipeline() = 0;

	public:
		static void SetFrameInfo(FrameInfo const& frameInfo);
		static PipelineSystem* At(PipelineID pipeID);
		static void Emplace(PipelineID pipeID, PipelineSystem* pipeSys);
		static void Destruct();
		static void DestructAt(PipelineID pipeID);

		void BindPipeline();

		void BindModel(EWEModel* model);
		void BindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet);
		void BindTextureDescriptor(uint8_t descSlot, TextureDesc texID);

		void Push(void* push);
		virtual void PushAndDraw(void* push);
		void DrawModel();
		virtual void DrawInstanced(EWEModel* model);

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