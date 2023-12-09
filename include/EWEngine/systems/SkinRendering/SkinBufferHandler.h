#pragma once

#include "../../graphics/DescriptorHandler.h"

namespace EWE {
	class InstancedSkinBufferHandler {
		//maybe use one buffer handler for instanced, one for non-instanced
		//otherwise I would need a bunch of if statements in here
		// if i go for 1 buffer handler for instanced, 1 for not instanced, i would have to specify actor type by instanced or not instanced, or have another handful of if statements in the actor
		//put a pointer to bufferstruct pointer in each actor
	public:

		InstancedSkinBufferHandler(EWEDevice& device, uint16_t boneCount, uint16_t maxActorCount, std::shared_ptr<EWEDescriptorPool> globalPool);
		void writeData(glm::mat4* modelMatrix, void* finalBoneMatrices);
		void changeMaxActorCount(EWEDevice& device, uint16_t actorCount, std::shared_ptr<EWEDescriptorPool> globalPool);
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
			std::unique_ptr<EWEBuffer> model{};
			std::unique_ptr<EWEBuffer> bone{};
			VkDescriptorSet descriptor{};

			InnerBufferStruct(EWEDevice& device, uint16_t maxActorCount, uint32_t boneBlockSize, std::shared_ptr<EWEDescriptorPool> globalPool);

			void changeActorCount(EWEDevice& device, uint16_t maxActorCount, uint32_t boneBlockSize, std::shared_ptr<EWEDescriptorPool> globalPool);
			void buildDescriptor(uint16_t maxActorCount, std::shared_ptr<EWEDescriptorPool> globalPool);

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
			std::unique_ptr<EWEBuffer> bone;
			VkDescriptorSet descriptor;
			uint16_t currentActorCount = 0;

			InnerBufferStruct(EWEDevice& device, uint8_t maxActorCount, uint32_t boneBlockSize, std::shared_ptr<EWEDescriptorPool> globalPool);

			void changeActorCount(EWEDevice& device, uint8_t maxActorCount, uint32_t boneBlockSize, std::shared_ptr<EWEDescriptorPool> globalPool);
			void buildDescriptor(std::shared_ptr<EWEDescriptorPool> globalPool);

			void flush() {
				if (updated) {
					bone->flush();
					updated = false;
				}
			}
		private:
			bool updated = false;
		};

	public:

		SkinBufferHandler(EWEDevice& device, uint16_t boneCount, uint8_t maxActorCount, std::shared_ptr<EWEDescriptorPool> globalPool);
		SkinBufferHandler(uint8_t maxActorCount, std::vector<InnerBufferStruct>* referencedData);

		void writeData(void* finalBoneMatrices);
		void changeMaxActorCount(EWEDevice& device, uint8_t actorCount, std::shared_ptr<EWEDescriptorPool> globalPool);
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