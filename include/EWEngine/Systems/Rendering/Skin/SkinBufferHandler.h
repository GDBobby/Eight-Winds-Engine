#pragma once

#include "EWEngine/Graphics/DescriptorHandler.h"

namespace EWE {
	class InstancedSkinBufferHandler {
		//maybe use one buffer handler for instanced, one for non-instanced
		//otherwise I would need a bunch of if statements in here
		// if i go for 1 buffer handler for instanced, 1 for not instanced, i would have to specify actor type by instanced or not instanced, or have another handful of if statements in the actor
		//put a pointer to bufferstruct pointer in each actor
	public:

		InstancedSkinBufferHandler(uint16_t boneCount, uint16_t maxActorCount);

		void writeData(glm::mat4* modelMatrix, void* finalBoneMatrices);
		void changeMaxActorCount(uint16_t actorCount);
		void flush();

		void setFrameIndex(uint8_t frameIndex) {
			//printf("setting instance count to 0 \n");
			this->frameIndex = frameIndex;
		}
		void resetInstanceCount() {
			currentInstanceCount = 0;
		}
		uint16_t getMaxActorCount() {
			return maxActorCount;
		}
		VkDescriptorSet* getDescriptor() {
			return &gpuData[frameIndex].descriptor;
		}
		uint32_t getInstanceCount() {
			return currentInstanceCount;
		}

	private:
		struct InnerBufferStruct {
			EWEBuffer* model;
			EWEBuffer* bone;
			VkDescriptorSet descriptor{};

			InnerBufferStruct(uint16_t maxActorCount, uint32_t boneBlockSize);
			~InnerBufferStruct() {
				model->~EWEBuffer();
				bone->~EWEBuffer();
				ewe_free(model);
				ewe_free(bone);
			}

			//changeActorCount should be done extremely infrequently. like, only on scene swaps.
			//if a more frequent change is required, instancing is recommended, even if the actor count is low.
			//	if a frequent change from 1-2 or 1 through 3 is required, maybe, MAYBE, MAYBE, 
			//	allocate the higher number and adjust this to ignore the additional memory when not necessary.
			//	i don't know if additional adjustments would be required or not, benchmarking against instancing is recommended
			void changeActorCount(uint16_t maxActorCount, uint32_t boneBlockSize);
			void buildDescriptor(uint16_t maxActorCount);

			void flush();
		private:
			bool updated = false;
		};
		std::vector<InnerBufferStruct> gpuData{}; //model, bone
		const uint32_t boneBlockSize;
		uint16_t maxActorCount{ 0 };

		uint64_t modelMemOffset = 0;
		uint64_t boneMemOffset = 0;
		uint8_t frameIndex{ 0 };
		uint32_t currentInstanceCount = 0;
	};

	class SkinBufferHandler {
		//maybe use one buffer handler for instanced, one for non-instanced
		//otherwise I would need a bunch of if statements in here
		// if i go for 1 buffer handler for instanced, 1 for not instanced, i would have to specify actor type by instanced or not instanced, or have another handful of if statements in the actor
		//put a pointer to bufferstruct pointer in each actor
	protected:
		struct InnerBufferStruct {
			EWEBuffer* bone;
			VkDescriptorSet descriptor;
			uint16_t currentActorCount = 0;

			InnerBufferStruct(uint8_t maxActorCount, uint32_t boneBlockSize);
			~InnerBufferStruct() {
				bone->~EWEBuffer();
				ewe_free(bone);
			}

			void changeActorCount(uint8_t maxActorCount, uint32_t boneBlockSize);
			void buildDescriptor();

			void flush() {
				if (updated) {
					bone->Flush();
					updated = false;
				}
			}
		private:
			bool updated = false;
		};

	public:

		SkinBufferHandler(uint16_t boneCount, uint8_t maxActorCount);
		SkinBufferHandler(uint8_t maxActorCount, std::vector<InnerBufferStruct>* referencedData);

		void writeData(void* finalBoneMatrices);
		void changeMaxActorCount(uint8_t actorCount);
		void flush() {
			gpuData[frameIndex].flush();
			boneMemOffset = 0;
		}
		bool CheckReference() {
			return gpuReference != nullptr;
		}
		void setFrameIndex(uint8_t frameIndex) {
			this->frameIndex = frameIndex;
		}
		uint16_t getMaxActorCount() {
			return maxActorCount;
		}
		VkDescriptorSet* getDescriptor() {
			if (gpuReference == nullptr) {
				return &gpuData[frameIndex].descriptor;
			}
			else {
				return &gpuReference->at(frameIndex).descriptor;
			}
		}
		std::vector<InnerBufferStruct>* getInnerPtr() {
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