#include "EWEngine/Systems/PipelineSystem.h"

#include "EWEngine/Graphics/Texture/Texture_Manager.h"

namespace EWE {
	std::unordered_map<PipelineID, std::unique_ptr<PipelineSystem>> PipelineSystem::pipelineSystem{};
	uint8_t PipelineSystem::frameIndex;
	VkCommandBuffer PipelineSystem::cmdBuf;
#ifdef _DEBUG
	PipelineID PipelineSystem::currentPipe;
#endif

	void PipelineSystem::setFrameInfo(FrameInfo frameInfo) {
		cmdBuf = frameInfo.cmdBuf;
		frameIndex = frameInfo.index;
	}
	void PipelineSystem::destruct() {
		for (auto iter = pipelineSystem.begin(); iter != pipelineSystem.end(); iter++) {
			vkDestroyPipelineLayout(EWEDevice::GetVkDevice(), iter->second->pipeLayout, nullptr);
		}

		pipelineSystem.clear();
	}

	PipelineSystem* PipelineSystem::at(PipelineID pipeID) {
		if (pipelineSystem.find(pipeID) != pipelineSystem.end()) {
#ifdef _DEBUG
			currentPipe = pipeID;
#endif
			return pipelineSystem.at(pipeID).get();
		}
		else {
			printf("invalid pipe ::at %d \n", pipeID);

			throw std::runtime_error("searching for invlaid pipe \n");
		}
	}
	
	void PipelineSystem::bindPipeline() {
#ifdef _DEBUG
		if (currentPipe != myID) {

			printf("pipe id mismatch on bind \n");

			throw std::runtime_error("pipe id mismatch on bind");
		}
#endif
		pipe->bind(cmdBuf);
		bindedTexture = TEXTURE_UNBINDED_DESC;
	}
	void PipelineSystem::bindModel(EWEModel* model) {
		bindedModel = model;
#ifdef _DEBUG
		if (currentPipe != myID) {
			printf("failed model bind \n");
			throw std::runtime_error("pipe id mismatch on model bind");
		}
#endif
		bindedModel->bind(cmdBuf);
	}
	void PipelineSystem::bindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet) {
#ifdef _DEBUG
		if (currentPipe != myID) {
			printf("failed desc bind \n");
			throw std::runtime_error("pipe id mismatch on desc bind");
		}
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
	void PipelineSystem::bindTextureDescriptor(uint8_t descSlot, TextureDesc texID) {
		if (bindedTexture != texID) {
			bindDescriptor(descSlot, &texID);
			bindedTexture = texID;
		}
	}


	void PipelineSystem::push(void* push) {
		vkCmdPushConstants(cmdBuf, pipeLayout, pushStageFlags, 0, pushSize, push);
	}

	void PipelineSystem::pushAndDraw(void* push) {
		vkCmdPushConstants(cmdBuf, pipeLayout, pushStageFlags, 0, pushSize, push);
		
#ifdef _DEBUG
		if (bindedModel == nullptr) {
			printf("failed model draw \n");
			throw std::runtime_error("attempting to draw a model while none is binded");
		}
		if (currentPipe != myID) {
			printf("failed model draw \n");
			throw std::runtime_error("pipe id mismatch on model draw");
		}
#endif
		bindedModel->draw(cmdBuf);
	}
	void PipelineSystem::drawModel() {
#ifdef _DEBUG
		if (bindedModel == nullptr) {
			printf("failed model draw \n");
			throw std::runtime_error("attempting to draw a model while none is binded");
		}
		if (currentPipe != myID) {
			printf("failed model draw \n");
			throw std::runtime_error("pipe id mismatch on model draw");
		}
#endif
		bindedModel->draw(cmdBuf);
	}
	void PipelineSystem::drawInstanced(EWEModel* model) {
		model->BindAndDrawInstance(cmdBuf);
	}
}