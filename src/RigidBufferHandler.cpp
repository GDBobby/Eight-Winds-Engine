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
		materialBuffer[0] = Construct<EWEBuffer>({ maxEntityCount * sizeof(MaterialBuffer), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, memoryFlags });
		materialBuffer[1] = Construct<EWEBuffer>({ maxEntityCount * sizeof(MaterialBuffer), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, memoryFlags });

#if DEBUG_NAMING
		transformBuffer[0]->SetName("rigid transform buffer 0");
		transformBuffer[1]->SetName("rigid transform buffer 1");
		materialBuffer[0]->SetName("rigid material buffer 0");
		materialBuffer[1]->SetName("rigid material buffer 1");
#endif
	}
	RigidInstancedBufferHandler::RigidInstancedBufferHandler(RigidInstancedBufferHandler&& other) noexcept
		:
		transformBuffer{ other.transformBuffer },
		materialBuffer{other.materialBuffer},
		computedTransforms{ other.computedTransforms },
		maxEntityCount{ other.maxEntityCount },
		currentEntityCount{ other.currentEntityCount }
	{
		other.transformBuffer[0] = nullptr;
		other.transformBuffer[1] = nullptr;
		other.materialBuffer[0] = nullptr;
		other.materialBuffer[1] = nullptr;
	}

	RigidInstancedBufferHandler::~RigidInstancedBufferHandler() {
		if (transformBuffer[0] != nullptr) {
			Deconstruct(transformBuffer[0]);
			Deconstruct(transformBuffer[1]);
		}
		if (materialBuffer[0] != nullptr) {
			Deconstruct(materialBuffer[0]);
			Deconstruct(materialBuffer[1]);
		}
	}

	RigidInstancedBufferHandler::RigidInstancedBufferHandler(RigidInstancedBufferHandler& other) 
		:
		transformBuffer{other.transformBuffer},
		materialBuffer{ other.materialBuffer },
		computedTransforms{other.computedTransforms},
		maxEntityCount{other.maxEntityCount},
		currentEntityCount{other.currentEntityCount}
	{
		other.transformBuffer[0] = nullptr;
		other.transformBuffer[1] = nullptr;
		other.materialBuffer[0] = nullptr;
		other.materialBuffer[1] = nullptr;
	}
	RigidInstancedBufferHandler& RigidInstancedBufferHandler::operator=(RigidInstancedBufferHandler& other) {
		transformBuffer[0] = other.transformBuffer[0];
		transformBuffer[1] = other.transformBuffer[1];
		materialBuffer[0] = other.materialBuffer[0];
		materialBuffer[1] = other.materialBuffer[1];
		other.transformBuffer[0] = nullptr;
		other.transformBuffer[1] = nullptr;
		other.materialBuffer[0] = nullptr;
		other.materialBuffer[1] = nullptr;
		computedTransforms = other.computedTransforms;
		maxEntityCount = other.maxEntityCount;
		currentEntityCount = other.currentEntityCount;

		return *this;
	}
	RigidInstancedBufferHandler& RigidInstancedBufferHandler::operator=(RigidInstancedBufferHandler&& other) {
		transformBuffer[0] = other.transformBuffer[0];
		transformBuffer[1] = other.transformBuffer[1];
		materialBuffer[0] = other.materialBuffer[0];
		materialBuffer[1] = other.materialBuffer[1];
		other.transformBuffer[0] = nullptr;
		other.transformBuffer[1] = nullptr;
		other.materialBuffer[0] = nullptr;
		other.materialBuffer[1] = nullptr;
		computedTransforms = other.computedTransforms;
		maxEntityCount = other.maxEntityCount;
		currentEntityCount = other.currentEntityCount;

		return *this;
	}
	void RigidInstancedBufferHandler::WritePartialTransformData(glm::mat4* transform, std::size_t offset) {
		transformBuffer[VK::Object->frameIndex]->WriteToBuffer(transform, sizeof(glm::mat4), offset);
	}
	void RigidInstancedBufferHandler::WritePartialTransformData(glm::mat4* transform) {
		transformBuffer[VK::Object->frameIndex]->WriteToBuffer(transform, sizeof(glm::mat4), currentMemOffset);
		currentMemOffset += sizeof(glm::mat4);
	}
	void RigidInstancedBufferHandler::WriteFullTransformData(glm::mat4* transform) {
		transformBuffer[VK::Object->frameIndex]->WriteToBuffer(transform, sizeof(glm::mat4) * currentEntityCount, 0);
	}

	void RigidInstancedBufferHandler::WritePartialMaterialData(MaterialBuffer* material, std::size_t offset) {
		materialBuffer[VK::Object->frameIndex]->WriteToBuffer(material, sizeof(MaterialBuffer), offset);
	}
	void RigidInstancedBufferHandler::WritePartialMaterialData(MaterialBuffer* material) {
		materialBuffer[VK::Object->frameIndex]->WriteToBuffer(material, sizeof(MaterialBuffer), currentMemOffset);
		currentMemOffset += sizeof(MaterialBuffer);
	}
	void RigidInstancedBufferHandler::WriteFullMaterialData(MaterialBuffer* material) {
		materialBuffer[VK::Object->frameIndex]->WriteToBuffer(material, sizeof(MaterialBuffer) * currentEntityCount, 0);
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