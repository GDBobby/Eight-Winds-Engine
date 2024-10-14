#pragma once

#include "EWEngine/Graphics/DescriptorHandler.h"

namespace EWE {
	class InstancedSkinBufferHandler {
	public:

		InstancedSkinBufferHandler(uint16_t boneCount, uint16_t maxActorCount);

		void WriteData(glm::mat4* modelMatrix, void* finalBoneMatrices);
		void ChangeMaxActorCount(uint16_t actorCount);
		void Flush();

		void SetFrameIndex(uint8_t frameIndex) {
			//printf("setting instance count to 0 \n");
			this->frameIndex = frameIndex;
		}
		void ResetInstanceCount() {
			currentInstanceCount = 0;
		}
		uint16_t GetMaxActorCount() {
			return maxActorCount;
		}
		VkDescriptorSet* GetDescriptor() {
			return &gpuData[frameIndex].descriptor;
		}
		uint32_t GetInstanceCount() {
			return currentInstanceCount;
		}

	private:
		struct InnerBufferStruct {
			EWEBuffer* model;
			EWEBuffer* bone;
			VkDescriptorSet descriptor{};

			InnerBufferStruct(uint16_t maxActorCount, uint32_t boneBlockSize);
			~InnerBufferStruct() {
				Deconstruct(model);
				Deconstruct(bone);
			}

			void ChangeActorCount(uint16_t maxActorCount, uint32_t boneBlockSize);
			void BuildDescriptor(uint16_t maxActorCount);

			void Flush();
		private:
			bool updated = false;
		};
		std::vector<InnerBufferStruct> gpuData{};
		const uint32_t boneBlockSize;
		uint16_t maxActorCount{ 0 };

		uint64_t modelMemOffset = 0;
		uint64_t boneMemOffset = 0;
		uint8_t frameIndex{ 0 };
		uint32_t currentInstanceCount = 0;
	};

	class SkinBufferHandler {
	protected:
		struct InnerBufferStruct {
			EWEBuffer* bone;
			VkDescriptorSet descriptor;
			uint16_t currentActorCount = 0;

			InnerBufferStruct(uint8_t maxActorCount, uint32_t boneBlockSize);
			~InnerBufferStruct() {
				Deconstruct(bone);
			}

			void ChangeActorCount(uint8_t maxActorCount, uint32_t boneBlockSize);
			void BuildDescriptor();

			void Flush();
		private:
			bool updated = false;
		};

	public:

		SkinBufferHandler(uint16_t boneCount, uint8_t maxActorCount);
		SkinBufferHandler(uint8_t maxActorCount, std::vector<InnerBufferStruct>* referencedData);

		void WriteData(void* finalBoneMatrices);

		//changeActorCount should be done extremely infrequently. like, only on scene swaps.
		//if a more frequent change is required, instancing is recommended, even if the actor count is low.
		//	if a frequent change from 1-2 or 1 through 3 is required, maybe, MAYBE, MAYBE, 
		//	allocate the higher number and adjust this to ignore the additional memory when not necessary.
		//	i don't know if additional adjustments would be required or not, benchmarking against instancing is recommended
		void ChangeMaxActorCount(uint8_t actorCount);
		void Flush();
		bool CheckReference() {
			return gpuReference != nullptr;
		}
		void SetFrameIndex(uint8_t frameIndex);
		uint16_t GetMaxActorCount() {
			return maxActorCount;
		}
		VkDescriptorSet* GetDescriptor();
		std::vector<InnerBufferStruct>* GetInnerPtr() {
			return &gpuData;
		}
	private:

		std::vector<InnerBufferStruct>* gpuReference{nullptr};
		std::vector<InnerBufferStruct> gpuData{};
		const uint32_t boneBlockSize;
		uint8_t maxActorCount{ 1 };

		uint64_t boneMemOffset = 0;
		uint8_t frameIndex{0};
	};
}