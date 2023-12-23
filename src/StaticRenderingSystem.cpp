#include "EWEngine/systems/StaticRendering/StaticRenderingSystem.h"

namespace EWE {
	StaticRenderSystem* StaticRenderSystem::skinnedMainObject{nullptr};

	StaticRenderSystem::StaticRenderSystem(EWEDevice& device, uint32_t pipelineCount, uint32_t modelLimit) : modelLimit{ modelLimit } {
		pipelineStructs.reserve(pipelineCount);

		size_t alignment = 0;
		alignment = device.getProperties().limits.minStorageBufferOffsetAlignment;

		alignment = std::ceil(static_cast<double>((sizeof(glm::mat4) + sizeof(glm::mat3))) / alignment) * alignment;

		transformBuffer = std::make_unique<EWEBuffer>(device, sizeof(glm::mat4) + sizeof(glm::mat3), modelLimit, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);// , device.getProperties().limits.minStorageBufferOffsetAlignment);
	}

	void StaticRenderSystem::initStaticRS(EWEDevice& device, uint32_t pipelineCount, uint32_t modelLimit) {
		if (skinnedMainObject != nullptr) {
			printf("trying to double init static RS \n");
			throw std::exception("trying to double init StaticRenderSystem");
		}
		skinnedMainObject = new StaticRenderSystem(device, pipelineCount, modelLimit);


	}
}