#include "EWEngine/Systems/Rendering/Skin/SkinRS.h"
#include "EWEngine/Graphics/Texture/Texture_Manager.h"
#include "EWEngine/Graphics/Texture/Material_Textures.h"

#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

#include <cassert>

namespace EWE {
	SkinRenderSystem* SkinRenderSystem::skinnedMainObject = nullptr;


	SkinRenderSystem::SkinRenderSystem() {
		skinnedMainObject = this;

		//this initializes the descriptors
		DescriptorHandler::getDescSetLayout(LDSL_boned);
		DescriptorHandler::getDescSetLayout(LDSL_largeInstance);
		
		MaterialPipelines::initStaticVariables();
	
	}
	SkinRenderSystem::~SkinRenderSystem() {
		printf("before clearing pipes \n");
		MaterialPipelines::cleanupStaticVariables();

		printf("before clearing skin buffer descriptors");// , amount created - % d: % d \n", buffersCreated, instancedBuffersCreated);
		uint16_t bufferDescriptorsCleared = 0;
		uint16_t instancedBuffersCleared = 0;
		for (auto& buffer : buffers) {
			if (!buffer.second.CheckReference()) {
				for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					buffer.second.setFrameIndex(i);
					EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, buffer.second.getDescriptor());
					bufferDescriptorsCleared++;
				}
			}
		}
		for (auto& instanceBuffer : instancedBuffers) {
			for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				instanceBuffer.second.setFrameIndex(i);
				EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, instanceBuffer.second.getDescriptor());
				instancedBuffersCleared++;
			}
		}
		printf("after clearing buffer descriptors - count - %d:%d  \n", bufferDescriptorsCleared, instancedBuffersCleared);
		skinnedMainObject = nullptr;
	}

	SkinBufferHandler* SkinRenderSystem::getSkinBuffer(SkeletonID skeletonID) {
#if EWE_DEBUG
		assert(skinnedMainObject->buffers.contains(skeletonID) && "buffer does not exist");
#endif
		return &skinnedMainObject->buffers.at(skeletonID);
	}

	InstancedSkinBufferHandler* SkinRenderSystem::getInstancedSkinBuffer(SkeletonID skeletonID) {
#if EWE_DEBUG
			assert(instancedBuffers.contains(skeletonID) && "requested buffer doesn't exist");
#endif
			return &instancedBuffers.at(skeletonID);
	}

	void SkinRenderSystem::changeActorCount(SkeletonID skeletonID, uint8_t maxActorCount) {
#if EWE_DEBUG
			assert(buffers.contains(skeletonID) && "trying to change the max actor count for a buffer that doesn't exist");
#endif
			buffers.at(skeletonID).changeMaxActorCount(maxActorCount);

	}
	void SkinRenderSystem::setPushData(SkeletonID skeletonID, void* pushData, uint8_t pushSize) {
			auto pushIterData = skinnedMainObject->pushConstants.find(skeletonID);
			if (pushIterData == skinnedMainObject->pushConstants.end()) {
				skinnedMainObject->pushConstants.emplace(skeletonID, SkinRS::PushConstantStruct{ pushData, pushSize });
				//pushConstants[skeletonID] = { pushData, pushSize };
			}
			else {
				pushIterData->second.addData(pushData, pushSize);
			}
		}
	void SkinRenderSystem::removePushData(SkeletonID skeletonID, void* pushRemoval) {
		auto pushIterData = skinnedMainObject->pushConstants.find(skeletonID);
		if (pushIterData == skinnedMainObject->pushConstants.end()) {
			std::cout << "invalid push to remove \n";
			throw std::runtime_error("invalid push to remove");
		}
		else {
			pushIterData->second.remove(pushRemoval);
		}
	}

	void SkinRenderSystem::renderInstanced(VkCommandBuffer cmdBuf, uint8_t frameIndex) {

		MaterialPipelines* pipe;

		for (auto& instanced : instancedData) {
			//if instanced.first.actorCount == 0 return;
			pipe = instanced.second.pipeline;

			pipe->bindPipeline();

			pipe->bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameIndex));

			int64_t bindedSkeletonID = -1;

			for (auto& skeleDataRef : instanced.second.skeletonData) {
				//printf("instance count? - %d:%d \n", skeleDataRef.first, instancedBuffers.at(skeleDataRef.first).getInstanceCount());

#if EWE_DEBUG
				assert(instancedBuffers.contains(skeleDataRef.first) && "requested buffer doesn't exist");
#endif

				if (instancedBuffers.at(skeleDataRef.first).getInstanceCount() <= 0) {
					continue;
				}

				if (bindedSkeletonID != skeleDataRef.first) {
					bindedSkeletonID = skeleDataRef.first;
					pipe->bindDescriptor(1, instancedBuffers.at(skeleDataRef.first).getDescriptor());
				}
				for (auto& skeleTextureRef : skeleDataRef.second) {
					pipe->bindTextureDescriptor(2, skeleTextureRef.texture);

					for (auto& meshRef : skeleTextureRef.meshes) {
						//meshRef->BindAndDrawInstanceNoBuffer(frameInfo.cmdBuf, actorCount.at(instanced.first));
						//printf("drawing instanced : %d \n", instancedBuffers.at(bindedSkeletonID).getInstanceCount());
						meshRef->BindAndDrawInstanceNoBuffer(cmdBuf, instancedBuffers.at(skeleDataRef.first).getInstanceCount());
					}
				}

			}
		}
		//printf("after indexed drawing \n");
		for (auto& instancedBuffer : instancedBuffers) {
			instancedBuffer.second.resetInstanceCount();
		}

	}
	void SkinRenderSystem::renderNonInstanced(VkCommandBuffer cmdBuf, uint8_t frameIndex) {

		MaterialPipelines* pipe;
		for (auto& boned : boneData) {
			pipe = boned.second.pipeline;
			//printf("shader flags on non-instanced : %d \n", boned.first);
			pipe->bindPipeline();

			pipe->bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameIndex));

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
				assert(buffers.contains(skeleDataRef.first) && "buffer does not exist");
