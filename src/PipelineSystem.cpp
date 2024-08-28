#include "EWEngine/Systems/PipelineSystem.h"

#include "EWEngine/Graphics/Texture/Texture_Manager.h"

namespace EWE {
	std::unordered_map<PipelineID, PipelineSystem*> PipelineSystem::pipelineSystem{};
	uint8_t PipelineSystem::frameIndex;
	VkCommandBuffer PipelineSystem::cmdBuf;
#if EWE_DEBUG
	PipelineID PipelineSystem::currentPipe;
#endif

	void PipelineSystem::SetFrameInfo(FrameInfo const& frameInfo) {
		cmdBuf = frameInfo.cmdBuf;
		frameIndex = frameInfo.index;
	}
	
	void PipelineSystem::Emplace(PipelineID pipeID, PipelineSystem* pipeSys) {
#if EWE_DEBUG
		assert(pipelineSystem.find(pipeID) == pipelineSystem.end() && "attempting to emplace a pipe with an existing id");
#endif
		pipelineSystem.emplace(pipeID, pipeSys);
	}

	void PipelineSystem::Destruct() {

		for (auto iter = pipelineSystem.begin(); iter != pipelineSystem.end(); iter++) {
			vkDestroyPipelineLayout(EWEDevice::GetVkDevice(), iter->second->pipeLayout, nullptr);
			ewe_free(iter->second);
		}

		pipelineSystem.clear();
	}
	void PipelineSystem::DestructAt(PipelineID pipeID) {
		auto foundPipe = pipelineSystem.find(pipeID);

		assert(foundPipe != pipelineSystem.end() && "destructing invalid pipe \n");

		vkDestroyPipelineLayout(EWEDevice::GetVkDevice(), foundPipe->second->pipeLayout, nullptr);
		foundPipe->second->~PipelineSystem();
	}

	PipelineSystem* PipelineSystem::At(PipelineID pipeID) {
		auto foundPipe = pipelineSystem.find(pipeID);

		assert(foundPipe != pipelineSystem.end() && "searching invalid pipe \n");
#if EWE_DEBUG
		currentPipe = pipeID;
#endif
		return foundPipe->second;
	}
	
	void PipelineSystem::BindPipeline() {
#if EWE_DEBUG
		assert(currentPipe == myID && "pipe id mismatch on model bind");
#endif
		pipe->bind(cmdBuf);
		bindedTexture = TEXTURE_UNBINDED_DESC;
	}
	void PipelineSystem::BindModel(EWEModel* model) {
		bindedModel = model;
#if EWE_DEBUG
		assert(currentPipe == myID && "pipe id mismatch on model bind");
#endif
		bindedModel->Bind(cmdBuf);
	}
	void PipelineSystem::BindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet) {
#if EWE_DEBUG
		assert(currentPipe == myID && "pipe id mismatch on desc bind");
#endif
		vkCmdBindDescriptorSets(cmdBuf,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeLayout,
			descSlot, 1,
			descSet, 
			0, nullptr
		);
		
	}
	//EWETexture::getDescriptorSets(tileSet.tileSetTexture, frameIndex)
	void PipelineSystem::BindTextureDescriptor(uint8_t descSlot, TextureDesc texID) {
		if (bindedTexture != texID) {
			BindDescriptor(descSlot, &texID);
			bindedTexture = texID;
		}
	}


	void PipelineSystem::Push(void* push) {
		vkCmdPushConstants(cmdBuf, pipeLayout, pushStageFlags, 0, pushSize, push);
	}

	void PipelineSystem::PushAndDraw(void* push) {
		vkCmdPushConstants(cmdBuf, pipeLayout, pushStageFlags, 0, pushSize, push);
		
#if EWE_DEBUG
		assert(bindedModel != nullptr && "attempting to draw a model while none is binded");
		assert(currentPipe == myID && "pipe id mismatch on model draw");
#endif
		bindedModel->Draw(cmdBuf);
	}
	void PipelineSystem::DrawModel() {
#if EWE_DEBUG
		assert(bindedModel != nullptr && "attempting to draw a model while none is binded");
		assert(currentPipe == myID && "pipe id mismatch on model draw");
#endif
		bindedModel->Draw(cmdBuf);
	}
	void PipelineSystem::DrawInstanced(EWEModel* model) {
		model->BindAndDrawInstance(cmdBuf);
	}
}