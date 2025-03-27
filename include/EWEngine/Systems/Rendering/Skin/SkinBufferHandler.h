#pragma once

#include "EWEngine/Graphics/DescriptorHandler.h"

namespace EWE {
	class InstancedSkinBufferHandler {
	private:
		struct InnerBufferStruct {
			EWEBuffer* model;
			EWEBuffer* bone;

			InnerBufferStruct(uint16_t maxActorCount, uint32_t boneBlockSize);
			~InnerBufferStruct() {
				Deconstruct(model);
				Deconstruct(bone);
			}

			void AddDescriptorBindings(uint16_t maxActorCount, EWEDescriptorWriter& descWriter);

			void Flush();
		private:
			bool updated = false;
		};
	public:

		InstancedSkinBufferHandler(uint16_t boneCount, uint16_t maxActorCount);

		void WriteData(glm::mat4* modelMatrix, void* finalBoneMatrices);
		void Flush();

		void ResetInstanceCount() {
			currentInstanceCount = 0;
		}
		uint16_t GetMaxActorCount() {
			return maxActorCount;
		}
		uint32_t GetInstanceCount() {
			return currentInstanceCount;
		}
		void AddDescriptorBindings(uint8_t whichFrame, EWEDescriptorWriter& descWriter) {
			gpuData[whichFrame].AddDescriptorBindings(maxActorCount, descWriter);
		}

	private:
		const uint32_t boneBlockSize;
		uint16_t maxActorCount{ 0 };
		std::array<InnerBufferStruct, MAX_FRAMES_IN_FLIGHT> gpuData;

		uint64_t modelMemOffset = 0;
		uint64_t boneMemOffset = 0;
		uint8_t frameIndex{ 0 };
		uint32_t currentInstanceCount = 0;
	};

	class SkinBufferHandler {
	protected:
		struct InnerBufferStruct {
			EWEBuffer* bone;
			uint16_t currentActorCount = 0;

			InnerBufferStruct(uint8_t maxActorCount, uint32_t boneBlockSize);
			~InnerBufferStruct() {
				Deconstruct(bone);
			}

			void AddDescriptorBindings(EWEDescriptorWriter& descWriter);

			void Flush();
		private:
			bool updated = false;
		};

	public:

		SkinBufferHandler(uint16_t boneCount, uint8_t maxActorCount);

		void WriteData(void* finalBoneMatrices);

		//changeActorCount should be done extremely infrequently. like, only on scene swaps.
		//if a more frequent change is required, instancing is recommended, even if the actor count is low.
		//	if a frequent change from 1-2 or 1 through 3 is required, maybe, MAYBE, MAYBE, 
		//	allocate the higher number and adjust this to ignore the additional memory when not necessary.
		//	i don't know if additional adjustments would be required or not, benchmarking against instancing is recommended
		void Flush();
		uint16_t GetMaxActorCount() {
			return maxActorCount;
		}
		void AddDescriptorBindings(uint8_t whichFrame, EWEDescriptorWriter& descWriter) {
			gpuData[whichFrame].AddDescriptorBindings(descWriter);
		}
	private:

		const uint32_t boneBlockSize;
		uint8_t maxActorCount{ 1 };
		std::array<InnerBufferStruct, MAX_FRAMES_IN_FLIGHT> gpuData;

		uint64_t boneMemOffset = 0;
	};
}