#include "EWEngine/Systems/PipelineSystem.h"

#include "EWEngine/Graphics/Texture/Image_Manager.h"

namespace EWE {
	std::unordered_map<PipelineID, PipelineSystem*> PipelineSystem::pipelineSystem{};
#if EWE_DEBUG
	PipelineID PipelineSystem::currentPipe;
#endif
	
	void PipelineSystem::Emplace(PipelineID pipeID, PipelineSystem* pipeSys) {
#if EWE_DEBUG
		assert(!pipelineSystem.contains(pipeID) && "attempting to emplace a pipe with an existing id");
#endif
		pipelineSystem.emplace(pipeID, pipeSys);
	}

	void PipelineSystem::Destruct() {

		for (auto iter = pipelineSystem.begin(); iter != pipelineSystem.end(); iter++) {
			EWE_VK(vkDestroyPipelineLayout, VK::Object->vkDevice, iter->second->pipeLayout, nullptr);
			Deconstruct(iter->second);
		}

		pipelineSystem.clear();
	}
	void PipelineSystem::DestructAt(PipelineID pipeID) {

#if EWE_DEBUG
		auto foundPipe = pipelineSystem.find(pipeID);
		assert(foundPipe != pipelineSystem.end() && "destructing invalid pipe \n");
		EWE_VK(vkDestroyPipelineLayout, VK::Object->vkDevice, foundPipe->second->pipeLayout, nullptr);
		Deconstruct(foundPipe->second);
#else
		auto& pipe = pipelineSystem.at(pipeID);
		EWE_VK(vkDestroyPipelineLayout, VK::Object->vkDevice, pipe->pipeLayout, nullptr);
		Deconstruct(pipe);
#endif

	}

	PipelineSystem* PipelineSystem::At(PipelineID pipeID) {
		auto foundPipe = pipelineSystem.find(pipeID);

#if EWE_DEBUG
		assert(foundPipe != pipelineSystem.end() && "searching invalid pipe \n");
#endif
		return foundPipe->second;
	}
	
	void PipelineSystem::BindPipeline() {
#if EWE_DEBUG
		currentPipe = myID;
#endif
		pipe->Bind();
		bindedTexture = VK_NULL_HANDLE;
	}
	void PipelineSystem::BindModel(EWEModel* model) {
		bindedModel = model;
#if EWE_DEBUG
		assert(currentPipe == myID && "pipe id mismatch on model bind");
#endif
		bindedModel->Bind();
	}
	void PipelineSystem::BindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet) {
#if EWE_DEBUG
		assert(currentPipe == myID && "pipe id mismatch on desc bind");
#endif
		EWE_VK(vkCmdBindDescriptorSets, VK::Object->GetFrameBuffer(),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeLayout,
			descSlot, 1,
			descSet, 
			0, nullptr
		);
		
	}

	void PipelineSystem::Push(void* push) {
		EWE_VK(vkCmdPushConstants, VK::Object->GetFrameBuffer(), pipeLayout, pushStageFlags, 0, pushSize, push);
	}

	void PipelineSystem::PushAndDraw(void* push) {
		EWE_VK(vkCmdPushConstants, VK::Object->GetFrameBuffer(), pipeLayout, pushStageFlags, 0, pushSize, push);
		
#if EWE_DEBUG
		assert(bindedModel != nullptr && "attempting to draw a model while none is binded");
		assert(currentPipe == myID && "pipe id mismatch on model draw");
#endif
		bindedModel->Draw();
	}
	void PipelineSystem::DrawModel() {
#if EWE_DEBUG
		assert(bindedModel != nullptr && "attempting to draw a model while none is binded");
		assert(currentPipe == myID && "pipe id mismatch on model draw");
#endif
		bindedModel->Draw();
	}
	void PipelineSystem::DrawInstanced(EWEModel* model) {
		model->BindAndDrawInstance();
	}
}