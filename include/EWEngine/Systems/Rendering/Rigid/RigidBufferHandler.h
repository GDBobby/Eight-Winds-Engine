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

		void WritePartialTransformData(glm::mat4* transform, std::size_t offset);
		//this is for sequential writing only, it's recommended to write to the entire buffer
		void WritePartialTransformData(glm::mat4* transform);
		void WriteFullTransformData(glm::mat4* transformData);

		void WritePartialMaterialData(MaterialBuffer* material, std::size_t offset);
		//this is for sequential writing only, it's recommended to write to the entire buffer
		void WritePartialMaterialData(MaterialBuffer* material);
		void WriteFullMaterialData(MaterialBuffer* material);
		void ChangeEntityCount(uint32_t entity_count);
		void Flush();

		VkDescriptorBufferInfo* GetTransformDescriptorBufferInfo(uint8_t whichFrame) {
			return transformBuffer[whichFrame]->DescriptorInfo();
		}
		std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothTransformBuffers() const {
			return transformBuffer;
		}
		const EWEBuffer* GetTransformBuffer() const {
			return transformBuffer[VK::Object->frameIndex];
		}
		VkDescriptorBufferInfo* GetMaterialDescriptorBufferInfo(uint8_t whichFrame) {
			return materialBuffer[whichFrame]->DescriptorInfo();
		}
		std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothMaterialBuffers() const {
			return materialBuffer;
		}
		const EWEBuffer* GetMaterialBuffer() const {
			return materialBuffer[VK::Object->frameIndex];
		}

		bool GetComputing() const {
			return computedTransforms;
		}
		uint32_t GetCurrentEntityCount() const {
			return currentEntityCount;
		}

	private:
		std::array<EWEBuffer*, 2> transformBuffer{ nullptr, nullptr };
		std::array<EWEBuffer*, 2> materialBuffer{ nullptr, nullptr };

		std::size_t currentMemOffset{ 0 };

		uint32_t maxEntityCount;
		uint32_t currentEntityCount;
		bool computedTransforms;
	};
}//namespace EWE

