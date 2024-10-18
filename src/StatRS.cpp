#include "EWEngine/Systems/Rendering/Stationary/StatRS.h"

namespace EWE {
	StaticRenderSystem* statMainObject{nullptr};

	void StaticRenderSystem::Init(uint32_t pipelineCount, uint32_t modelLimit) {
		this->modelLimit = modelLimit;
		pipelineStructs.reserve(pipelineCount);

		size_t alignment = 0;
		alignment = EWEDevice::GetEWEDevice()->GetProperties().limits.minStorageBufferOffsetAlignment;

		alignment = std::ceil(static_cast<double>((sizeof(glm::mat4) + sizeof(glm::mat3))) / alignment) * alignment;

		transformBuffer = std::make_unique<EWEBuffer>(sizeof(glm::mat4) + sizeof(glm::mat3), modelLimit, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);// , device.getProperties().limits.minStorageBufferOffsetAlignment);
	}

	void StaticRenderSystem::InitStaticRS(uint32_t pipelineCount, uint32_t modelLimit) {
		assert(statMainObject == nullptr && "double init");

		statMainObject = Construct<StaticRenderSystem>({});
		statMainObject->Init(pipelineCount, modelLimit);
	}
	void StaticRenderSystem::DestructStaticRS() {

		Deconstruct(statMainObject);
	}
	bool StaticRenderSystem::AddStaticObject(uint16_t PipelineID, std::unique_ptr<EWEModel>& model, TextureDesc texture, TransformComponent& transform) {

		return false;
	}
	bool StaticRenderSystem::AddStaticToBack() {

		return false;
	}
}