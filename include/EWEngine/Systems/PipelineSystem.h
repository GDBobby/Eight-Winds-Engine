#pragma once

#include "EWEngine/Graphics/Device.hpp"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Data/EngineDataTypes.h"

#include <unordered_map>
#include <memory>

namespace EWE {
	class PipelineSystem {
	protected:
		static std::unordered_map<PipelineID, PipelineSystem*> pipelineSystem;
#if EWE_DEBUG
		static PipelineID currentPipe;
		PipelineSystem(PipelineID pipeID) : myID{pipeID} {}
#else
		PipelineSystem() {}
#endif
		virtual void CreatePipeLayout() = 0;
		virtual void CreatePipeline() = 0;

	public:
		virtual ~PipelineSystem() {}
		static PipelineSystem* At(PipelineID pipeID);
		static void Emplace(PipelineID pipeID, PipelineSystem* pipeSys);
		static void Destruct();
		static void DestructAt(PipelineID pipeID);

		void BindPipeline();

		void BindModel(EWEModel* model);
		void BindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet);

		void Push(void* push);
		virtual void PushAndDraw(void* push);
		void DrawModel();
		virtual void DrawInstanced(EWEModel* model);
		
		[[nodiscard]] EWEDescriptorSetLayout* GetDSL() {
			return eDSL;
		}

	protected:
		std::unique_ptr<EWEPipeline> pipe{nullptr};
		VkPipelineLayout pipeLayout{};
		VkDescriptorSet bindedTexture = VK_NULL_HANDLE;
		//VkPipelineCache cache{VK_NULL_HANDLE};
		EWEModel* bindedModel = nullptr;
		VkShaderStageFlags pushStageFlags;
		uint32_t pushSize; 
		EWEDescriptorSetLayout* eDSL{};
#if EWE_DEBUG
		PipelineID myID;
#endif

	};
}