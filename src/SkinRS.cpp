#include "EWEngine/Systems/Rendering/Skin/SkinRS.h"
#include "EWEngine/Graphics/Texture/Image_Manager.h"
#include "EWEngine/Graphics/Texture/Material_Textures.h"

#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

#include <cassert>

namespace EWE {
	SkinRenderSystem* SkinRenderSystem::skinnedMainObject = nullptr;

	template<typename BufferHandler>
	std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> CreateDescriptorSetsHelper(MaterialInfo materialInfo, BufferHandler* bufferHandler) {
		EWEDescriptorSetLayout* eDSL = MaterialPipelines::GetDSLFromFlags(materialInfo.materialFlags);
		std::array < VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descSets;
		for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			EWEDescriptorWriter descWriter{ eDSL, DescriptorPool_Global };
			DescriptorHandler::AddGlobalsToDescriptor(descWriter, i);
			bufferHandler->AddDescriptorBindings(i, descWriter);
			descWriter.WriteImage(materialInfo.imageID);
			descSets[i] = descWriter.Build();
		}
		return descSets;
	}

	std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> SkinRenderSystem::CreateDescriptorSets(MaterialInfo materialInfo, SkeletonID skeletonID) {

		if (materialInfo.materialFlags & MaterialF_instanced) {
			return CreateDescriptorSetsHelper(materialInfo, &skinnedMainObject->instancedBuffers.at(skeletonID));
		}
		else {
			return CreateDescriptorSetsHelper(materialInfo, &skinnedMainObject->buffers.at(skeletonID));
		}
	}


	SkinRenderSystem::SkinRenderSystem() {
		skinnedMainObject = this;
		
		MaterialPipelines::InitStaticVariables();
	
	}
	SkinRenderSystem::~SkinRenderSystem() {
#if DECONSTRUCTION_DEBUG
		printf("before clearing pipes \n");

		printf("before clearing skin buffer descriptors");// , amount created - % d: % d \n", buffersCreated, instancedBuffersCreated);
		uint16_t bufferDescriptorsCleared = 0;
		uint16_t instancedBuffersCleared = 0;
#endif

		printf("DO NOT REMOVE THIS RBEAKPOINT WITHOUT FIXING THE ISSUE, OR RE-ENABLING THE ASSERT\n");
		//assert(false); //this was moved to the TextureMeshStruct
		//for (auto& buffer : buffers) {
		//	bufferDescriptorsCleared++;
		//}
		//for (auto& instanceBuffer : instancedBuffers) {
		//	instanceBuffer.second.FreeDescriptors();
		//	instancedBuffersCleared++;
		//}
#if DECONSTRUCTION_DEBUG
		printf("after clearing buffer descriptors - count - %d:%d  \n", bufferDescriptorsCleared, instancedBuffersCleared);
#endif
		skinnedMainObject = nullptr;
	}

	SkinBufferHandler* SkinRenderSystem::GetSkinBuffer(SkeletonID skeletonID) {
#if EWE_DEBUG
		assert(skinnedMainObject->buffers.contains(skeletonID) && "buffer does not exist");
#endif
		return &skinnedMainObject->buffers.at(skeletonID);
	}

	InstancedSkinBufferHandler* SkinRenderSystem::GetInstancedSkinBuffer(SkeletonID skeletonID) {
#if EWE_DEBUG
			assert(instancedBuffers.contains(skeletonID) && "requested buffer doesn't exist");
#endif
			return &instancedBuffers.at(skeletonID);
	}

	void SkinRenderSystem::SetPushData(SkeletonID skeletonID, void* pushData, uint8_t pushSize) {
			auto pushIterData = skinnedMainObject->pushConstants.find(skeletonID);
			if (pushIterData == skinnedMainObject->pushConstants.end()) {
				skinnedMainObject->pushConstants.emplace(skeletonID, SkinRS::PushConstantStruct{ pushData, pushSize });
				//pushConstants[skeletonID] = { pushData, pushSize };
			}
			else {
				pushIterData->second.AddData(pushData, pushSize);
			}
		}
	void SkinRenderSystem::RemovePushData(SkeletonID skeletonID, void* pushRemoval) {
		auto pushIterData = skinnedMainObject->pushConstants.find(skeletonID);
		if (pushIterData == skinnedMainObject->pushConstants.end()) {
			assert(false && "invalid push to remove\n");
		}
		else {
			pushIterData->second.Remove(pushRemoval);
		}
	}

	void SkinRenderSystem::RenderInstanced() {

		MaterialPipelines* pipe;

		for (auto& instanced : instancedData) {
			//if instanced.first.actorCount == 0 return;
			pipe = instanced.second.pipeline;

			pipe->BindPipeline();

			//pipe->BindDescriptor(0, DescriptorHandler::GetDescSet(DS_global));

			int64_t bindedSkeletonID = -1;

			for (auto& skeleDataRef : instanced.second.skeletonData) {
				//printf("instance count? - %d:%d \n", skeleDataRef.first, instancedBuffers.at(skeleDataRef.first).getInstanceCount());

#if EWE_DEBUG
				assert(instancedBuffers.contains(skeleDataRef.first) && "requested buffer doesn't exist");
#endif

				if (instancedBuffers.at(skeleDataRef.first).GetInstanceCount() <= 0) {
					continue;
				}

				if (bindedSkeletonID != skeleDataRef.first) {
					bindedSkeletonID = skeleDataRef.first;
					//pipe->BindDescriptor(1, instancedBuffers.at(skeleDataRef.first).GetDescriptor());
				}
				for (auto& skeleTextureRef : skeleDataRef.second) {
					//pipe->BindTextureDescriptor(2, skeleTextureRef.texture);
					pipe->BindDescriptor(0, &skeleTextureRef.descriptorSets[VK::Object->frameIndex]);

					for (auto& meshRef : skeleTextureRef.meshes) {
						meshRef->BindAndDrawInstanceNoBuffer(instancedBuffers.at(skeleDataRef.first).GetInstanceCount());
					}
				}

			}
		}
		//printf("after indexed drawing \n");
		for (auto& instancedBuffer : instancedBuffers) {
			instancedBuffer.second.ResetInstanceCount();
		}

	}
	void SkinRenderSystem::RenderNonInstanced() {

		MaterialPipelines* pipe;
		for (auto& boned : boneData) {
			pipe = boned.second.pipeline;
			//printf("shader flags on non-instanced : %d \n", boned.first);
			pipe->BindPipeline();

			//pipe->BindDescriptor(0, DescriptorHandler::GetDescSet(DS_global));

			for (auto& skeleDataRef : boned.second.skeletonData) {
				if (!pushConstants.contains(skeleDataRef.first)) {
//#if EWE_DEBUG
					//std::cout << "this skeleton doesn't have push constants - skeletonID : " << skeleDataRef.first << std::endl;
					//std::cout << "push count is 0 : " << std::endl;
//#endif
					continue;
				}

				auto& skelePush = pushConstants.at(skeleDataRef.first);
				if (skelePush.count == 0) {
//#if EWE_DEBUG
					//std::cout << "push count is 0 : " << std::endl;
//#endif
					continue;
				}

#if EWE_DEBUG
				//assert(buffers.contains(skeleDataRef.first) && "buffer does not exist");
#endif
				//pipe->BindDescriptor(1, buffers.at(skeleDataRef.first).GetDescriptor());

				for (auto& skeleTextureRef : skeleDataRef.second) {
					//pipe->BindTextureDescriptor(2, skeleTextureRef.texture);
					pipe->BindDescriptor(0, &skeleTextureRef.descriptorSets[VK::Object->frameIndex]);

					//race condition here for deletion of push constant
					for (auto& meshRef : skeleTextureRef.meshes) {
						//printf("for each mesh in non-instanced \n");
						pipe->BindModel(meshRef);
#if RENDER_DEBUG
						printf("before drawing to noninstanced skin pipeline : %d \n", boned.first);
#endif
						for (auto& pushData : skelePush.data) {
							pipe->PushAndDraw(pushData);
						}
					}
				}
			}
		}
	}

	void SkinRenderSystem::Render() {
		//printf("skin render? \n");
		/*
		if (enemyData->actorCount[actorType - 3] == 0) {
			return;
		}
		*/
		//^ figure out how to contain new actors to replace that line
		//setFrameIndex(frameInfo.index);

		//printf("pre instance drawing in skinned RS \n");
		RenderInstanced();

		RenderNonInstanced();
		

		//printf("after skinned RS drawing \n");
	}

	void SkinRenderSystem::UpdateBuffers() {
		printf("am i using update buffers? i dont think i should be \n");
		FlushBuffers();
	}

	void SkinRenderSystem::FlushBuffers() {
		for (auto& buffer : buffers) {
			buffer.second.Flush();
		}

		/*
		if (currentMemOffset[0] > 0) {
			//if (currentMemOffset[0] >= 65536) {
			//	printf("too much memory to a uniform buffer \n");
			//	throw std::exception("writing too much memory to a uniform buffer");
			//}
			buffers[frameInfo.index][0]->flush();
		}
		if (currentMemOffset[1] > 0) {
			buffers[frameInfo.index][1]->flush();
		}
		//buffers[frameInfo.index][2]->flush();


		currentMemOffset[0] = 0;
		currentMemOffset[1] = 0;
		//currentMemOffset[2] = 0;
		//currentBoneOffset = 0;
		*/

	}
	void SkinRenderSystem::AddSkeletonToStructs(std::unordered_map<SkeletonID, std::vector<SkinRS::TextureMeshStruct>>& skeleRef, MaterialInfo const& materialInfo, EWEModel* modelPtr, SkeletonID skeletonID) {

		auto textureMeshStructPair = skeleRef.find(skeletonID);

		if (textureMeshStructPair != skeleRef.end()) {
			bool foundATextureMatch = false;
			for (auto& textureRef : textureMeshStructPair->second) {
				if (textureRef.imageID == materialInfo.imageID) {
					foundATextureMatch = true;
					textureRef.meshes.push_back(modelPtr);
					break;
				}
			}
			if (!foundATextureMatch) {
				std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descSets = CreateDescriptorSets(materialInfo, skeletonID);

				textureMeshStructPair->second.emplace_back(descSets, std::vector<EWEModel*>{modelPtr}, materialInfo.imageID);
			}
		}
		else {
			auto emplaceRet = skeleRef.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{});
			std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descSets = CreateDescriptorSets(materialInfo, skeletonID);
			emplaceRet.first->second.emplace_back(descSets, std::vector<EWEModel*>{modelPtr}, materialInfo.imageID);
		}
	}
	void SkinRenderSystem::AddSkeletonToStructs(std::unordered_map<SkeletonID, std::vector<SkinRS::TextureMeshStruct>>& skeleRef, MaterialInfo const& materialInfo, EWEModel* modelPtr, SkeletonID weaponID, SkeletonID skeletonID) {

		auto textureMeshStructPair = skeleRef.find(weaponID);

		if (textureMeshStructPair != skeleRef.end()) {
			bool foundATextureMatch = false;
			for (auto& textureRef : textureMeshStructPair->second) {
				if (textureRef.imageID == materialInfo.imageID) {
					foundATextureMatch = true;
					textureRef.meshes.push_back(modelPtr);
					break;
				}
			}
			if (!foundATextureMatch) {
				std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descSets = CreateDescriptorSets(materialInfo, skeletonID);

				textureMeshStructPair->second.emplace_back(descSets, std::vector<EWEModel*>{modelPtr}, materialInfo.imageID);
			}
		}
		else {
			auto emplaceRet = skeleRef.emplace(weaponID, std::vector<SkinRS::TextureMeshStruct>{});
			std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descSets = CreateDescriptorSets(materialInfo, skeletonID);
			emplaceRet.first->second.emplace_back(descSets, std::vector<EWEModel*>{modelPtr}, materialInfo.imageID);
		}
	}

	void SkinRenderSystem::AddSkeleton(MaterialInfo& materialInfo, uint16_t boneCount, EWEModel* modelPtr, SkeletonID skeletonID, bool instanced) {
#if EWE_DEBUG
		assert(skinnedMainObject != nullptr);
		printf("adding skeleton \n");
#endif

		materialInfo.materialFlags |= MaterialF_hasBones;

		if (instanced) {
			materialInfo.materialFlags |= MaterialF_instanced;

			uint32_t instancedFlags = (boneCount << 16) + materialInfo.materialFlags;

			auto instancedDataIter = skinnedMainObject->instancedData.find(instancedFlags);

			InstancedSkinBufferHandler* bufferHandler = skinnedMainObject->CreateInstancedBuffer(skeletonID, boneCount);

			if (instancedDataIter == skinnedMainObject->instancedData.end()) {
				SkinRS::PipelineStruct& instancedPipe = skinnedMainObject->CreateInstancedPipe(instancedFlags, boneCount, materialInfo.materialFlags);

				std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descSets = CreateDescriptorSetsHelper(materialInfo, bufferHandler);

				instancedPipe.skeletonData.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{SkinRS::TextureMeshStruct{ descSets, std::vector<EWEModel*>{modelPtr}, materialInfo.imageID }});
			}
			else {
				AddSkeletonToStructs(instancedDataIter->second.skeletonData, materialInfo, modelPtr, skeletonID);
			}
		}
		else {
			auto boneDataIter = skinnedMainObject->boneData.find(materialInfo.materialFlags);

			SkinBufferHandler* bufferHandler = skinnedMainObject->CreateBoneBuffer(skeletonID, boneCount);
			if (boneDataIter == skinnedMainObject->boneData.end()) {

				SkinRS::PipelineStruct& bonePipe = skinnedMainObject->CreateBonePipe(materialInfo.materialFlags);
				std::array < VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descSets = CreateDescriptorSetsHelper(materialInfo, bufferHandler);

				bonePipe.skeletonData.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{SkinRS::TextureMeshStruct{ descSets, std::vector<EWEModel*>{modelPtr}, materialInfo.imageID }});
			}
			else {
				AddSkeletonToStructs(boneDataIter->second.skeletonData, materialInfo, modelPtr, skeletonID);
			}
		}
	}
	void SkinRenderSystem::AddWeapon(MaterialInfo& materialInfo, EWEModel* modelPtr, SkeletonID weaponID, SkeletonID ownerID) {
		//need this to reference the owner buffer
		//just create a descriptor with the owner buffer

		auto boneDataIter = skinnedMainObject->boneData.find(materialInfo.materialFlags);

		if (boneDataIter == skinnedMainObject->boneData.end()) {

			SkinRS::PipelineStruct& retPipe = skinnedMainObject->CreateBonePipe(materialInfo.materialFlags);
			auto emplaceRet = retPipe.skeletonData.emplace(weaponID, std::vector<SkinRS::TextureMeshStruct>{});

			std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descSets = CreateDescriptorSets(materialInfo, ownerID);

			emplaceRet.first->second.emplace_back(descSets, std::vector<EWEModel*>{modelPtr}, materialInfo.imageID);
			//secondRef.meshes.push_back(modelPtr);
		}
		else {
			//assert(false && "I need to figure out how to handle this, avoid currently");
			AddSkeletonToStructs(boneDataIter->second.skeletonData, materialInfo, modelPtr, weaponID, ownerID);
		}
	}

	void SkinRenderSystem::RemoveSkeleton(SkeletonID skeletonID) {
		for (auto& instanced : skinnedMainObject->instancedData) {
			//auto skeletonDataIter = instanced.second.skeletonData.find(skeletonID);
			//if (skeletonDataIter != instanced.second.skeletonData.end()) {
			//	for (auto& textureRef : skeletonDataIter->second){
			//		//skinnedMainObject->cleanTheseTextures.push_back(textureRef.textureID);
			//	}
			//	//texture erasure here, if i decide to do it that way
			//}
			if (instanced.second.skeletonData.erase(skeletonID) > 0) {
#if EWE_DEBUG
				printf("erasing an instanced skeleton : %d \n", skeletonID);
#endif
				if (instanced.second.skeletonData.size() == 0) {
#if EWE_DEBUG
					printf("erasing a instanced skin pipeline :%d \n", instanced.first);
#endif

					skinnedMainObject->instancedData.erase(instanced.first);
					//need to remove textures, 
					//or have textures remove themself, probably the better option
					//currently, texture data will remain forever

					//that comment was written when i first wrote this, in maybe oct 2023. in feb 2024 im not sure if this is still an issue. worth chekcing into
					//i believe im manually erasing textures before removing a skeleton. not sure tho
				}
			}
#if EWE_DEBUG
			else {
				printf("attempting to remove a skeleton that does not exist \n");
			}
#endif
		}
		for (auto& boned : skinnedMainObject->boneData) {
			//if (instanced.second.skeletonData.find(skeletonID) != instanced.second.skeletonData.end()) {
			//	auto& skeleDataRef = instanced.second.skeletonData.at(skeletonID);
			//	for(auto iter = )
			// texture erasure here, if i decide to do it that way
			//}
			if (boned.second.skeletonData.erase(skeletonID) > 0) {
				printf("erasing a boned skeleton : %d \n", skeletonID);
				if (boned.second.skeletonData.size() == 0) {
#if EWE_DEBUG
					printf("erasing a boned skin pipeline :%d \n", boned.first);
#endif
					skinnedMainObject->boneData.erase(boned.first);
					//need to remove textures, 
					//or have textures remove themself, probably the better option
					//currently, texture data will remain forever
				}
			}
		}
	}
}