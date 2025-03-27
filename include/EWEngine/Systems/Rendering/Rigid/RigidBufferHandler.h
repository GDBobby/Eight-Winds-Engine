#pragma once

#include "EWEngine/Graphics/DescriptorHandler.h"

namespace EWE {
	class RigidInstancedBufferHandler {
	public:
		RigidInstancedBufferHandler(uint32_t entityCount, bool computedTransforms, EWEDescriptorSetLayout* eDSL, ImageID imgID);
		RigidInstancedBufferHandler(RigidInstancedBufferHandler&&) noexcept;
		RigidInstancedBufferHandler(RigidInstancedBufferHandler&);
		RigidInstancedBufferHandler& operator=(RigidInstancedBufferHandler& other);
		RigidInstancedBufferHandler& operator=(RigidInstancedBufferHandler&& other);
		~RigidInstancedBufferHandler();

		void WritePartialData(glm::mat4* transform, std::size_t offset);
		//this is for sequential writing only, it's recommended to write to the entire buffer
		void WritePartialData(glm::mat4* transform);
		void WriteFullData(glm::mat4* transformData);
		void ChangeEntityCount(uint32_t entity_count);
		void Flush();

		VkDescriptorBufferInfo* GetDescriptorBufferInfo(uint8_t whichFrame) {
			return transformBuffer[whichFrame]->DescriptorInfo();
		}
		uint32_t GetCurrentEntityCount() const {
			return currentEntityCount;
		}
		std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothBuffers() const {
			return transformBuffer;
		}
		const EWEBuffer* GetBuffer() const {
			return transformBuffer[VK::Object->frameIndex];
		}
		bool GetComputing() const {
			return computedTransforms;
		}

	private:
		std::array<EWEBuffer*, 2> transformBuffer{ nullptr, nullptr };

		std::size_t currentMemOffset{ 0 };

		uint32_t maxEntityCount;
		uint32_t currentEntityCount;
		bool computedTransforms;
	};
}//namespace EWE

