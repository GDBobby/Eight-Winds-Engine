#pragma once

#include "EWEngine/Graphics/DescriptorHandler.h"

namespace EWE {
	class RigidInstancedBufferHandler {
	public:
		RigidInstancedBufferHandler(uint32_t entityCount, bool computedTransforms);

		void WritePartialData(glm::mat4* transform, std::size_t offset);
		//this is for sequential writing only, it's recommended to write to the entire buffer
		void WritePartialData(glm::mat4* transform);
		void WriteFullData(glm::mat4* transformData);
		void ChangeEntityCount(uint32_t entity_count);
		void Flush();

		void SetFrameIndex(uint8_t frameIndex);

		VkDescriptorBufferInfo* GetDescriptorInfo() {
			return transformBuffer[frameIndex]->DescriptorInfo();
		}
		uint32_t GetCurrentEntityCount() const {
			return currentEntityCount;
		}
	private:
		EWEBuffer* transformBuffer[MAX_FRAMES_IN_FLIGHT] = { nullptr, nullptr };

		std::size_t currentMemOffset{ 0 };

		uint32_t maxEntityCount;
		uint32_t currentEntityCount;
		uint8_t frameIndex; 
		bool computedTransforms;
	};
}//namespace EWE

