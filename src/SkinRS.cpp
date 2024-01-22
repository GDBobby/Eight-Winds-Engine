#include "EWEngine/Systems/Rendering/Skin/SkinRS.h"
#include "EWEngine/Graphics/Textures/Texture_Manager.h"
#include "EWEngine/Graphics/Textures/Material_Textures.h"

#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

#define RENDER_DEBUG false

namespace EWE {
	SkinRenderSystem* SkinRenderSystem::skinnedMainObject = nullptr;


	SkinRenderSystem::SkinRenderSystem(EWEDevice& device) : device { device } {
		skinnedMainObject = this;

		//this initializes the descriptors
		DescriptorHandler::getDescSetLayout(LDSL_boned, device);
		DescriptorHandler::getDescSetLayout(LDSL_largeInstance, device);
		
		MaterialPipelines::initStaticVariables();
	
	}
	SkinRenderSystem::~SkinRenderSystem() {
		printf("before clearing pipes \n");
		MaterialPipelines::cleanupStaticVariables(device);

		printf("before clearing skin buffer descriptors");// , amount created - % d: % d \n", buffersCreated, instancedBuffersCreated);
		uint16_t bufferDescriptorsCleared = 0;
		uint16_t instancedBuffersCleared = 0;
		for (auto& buffer : buffers) {
			if (!buffer.second.CheckReference()) {
				for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					buffer.second.setFrameIndex(i);
					EWEDescriptorPool::freeDescriptor(DescriptorPool_Global, buffer.second.getDescriptor());
					bufferDescriptorsCleared++;
				}
			}
		}
		for (auto& instanceBuffer : instancedBuffers) {
			for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				instanceBuffer.second.setFrameIndex(i);
				EWEDescriptorPool::freeDescriptor(DescriptorPool_Global, instanceBuffer.second.getDescriptor());
				instancedBuffersCleared++;
			}
		}
		printf("after clearing buffer descriptors - count - %d:%d  \n", bufferDescriptorsCleared, instancedBuffersCleared);
		skinnedMainObject = nullptr;
	}

	void SkinRenderSystem::renderInstanced(VkCommandBuffer cmdBuf, uint8_t frameIndex) {

		MaterialPipelines* pipe;

		for (auto& instanced : instancedData) {
			//if instanced.first.actorCount == 0 return;
			pipe = instanced.second.pipeline;

			pipe->bindPipeline();

			pipe->bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameIndex));

			int64_t bindedSkeletonID = -1;
			int32_t currentlyBindedTexture = -1;

			for (auto& skeleDataRef : instanced.second.skeletonData) {
				//printf("instance count? - %d:%d \n", skeleDataRef.first, instancedBuffers.at(skeleDataRef.first).getInstanceCount());

				if (instancedBuffers.at(skeleDataRef.first).getInstanceCount() <= 0) {
					continue;
				}

				if (bindedSkeletonID != skeleDataRef.first) {
					bindedSkeletonID = skeleDataRef.first;
#ifdef _DEBUG
					if (instancedBuffers.find(skeleDataRef.first) == instancedBuffers.end()) {
						std::cout << std::format("buffer at {} does not exist \n", bindedSkeletonID) << std::endl;
						throw std::exception("skinned rs invlaid buffer");
					}
#endif
					pipe->bindDescriptor(1, instancedBuffers.at(skeleDataRef.first).getDescriptor());
				}
				for (auto& skeleTextureRef : skeleDataRef.second) {
					if (currentlyBindedTexture != skeleTextureRef.textureID) {
						currentlyBindedTexture = skeleTextureRef.textureID;
						pipe->bindDescriptor(2, Texture_Manager::getDescriptorSet(currentlyBindedTexture));
					}

					for (auto& meshRef : skeleTextureRef.meshes) {
						//meshRef->BindAndDrawInstanceNoBuffer(cmdIndexPair.first, actorCount.at(instanced.first));
						//printf("drawing instanced : %d \n", instancedBuffers.at(bindedSkeletonID).getInstanceCount());
						meshRef->BindAndDrawInstanceNoBuffer(cmdBuf, instancedBuffers.at(skeleDataRef.first).getInstanceCount());
					}
				}

			}
		}
		//printf("after indexed drawing \n");
		for (auto iter = instancedBuffers.begin(); iter != instancedBuffers.end(); iter++) {
			iter->second.resetInstanceCount();
		}

	}
	void SkinRenderSystem::renderNonInstanced(VkCommandBuffer cmdBuf, uint8_t frameIndex) {

		MaterialPipelines* pipe;
		for (auto& boned : boneData) {
			pipe = boned.second.pipeline;
			//printf("shader flags on non-instanced : %d \n", boned.first);
			pipe->bindPipeline();

			pipe->bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameIndex));
			int32_t currentlyBindedTexture = -1;

			for (auto& skeleDataRef : boned.second.skeletonData) {
				if (pushConstants.find(skeleDataRef.first) == pushConstants.end()) {
#ifdef _DEBUG
					//std::cout << "this skeleton doesn't have push constants - skeletonID : " << skeleDataRef.first << std::endl;
#endif
					continue;
				}
				if (pushConstants.at(skeleDataRef.first).count == 0) {
					//std::cout << "push count is 0 : " << std::endl;
					continue;
				}
#ifdef _DEBUG
				if (buffers.find(skeleDataRef.first) == buffers.end()) {
					printf("buffer at %d does not exist \n", skeleDataRef.first);
					throw std::exception("skinned rs invlaid buffer");
				}
#endif
				pipe->bindDescriptor(1, buffers.at(skeleDataRef.first).getDescriptor());

				for (auto& skeleTextureRef : skeleDataRef.second) {
					if (currentlyBindedTexture != skeleTextureRef.textureID) {
						currentlyBindedTexture = skeleTextureRef.textureID;

						pipe->bindDescriptor(2, Texture_Manager::getDescriptorSet(currentlyBindedTexture));
					}

					//race condition here for deletion of push constant

					for (auto& meshRef : skeleTextureRef.meshes) {
						//printf("for each mesh in non-instanced \n");
						//meshRef->BindAndDrawInstanceNoBuffer(cmdIndexPair.first, actorCount.at(instanced.first));
						pipe->bindModel(meshRef);

#if RENDER_DEBUG
						printf("before drawing to pipeline : %d \n", boned.first);
#endif
						for (auto& pushData : pushConstants.at(skeleDataRef.first).data) {
							pipe->pushAndDraw(pushData);
						}
					}
				}
			}
		}
	}

	void SkinRenderSystem::render(std::pair<VkCommandBuffer, uint8_t> cmdIndexPair) {
		//printf("skin render? \n");
		/*
		if (enemyData->actorCount[actorType - 3] == 0) {
			return;
		}
		*/
		//^ figure out how to contain new actors to replace that line
		//setFrameIndex(cmdIndexPair.second);

		//printf("pre instance drawing in skinned RS \n");
		MaterialPipelines::setCmdIndexPair(cmdIndexPair);
		renderInstanced(cmdIndexPair.first, cmdIndexPair.second);

		renderNonInstanced(cmdIndexPair.first, cmdIndexPair.second);
		

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
			buffers[cmdIndexPair.second][0]->flush();
		}
		if (currentMemOffset[1] > 0) {
			buffers[cmdIndexPair.second][1]->flush();
		}
		//buffers[cmdIndexPair.second][2]->flush();


		currentMemOffset[0] = 0;
		currentMemOffset[1] = 0;
		//currentMemOffset[2] = 0;
		//currentBoneOffset = 0;
		*/

	}
	void SkinRenderSystem::addSkeleton(MaterialTextureInfo& materialInfo, uint16_t boneCount, EWEModel* modelPtr, SkeletonID skeletonID, bool instanced) {
#ifdef _DEBUG
		if (skinnedMainObject == nullptr) {
			printf("skinned main object is nullptr \n");
			throw std::exception("skinned main object is nullptr");
		}
#endif
		if (instanced) {
			uint32_t instancedFlags = (boneCount << 16) + materialInfo.materialFlags;

			if (skinnedMainObject->instancedData.find(instancedFlags) == skinnedMainObject->instancedData.end()) {

				skinnedMainObject->createInstancedBuffer(skeletonID, boneCount);

				skinnedMainObject->createInstancedPipe(instancedFlags, boneCount, materialInfo.materialFlags);
				skinnedMainObject->instancedData.at(instancedFlags).skeletonData.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{});
				//auto& secondRef = 
				skinnedMainObject->instancedData.at(instancedFlags).skeletonData.at(skeletonID).emplace_back(materialInfo.textureID, std::vector<EWEModel*>{modelPtr});
				//secondRef.meshes.push_back(modelPtr);
			}
			else {

				auto& skeleRef = skinnedMainObject->instancedData.at(instancedFlags).skeletonData;
				if (skeleRef.find(skeletonID) != skeleRef.end()) {
					bool foundATextureMatch = false;
					for (auto& textureRef : skeleRef.at(skeletonID)) {
						if (textureRef.textureID == materialInfo.textureID) {
							foundATextureMatch = true;
							textureRef.meshes.push_back(modelPtr);
							break;
						}
					}
					if (!foundATextureMatch) {
						skeleRef.at(skeletonID).emplace_back(materialInfo.textureID, std::vector<EWEModel*>{modelPtr});
					}
				}
				else {
					skeleRef.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{});
					skeleRef.at(skeletonID).emplace_back(materialInfo.textureID, std::vector<EWEModel*>{modelPtr});
				}
			}
		}
		else {
			skinnedMainObject->createBoneBuffer(skeletonID, boneCount);

			if (skinnedMainObject->boneData.find(materialInfo.materialFlags) == skinnedMainObject->boneData.end()) {

				skinnedMainObject->createBonePipe(materialInfo.materialFlags);
				skinnedMainObject->boneData.at(materialInfo.materialFlags).skeletonData.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{});
				//auto& secondRef = 
				skinnedMainObject->boneData.at(materialInfo.materialFlags).skeletonData.at(skeletonID).emplace_back(materialInfo.textureID, std::vector<EWEModel*>{modelPtr});
				//secondRef.meshes.push_back(modelPtr);
			}
			else {

				auto& skeleRef = skinnedMainObject->boneData.at(materialInfo.materialFlags).skeletonData;
				if (skeleRef.find(skeletonID) != skeleRef.end()) {
					bool foundATextureMatch = false;
					for (auto& textureRef : skeleRef.at(skeletonID)) {
						if (textureRef.textureID == materialInfo.textureID) {
							foundATextureMatch = true;
							textureRef.meshes.push_back(modelPtr);
							break;
						}
					}
					if (!foundATextureMatch) {
						skeleRef.at(skeletonID).emplace_back(materialInfo.textureID, std::vector<EWEModel*>{modelPtr});
					}
				}
				else {
					skeleRef.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{});
					skeleRef.at(skeletonID).emplace_back(materialInfo.textureID, std::vector<EWEModel*>{modelPtr});
				}
			}
		}
	}
	void SkinRenderSystem::addWeapon(MaterialTextureInfo& materialInfo, EWEModel* modelPtr, SkeletonID skeletonID, SkeletonID ownerID) {
		//need this to reference the owner buffer
		if (skinnedMainObject->buffers.find(skeletonID) == skinnedMainObject->buffers.end()) {
			std::cout << "creating reference buffer \n";
			skinnedMainObject->createReferenceBuffer(skeletonID, ownerID);
		}
		if (skinnedMainObject->boneData.find(materialInfo.materialFlags) == skinnedMainObject->boneData.end()) {

			skinnedMainObject->createBonePipe(materialInfo.materialFlags);
			skinnedMainObject->boneData.at(materialInfo.materialFlags).skeletonData.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{});
			//auto& secondRef = 
			skinnedMainObject->boneData.at(materialInfo.materialFlags).skeletonData.at(skeletonID).emplace_back(materialInfo.textureID, std::vector<EWEModel*>{modelPtr});
			//secondRef.meshes.push_back(modelPtr);
		}
		else {

			auto& skeleRef = skinnedMainObject->boneData.at(materialInfo.materialFlags).skeletonData;
			if (skeleRef.find(skeletonID) != skeleRef.end()) {
				bool foundATextureMatch = false;
				for (auto& textureRef : skeleRef.at(skeletonID)) {
					if (textureRef.textureID == materialInfo.textureID) {
						foundATextureMatch = true;
						textureRef.meshes.push_back(modelPtr);
						break;
					}
				}
				if (!foundATextureMatch) {
					skeleRef.at(skeletonID).emplace_back(materialInfo.textureID, std::vector<EWEModel*>{modelPtr});
				}
			}
			else {
				skeleRef.emplace(skeletonID, std::vector<SkinRS::TextureMeshStruct>{});
				skeleRef.at(skeletonID).emplace_back(materialInfo.textureID, std::vector<EWEModel*>{modelPtr});
			}
		}
	}

	void SkinRenderSystem::removeSkeleton(SkeletonID skeletonID) {
		for (auto& instanced : skinnedMainObject->instancedData) {
			if (instanced.second.skeletonData.find(skeletonID) != instanced.second.skeletonData.end()) {
				for (auto& textureRef : instanced.second.skeletonData.at(skeletonID)) {
					//skinnedMainObject->cleanTheseTextures.push_back(textureRef.textureID);
				}
				//texture erasure here, if i decide to do it that way
			}
			if (instanced.second.skeletonData.erase(skeletonID) > 0) {
				printf("erasing an instanced skeleton : %d \n", skeletonID);
				if (instanced.second.skeletonData.size() == 0) {
#ifdef _DEBUG
					printf("erasing a instanced skin pipeline :%d \n", instanced.first);
#endif

					skinnedMainObject->instancedData.erase(instanced.first);
					//need to remove textures, 
					//or have textures remove themself, probably the better option
					//currently, texture data will remain forever
				}
			}
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
#ifdef _DEBUG
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