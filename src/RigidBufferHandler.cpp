#include "EWEngine/Systems/Rendering/Rigid/RigidBufferHandler.h"

namespace EWE {
	RigidInstancedBufferHandler::RigidInstancedBufferHandler(uint32_t entityCount, bool computedTransforms) : computedTransforms{ computedTransforms } {
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

		EWEDescriptorSetLayout* descLayout;

		EWEDescriptorSetLayout::Builder dslBuilder{};
		dslBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
		descLayout = dslBuilder.Build();
		{
			EWEDescriptorWriter descWriter{ descLayout, DescriptorPool_Global };
			descWriter.WriteBuffer(0, transformBuffer[0]->DescriptorInfo());
			descriptorSet[0] = descWriter.Build();
		} 
		{
			EWEDescriptorWriter descWriter{ descLayout, DescriptorPool_Global };
			descWriter.WriteBuffer(0, transformBuffer[1]->DescriptorInfo());
			descriptorSet[1] = descWriter.Build();
		}
	}

	RigidInstancedBufferHandler::~RigidInstancedBufferHandler() {
		Deconstruct(transformBuffer[0]);
		Deconstruct(transformBuffer[1]);
	}

	void RigidInstancedBufferHandler::WritePartialData(glm::mat4* transform, std::size_t offset) {
		transformBuffer[frameIndex]->WriteToBuffer(transform, sizeof(glm::mat4), offset);
	}
	void RigidInstancedBufferHandler::WritePartialData(glm::mat4* transform) {
		transformBuffer[frameIndex]->WriteToBuffer(transform, sizeof(glm::mat4), currentMemOffset);
		currentMemOffset += sizeof(glm::mat4);
	}
	void RigidInstancedBufferHandler::WriteFullData(glm::mat4* transform) {
		transformBuffer[frameIndex]->WriteToBuffer(transform, sizeof(glm::mat4) * currentEntityCount, 0);
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
	void RigidInstancedBufferHandler::SetFrameIndex(uint8_t frameIndex) {
		this->frameIndex = frameIndex;
		if (!computedTransforms) {
			transformBuffer[frameIndex]->Map();
		}
	}
	void RigidInstancedBufferHandler::Flush() {
#if EWE_DEBUG
		assert(!computedTransforms && "this buffer is unwriteable from CPU");
#endif
		transformBuffer[frameIndex]->Flush();
		currentMemOffset = 0;
		transformBuffer[frameIndex]->Unmap();
	}
}