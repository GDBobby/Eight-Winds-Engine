#pragma once

#include "SkinBufferHandler.h"

#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Graphics/Textures/Material_Textures.h"
#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

#include "EWEngine/Systems/Rendering/Skin/SupportingStructs.h"

#include <algorithm>


namespace EWE {
	class SkinRenderSystem {
	private:
		static SkinRenderSystem* skinnedMainObject;

	public:

		SkinRenderSystem(EWEDevice& device);
		~SkinRenderSystem();
		//~MonsterBoneBufferDescriptorStruct();

		//void addActorToBuffer(glm::mat4* modelMatrix, void* finalBoneMatrices, uint32_t skeletonID);
		void updateBuffers(uint8_t frameIndex);

		void flushBuffers(uint8_t frameIndex);

		void render(std::pair<VkCommandBuffer, uint8_t> cmdIndexPair);
	protected:
		void renderInstanced(VkCommandBuffer cmdBuf, uint8_t frameIndex);
		void renderNonInstanced(VkCommandBuffer cmdBuf, uint8_t frameIndex);
	public:

		static SkeletonID getSkinID() {
			return skinnedMainObject->skinID++;
		}

		static void addSkeleton(MaterialTextureInfo& materialInfo, uint16_t boneCount, EWEModel* modelPtr, SkeletonID skeletonID, bool instanced);
		static void addWeapon(MaterialTextureInfo& materialInfo, EWEModel* meshes, SkeletonID skeletonID, SkeletonID ownerID);

		static void removeSkeleton(SkeletonID skeletonID);

		//put this pointer in to actor classes, matching the skeleton id, only use writedata
		void setFrameIndex(uint8_t frameIndex) {
			for (auto& buffer : buffers) {
				buffer.second.setFrameIndex(frameIndex);
			}
			for (auto& instanceBuffer : instancedBuffers) {
				instanceBuffer.second.setFrameIndex(frameIndex);
			}
		}

		static SkinBufferHandler* getSkinBuffer(SkeletonID skeletonID) {
#ifdef _DEBUG
			if (skinnedMainObject->buffers.find(skeletonID) == skinnedMainObject->buffers.end()) {
				printf("trying to get a pointer to a skin buffer that doesn't exist : %d \n", skeletonID);
				//most likely cause is its in the instanced buffer
				throw std::exception("trying to get a pointer to a skin buffer that doesn't exist");
			}
#endif
			return &skinnedMainObject->buffers.at(skeletonID);
		}
		InstancedSkinBufferHandler* getInstancedSkinBuffer(SkeletonID skeletonID) {
#ifdef _DEBUG
			if (instancedBuffers.find(skeletonID) == instancedBuffers.end()) {
				printf("trying to get a pointer to an instanced skin buffer that doesn't exist : %d \n", skeletonID);
				//most likely cause is its in the non-instanced buffer map
				throw std::exception("trying to get a pointer to an instanced skin buffer that doesn't exist");
			}
#endif
			return &instancedBuffers.at(skeletonID);
		}

		std::unordered_map<SkeletonID, SkinRS::PipelineStruct> instancedData{};
		std::unordered_map<MaterialFlags, SkinRS::PipelineStruct> boneData{};
		//uint8_t frameIndex = 0;

		//changes memory size allocated to buffers
		void changeActorCount(SkeletonID skeletonID, uint8_t maxActorCount) {
#if _DEBUG
			if (buffers.find(skeletonID) == buffers.end()) {
				printf("trying to change the max actor count for a buffer that doesn't exist \n");
				throw std::exception("trying to change the max actor count for a buffer that doesn't exist");
			}
#endif
			buffers.at(skeletonID).changeMaxActorCount(device, maxActorCount);

		}

		static void setPushData(SkeletonID skeletonID, void* pushData, uint8_t pushSize) {
			if (skinnedMainObject->pushConstants.find(skeletonID) == skinnedMainObject->pushConstants.end()) {
				skinnedMainObject->pushConstants.emplace(skeletonID, SkinRS::PushConstantStruct{ pushData, pushSize });
				//pushConstants[skeletonID] = { pushData, pushSize };
			}
			else {
				
				skinnedMainObject->pushConstants.at(skeletonID).addData(pushData, pushSize);
			}
		}
		static void removePushData(SkeletonID skeletonID, void* pushRemoval) {
			if (skinnedMainObject->pushConstants.find(skeletonID) == skinnedMainObject->pushConstants.end()) {
				std::cout << "invalid push to remove \n";
				throw std::runtime_error("invalid push to remove");
			}
			else {
				skinnedMainObject->pushConstants.at(skeletonID).remove(pushRemoval);
			}
		}

	private:

		void createInstancedBuffer(SkeletonID skeletonID, uint16_t boneCount) {
#ifdef _DEBUG
			if (instancedBuffers.find(skeletonID) != instancedBuffers.end()) {
				return;
				printf("creating a buffer that already exist \n");
				throw std::exception("creating a buffer that already exist ");
			}
#endif
			//instancedBuffersCreated += 2;
			instancedBuffers.emplace(skeletonID, InstancedSkinBufferHandler{ device, boneCount, 2000});
		}
		void createBoneBuffer(SkeletonID skeletonID, uint16_t boneCount) {
			if (buffers.find(skeletonID) != buffers.end()) {
				return;
				printf("creating a buffer that already exist \n");
				throw std::runtime_error("creating a buffer that already exist ");
			}
			//buffersCreated += 2;
			printf("creating bone buffer \n");
			buffers.emplace(skeletonID, SkinBufferHandler{ device, boneCount, 1});
		}
		void createReferenceBuffer(SkeletonID skeletonID, SkeletonID referenceID) {
			if (buffers.find(skeletonID) != buffers.end()) {
				return;
				printf("creating a buffer that already exist \n");
				throw std::runtime_error("creating a buffer that already exist ");
			}
			buffers.emplace(skeletonID, SkinBufferHandler{ 1, buffers.at(referenceID).getInnerPtr() });
		}

		void createInstancedPipe(SkeletonID instancedFlags, uint16_t boneCount, MaterialFlags textureFlags) {
			instancedData.emplace(instancedFlags,
				SkinRS::PipelineStruct{ boneCount, textureFlags, device }
			);
		}
		void createBonePipe(MaterialFlags boneFlags) {
			boneData.emplace(boneFlags, SkinRS::PipelineStruct{ boneFlags, device });
		}

		uint32_t skinID = 0;

		//key is skeletonID
		std::unordered_map<SkeletonID, SkinBufferHandler> buffers{};
		std::unordered_map<SkeletonID, InstancedSkinBufferHandler> instancedBuffers{};
		std::unordered_map<SkeletonID, SkinRS::PushConstantStruct> pushConstants{};

		EWEDevice& device;

		//uint32_t buffersCreated = 0;
		//uint32_t instancedBuffersCreated = 0;
	};
}

