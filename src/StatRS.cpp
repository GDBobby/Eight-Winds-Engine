#include "EWEngine/Systems/Rendering/Stationary/StatRS.h"

namespace EWE {
	StaticRenderSystem* StaticRenderSystem::skinnedMainObject{nullptr};

	StaticRenderSystem::StaticRenderSystem(uint32_t pipelineCount, uint32_t modelLimit) : modelLimit{ modelLimit } {
		pipelineStructs.reserve(pipelineCount);

		size_t alignment = 0;
		alignment = EWEDevice::GetEWEDevice()->GetProperties().limits.minStorageBufferOffsetAlignment;

		alignment = std::ceil(static_cast<double>((sizeof(glm::mat4) + sizeof(glm::mat3))) / alignment) * alignment;

		transformBuffer = std::make_unique<EWEBuffer>(sizeof(glm::mat4) + sizeof(glm::mat3), modelLimit, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);// , device.getProperties().limits.minStorageBufferOffsetAlignment);
	}

	void StaticRenderSystem::initStaticRS(uint32_t pipelineCount, uint32_t modelLimit) {
		if (skinnedMainObject != nullptr) {
			printf("trying to double init static RS \n");
			throw std::runtime_error("trying to double init StaticRenderSystem");
		}
		skinnedMainObject = reinterpret_cast<StaticRenderSystem*>(ewe_alloc(sizeof(StaticRenderSystem), 1));
		skinnedMainObject = new(skinnedMainObject) StaticRenderSystem(pipelineCount, modelLimit);


	}
	bool StaticRenderSystem::addStaticObject(uint16_t PipelineID, std::unique_ptr<EWEModel>& model, TextureDesc texture, TransformComponent& transform) {

		return false;
	}
	bool StaticRenderSystem::addStaticToBack() {

		return false;
	}
}