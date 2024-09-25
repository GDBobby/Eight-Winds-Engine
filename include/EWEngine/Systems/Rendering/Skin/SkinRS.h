#pragma once

#include "SkinBufferHandler.h"

#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Graphics/Pipeline.h"
#include "EWEngine/Graphics/Texture/Material_Textures.h"
#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

#include "EWEngine/Systems/Rendering/Skin/SupportingStructs.h"

#include <algorithm>
#include <unordered_map>


namespace EWE {
	class SkinRenderSystem {
	private:
		static SkinRenderSystem* skinnedMainObject;
		
	public:

		SkinRenderSystem();
		~SkinRenderSystem();
		//~MonsterBoneBufferDescriptorStruct();

		//void addActorToBuffer(glm::mat4* modelMatrix, void* finalBoneMatrices, uint32_t skeletonID);
		void updateBuffers(uint8_t frameIndex);

		void flushBuffers(uint8_t frameIndex);

		void render(FrameInfo frameInfo);
	protected:
		void renderInstanced(VkCommandBuffer cmdBuf, uint8_t frameIndex);
		void renderNonInstanced(VkCommandBuffer cmdBuf, uint8_t frameIndex);
	public:

		static SkeletonID getSkinID() {
			return skinnedMainObject->skinID++;
		}

		static void addSkeleton(MaterialTextureInfo& materialInfo, uint16_t boneCount, EWEModel* modelPtr, SkeletonID skeletonID, bool instanced);
		static void addSkeletonToStructs(std::unordered_map<SkeletonID, std::vector<SkinRS::TextureMeshStruct>>& skeleRef, TextureDesc texID, EWEModel* modelPtr, SkeletonID skeletonID);

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

		static SkinBufferHandler* getSkinBuffer(SkeletonID skeletonID);
		InstancedSkinBufferHandler* getInstancedSkinBuffer(SkeletonID skeletonID);

		std::unordered_map<SkeletonID, SkinRS::PipelineStruct> instancedData{};
		std::unordered_map<MaterialFlags, SkinRS::PipelineStruct> boneData{};
		//uint8_t frameIndex = 0;

		//changes memory size allocated to buffers
		void changeActorCount(SkeletonID skeletonID, uint8_t maxActorCount);

		static void setPushData(SkeletonID skeletonID, void* pushData, uint8_t pushSize);
		static void removePushData(SkeletonID skeletonID, void* pushRemoval);

	private:

		void createInstancedBuffer(SkeletonID skeletonID, uint16_t boneCount) {
#if EWE_DEBUG
			assert(!instancedBuffers.contains(skeletonID));
#endif
			//instancedBuffersCreated += 2;
			instancedBuffers.emplace(skeletonID, InstancedSkinBufferHandler{ boneCount, 2000});
		}
		void createBoneBuffer(SkeletonID skeletonID, uint16_t boneCount) {
#if EWE_DEBUG
			assert(!buffers.contains(skeletonID));
#endif
			//buffersCreated += 2;
			buffers.emplace(skeletonID, SkinBufferHandler{ boneCount, 1});
		}
		void createReferenceBuffer(SkeletonID skeletonID, SkeletonID referenceID) {
			if (buffers.contains(skeletonID)) {
				return;
				//printf("creating a buffer that already exist \n");
				//throw std::runtime_error("creating a buffer that already exist ");
			}
			buffers.emplace(skeletonID, SkinBufferHandler{ 1, buffers.at(referenceID).getInnerPtr() });
		}

		SkinRS::PipelineStruct& createInstancedPipe(SkeletonID instancedFlags, uint16_t boneCount, MaterialFlags textureFlags) {
			return instancedData.try_emplace(instancedFlags, boneCount, textureFlags).first->second;
		}
		SkinRS::PipelineStruct& createBonePipe(MaterialFlags boneFlags) {
			return boneData.try_emplace(boneFlags, boneFlags).first->second;
		}

		uint32_t skinID = 0;

		//key is skeletonID
		std::unordered_map<SkeletonID, SkinBufferHandler> buffers{};
		std::unordered_map<SkeletonID, InstancedSkinBufferHandler> instancedBuffers{};
		std::unordered_map<SkeletonID, SkinRS::PushConstantStruct> pushConstants{};

		//uint32_t buffersCreated = 0;
		//uint32_t instancedBuffersCreated = 0;
	};
}

