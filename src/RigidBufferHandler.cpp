#include "EWEngine/Systems/Rendering/Rigid/RigidBufferHandler.h"

#include "EWEngine/Graphics/Texture/Image_Manager.h"

namespace EWE {
	RigidInstancedBufferHandler::RigidInstancedBufferHandler(uint32_t entityCount, bool computedTransforms, EWEDescriptorSetLayout* eDSL, ImageID imgID) : computedTransforms{ computedTransforms } {
		VkMemoryPropertyFlagBits memoryFlags;
		if (computedTransforms) {
			memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		else {
			memoryFlags = static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		maxEntityCount = 100000; //need to call device limits and get the maximum size of a storage buffer there. not sure i would want to max that out tho
		currentEntityCount = entityCount;

		transformBuffer[0] = Construct<EWEBuffer>({ maxEntityCount * sizeof(glm::mat4), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, memoryFlags });
		transformBuffer[1] = Construct<EWEBuffer>({ maxEntityCount * sizeof(glm::mat4), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, memoryFlags });
	}

	RigidInstancedBufferHandler::~RigidInstancedBufferHandler() {
		Deconstruct(transformBuffer[0]);
		Deconstruct(transformBuffer[1]);
	}

	void RigidInstancedBufferHandler::WritePartialData(glm::mat4* transform, std::size_t offset) {
		transformBuffer[VK::Object->frameIndex]->WriteToBuffer(transform, sizeof(glm::mat4), offset);
	}
	void RigidInstancedBufferHandler::WritePartialData(glm::mat4* transform) {
		transformBuffer[VK::Object->frameIndex]->WriteToBuffer(transform, sizeof(glm::mat4), currentMemOffset);
		currentMemOffset += sizeof(glm::mat4);
	}
	void RigidInstancedBufferHandler::WriteFullData(glm::mat4* transform) {
		transformBuffer[VK::Object->frameIndex]->WriteToBuffer(transform, sizeof(glm::mat4) * currentEntityCount, 0);
	}

	void RigidInstancedBufferHandler::ChangeEntityCount(uint32_t entityCount) {
		//if (entityCount == maxEntityCount) {
		//	return;
		//}
		//if (maxEntityCount < 1024 && entityCount > 1024) {
		//	//swap from a uniform buffer to a storage buffer
		//}
		//if (maxEntityCount > 1024 && entityCount <= 1024) {
		//	//swap from a storage buffer to a uniform buffer
		//}
		
		
#if EWE_DEBUG
		//until i set up the storage/uniform swap, swapping is disabled
		assert(entityCount <= maxEntityCount);
#endif
		currentEntityCount = entityCount;
	}
	void RigidInstancedBufferHandler::Flush() {
#if EWE_DEBUG
		assert(!computedTransforms && "this buffer is not writeable from CPU");
#endif
		transformBuffer[VK::Object->frameIndex]->Flush();
		currentMemOffset = 0;
		transformBuffer[VK::Object->frameIndex]->Unmap();
	}
}