#endif
				pipe->bindDescriptor(1, buffers.at(skeleDataRef.first).getDescriptor());

				for (auto& skeleTextureRef : skeleDataRef.second) {
					pipe->bindTextureDescriptor(2, skeleTextureRef.texture);

					//race condition here for deletion of push constant
					for (auto& meshRef : skeleTextureRef.meshes) {
						//printf("for each mesh in non-instanced \n");
						//meshRef->BindAndDrawInstanceNoBuffer(frameInfo.cmdBuf, actorCount.at(instanced.first));
						pipe->bindModel(meshRef);
#if RENDER_DEBUG
						printf("before drawing to noninstanced skin pipeline : %d \n", boned.first);
#endif
						for (auto& pushData : skelePush.data) {
							pipe->pushAndDraw(pushData);
						}
					}
				}
			}
		}
	}

	void SkinRenderSystem::render(FrameInfo frameInfo) {
		//printf("skin render? \n");
		/*
		if (enemyData->actorCount[actorType - 3] == 0) {
			return;
		}
		*/
		//^ figure out how to contain new actors to replace that line
		//setFrameIndex(frameInfo.index);

		//printf("pre instance drawing in skinned RS \n");
		MaterialPipelines::setFrameInfo(frameInfo);
		renderInstanced(frameInfo.cmdBuf, frameInfo.index);

		renderNonInstanced(frameInfo.cmdBuf, frameInfo.index);
		

		//printf("after skinned RS drawing \n");
	}

	void SkinRenderSystem::updateBuffers(uint8_t frameIndex) {
		printf("am i using update buffers? i dont think i should be \n");
		flushBuffers(frameIndex);
	}

	void SkinRenderSystem::flushBuffers(uint8_t frameIndex) {
		for (auto& buffer : buffers) {
			buffer.second.flush();
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
	void SkinRenderSystem::addSkeletonToStructs(std::unordered_map<SkeletonID, std::vector<SkinRS::TextureMeshStruct>>& skeleRef, TextureDesc tex, EWEModel* modelPtr, SkeletonID skeletonID) {

		auto textureMeshStructPair = skeleRef.find(skeletonID);

		if (textureMeshStructPair != skeleRef.end()) {
			bool foundATextureMatch = false;
			for (auto& textureRef : textureMeshStructPair->second) {
				if (textureRef.texture == tex) {
					foundATextureMatch = true;
					textureRef.meshes.push_back(modelPtr);
					break;
				}
			}
			if (!foundATextureMatch) {
				textureMeshStructPair->second.emplace_back(tex, std::vector<EWEModel*>{modelPtr});
			}
		}
		else {
			auto emplaceRet = skeleRef.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{});
			emplaceRet.first->second.emplace_back(tex, std::vector<EWEModel*>{modelPtr});
		}
	}

	void SkinRenderSystem::addSkeleton(MaterialTextureInfo& materialInfo, uint16_t boneCount, EWEModel* modelPtr, SkeletonID skeletonID, bool instanced) {
#if EWE_DEBUG
		assert(skinnedMainObject != nullptr);
		printf("adding skeleton \n");
#endif

		materialInfo.materialFlags |= MaterialF_hasBones;

		if (instanced) {
			materialInfo.materialFlags |= MaterialF_instanced;

			uint32_t instancedFlags = (boneCount << 16) + materialInfo.materialFlags;

			auto instancedDataIter = skinnedMainObject->instancedData.find(instancedFlags);

			skinnedMainObject->createInstancedBuffer(skeletonID, boneCount);

			if (instancedDataIter == skinnedMainObject->instancedData.end()) {
				SkinRS::PipelineStruct& instancedPipe = skinnedMainObject->createInstancedPipe(instancedFlags, boneCount, materialInfo.materialFlags);
				instancedPipe.skeletonData.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{SkinRS::TextureMeshStruct{ materialInfo.texture, std::vector<EWEModel*>{modelPtr} }});
			}
			else {
				addSkeletonToStructs(instancedDataIter->second.skeletonData, materialInfo.texture, modelPtr, skeletonID);
			}
		}
		else {
			auto boneDataIter = skinnedMainObject->boneData.find(materialInfo.materialFlags);
			skinnedMainObject->createBoneBuffer(skeletonID, boneCount);
			if (boneDataIter == skinnedMainObject->boneData.end()) {

				SkinRS::PipelineStruct& bonePipe = skinnedMainObject->createBonePipe(materialInfo.materialFlags);
				bonePipe.skeletonData.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{SkinRS::TextureMeshStruct{ materialInfo.texture, std::vector<EWEModel*>{modelPtr} }});
			}
			else {
				addSkeletonToStructs(boneDataIter->second.skeletonData, materialInfo.texture, modelPtr, skeletonID);
			}
		}
	}
	void SkinRenderSystem::addWeapon(MaterialTextureInfo& materialInfo, EWEModel* modelPtr, SkeletonID skeletonID, SkeletonID ownerID) {
		//need this to reference the owner buffer


		if (!skinnedMainObject->buffers.contains(skeletonID)) {
			std::cout << "creating reference buffer \n";
			skinnedMainObject->createReferenceBuffer(skeletonID, ownerID);
		}

		auto boneDataIter = skinnedMainObject->boneData.find(materialInfo.materialFlags);

		if (boneDataIter == skinnedMainObject->boneData.end()) {

			SkinRS::PipelineStruct& retPipe = skinnedMainObject->createBonePipe(materialInfo.materialFlags);
			auto emplaceRet = retPipe.skeletonData.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{});
			//auto& secondRef = 
			emplaceRet.first->second.emplace_back(materialInfo.texture, std::vector<EWEModel*>{modelPtr});
			//secondRef.meshes.push_back(modelPtr);
		}
		else {
			addSkeletonToStructs(boneDataIter->second.skeletonData, materialInfo.texture, modelPtr, skeletonID);
		}
	}

	void SkinRenderSystem::removeSkeleton(SkeletonID skeletonID) {
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