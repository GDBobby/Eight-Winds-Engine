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

		void UpdateBuffers();
		void FlushBuffers();

		void Render();
	protected:
		void RenderInstanced();
		void RenderNonInstanced();
	public:

		static SkeletonID GetSkinID() {
			return skinnedMainObject->skinID++;
		}

		static void AddSkeleton(MaterialInfo& materialInfo, uint16_t boneCount, EWEModel* modelPtr, SkeletonID skeletonID, bool instanced);
		static void AddSkeletonToStructs(std::unordered_map<SkeletonID, std::vector<SkinRS::TextureMeshStruct>>& skeleRef, MaterialInfo const& materialInfo, EWEModel* modelPtr, SkeletonID skeletonID);
		static void AddSkeletonToStructs(std::unordered_map<SkeletonID, std::vector<SkinRS::TextureMeshStruct>>& skeleRef, MaterialInfo const& materialInfo, EWEModel* modelPtr, SkeletonID weaponID, SkeletonID skeletonID);

		static void AddWeapon(MaterialInfo& materialInfo, EWEModel* meshes, SkeletonID skeletonID, SkeletonID ownerID);

		static void RemoveSkeleton(SkeletonID skeletonID);

		static SkinBufferHandler* GetSkinBuffer(SkeletonID skeletonID);
		InstancedSkinBufferHandler* GetInstancedSkinBuffer(SkeletonID skeletonID);

		std::unordered_map<SkeletonID, SkinRS::PipelineStruct> instancedData{};
		std::unordered_map<MaterialFlags, SkinRS::PipelineStruct> boneData{};


		static void SetPushData(SkeletonID skeletonID, void* pushData, uint8_t pushSize);
		static void RemovePushData(SkeletonID skeletonID, void* pushRemoval);

	private:

		[[nodiscard]] InstancedSkinBufferHandler* CreateInstancedBuffer(SkeletonID skeletonID, uint16_t boneCount) {
#if EWE_DEBUG
			assert(!instancedBuffers.contains(skeletonID));
#endif
			//instancedBuffersCreated += 2;
			return &instancedBuffers.try_emplace(skeletonID, boneCount, 2000).first->second;
		}
		[[nodiscard]] SkinBufferHandler* CreateBoneBuffer(SkeletonID skeletonID, uint16_t boneCount) {
#if EWE_DEBUG
			assert(!buffers.contains(skeletonID));
#endif
			//buffersCreated += 2;
			return &buffers.try_emplace(skeletonID, boneCount, 2).first->second; //maxactorcount should probably be either 1 or some small number under 5 idk. too lazy to set up instancing for 2 objects so im setting it to 2
		}

		static std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> CreateDescriptorSets(MaterialInfo materialInfo, SkeletonID skeletonID);
		//void CreateReferenceBuffer(SkeletonID skeletonID, SkeletonID referenceID) {
		//	if (buffers.contains(skeletonID)) {
		//		return;
		//		//printf("creating a buffer that already exist \n");
		//		//throw std::runtime_error("creating a buffer that already exist ");
		//	}
		//	buffers.emplace(skeletonID, SkinBufferHandler{ 1, buffers.at(referenceID).GetInnerPtr() });
		//}

		SkinRS::PipelineStruct& CreateInstancedPipe(SkeletonID instancedFlags, uint16_t boneCount, MaterialFlags textureFlags) {
			return instancedData.try_emplace(instancedFlags, boneCount, textureFlags).first->second;
		}
		SkinRS::PipelineStruct& CreateBonePipe(MaterialFlags boneFlags) {
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

