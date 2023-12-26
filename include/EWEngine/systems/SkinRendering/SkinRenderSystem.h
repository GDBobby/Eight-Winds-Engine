#pragma once

#include "SkinBufferHandler.h"

#include "EWEngine/graphics/model/EWE_Model.h"
#include "EWEngine/graphics/EWE_Pipeline.h"

#include <algorithm>


namespace EWE {
	class SkinRenderSystem {
	private:
		static SkinRenderSystem* skinnedMainObject;
		struct TextureMeshStruct {
			TextureID textureID;
			std::vector<EWEModel*> meshes;
			TextureMeshStruct(TextureID textureID) : textureID{ textureID }, meshes{} {}
			TextureMeshStruct(TextureID textureID, std::vector<EWEModel*> meshes) : textureID{ textureID }, meshes{ meshes }{}
		};

		struct PushConstantStruct {
			std::vector<void*> data{};
			uint8_t size{};
			uint8_t count{ 0 };
			PushConstantStruct(void* data, uint8_t size) : data{ data }, size{ size }{
				count++;
			}
			void addData(void* data, uint8_t pushSize) {
#ifdef _DEBUG
				if (pushSize != size) {
					printf("misaligned push size between skeletons of the same id \n");
					throw std::exception("misaligned push size between skeletons of the same id");
				}
#endif
				count++;
				this->data.emplace_back(data);
			}
			void remove(void* removalData) {

				auto findVal = std::find(data.cbegin(), data.cend(), removalData);
				if (findVal != data.cend()) {
					//std::cout << "successfully removed push data \n";
					data.erase(findVal);
					count--;
				}
				//else {
				//	std::cout << "failed to find address of push to be removed \n";
				//}
			}
		};

		struct PipelineStruct {
			std::unique_ptr<EWEPipeline> pipeline;
			uint16_t pipeLayoutIndex; //a lot of work to find this value, might as well just store it
			std::unordered_map<SkeletonID, std::vector<TextureMeshStruct>> skeletonData; //key is skeletonID

			PipelineStruct(uint16_t boneCount, ShaderFlags textureFlags, VkPipelineRenderingCreateInfo const& pipeRenderInfo, std::shared_ptr<EWEDescriptorPool> globalPool, EWEDevice& device) :
				pipeline{ PipelineManager::createInstancedRemote(textureFlags, boneCount, pipeRenderInfo, device) }, skeletonData{}
				//instanced
			{
				bool hasBumps = textureFlags & DynF_hasBump;
				bool hasNormal = textureFlags & DynF_hasNormal;
				bool hasRough = textureFlags & DynF_hasRough;
				bool hasMetal = textureFlags & DynF_hasMetal;
				bool hasAO = textureFlags & DynF_hasAO;

				uint8_t textureCount = hasNormal + hasRough + hasMetal + hasAO + hasBumps;
				pipeLayoutIndex = textureCount + (3 * MAX_SMART_TEXTURE_COUNT);
			}
			PipelineStruct(ShaderFlags textureFlags, VkPipelineRenderingCreateInfo const& pipeRenderInfo, std::shared_ptr<EWEDescriptorPool> globalPool, EWEDevice& device) :
				pipeline{ PipelineManager::createBoneRemote(textureFlags, pipeRenderInfo, device) }, skeletonData{}
				//non instanced
			{
				bool hasBumps = textureFlags & DynF_hasBump;
				bool hasNormal = textureFlags & DynF_hasNormal;
				bool hasRough = textureFlags & DynF_hasRough;
				bool hasMetal = textureFlags & DynF_hasMetal;
				bool hasAO = textureFlags & DynF_hasAO;

				uint8_t textureCount = hasNormal + hasRough + hasMetal + hasAO + hasBumps;
				pipeLayoutIndex = textureCount + MAX_SMART_TEXTURE_COUNT;
			}
		};

	public:

		SkinRenderSystem(EWEDevice& device, std::shared_ptr<EWEDescriptorPool> globalPool, VkPipelineRenderingCreateInfo const& pipeRenderInfo);
		~SkinRenderSystem();
		//~MonsterBoneBufferDescriptorStruct();

		//void addActorToBuffer(glm::mat4* modelMatrix, void* finalBoneMatrices, uint32_t skeletonID);
		void updateBuffers(uint8_t frameIndex);

		void flushBuffers(uint8_t frameIndex);

		void render(std::pair<VkCommandBuffer, uint8_t> cmdIndexPair);

		static SkeletonID getSkinID() {
			return skinnedMainObject->skinID++;
		}

		static void addSkeleton(std::pair<ShaderFlags, TextureID>& texturePair, uint16_t boneCount, EWEModel* modelPtr, SkeletonID skeletonID, bool instanced);
		static void addWeapon(std::pair<ShaderFlags, TextureID>& texturePair, EWEModel* meshes, SkeletonID skeletonID, SkeletonID ownerID);

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

		std::unordered_map<SkeletonID, PipelineStruct> instancedData{};
		std::unordered_map<ShaderFlags, PipelineStruct> boneData{};
		//uint8_t frameIndex = 0;

		//changes memory size allocated to buffers
		void changeActorCount(SkeletonID skeletonID, uint8_t maxActorCount) {
#if _DEBUG
			if (buffers.find(skeletonID) == buffers.end()) {
				printf("trying to change the max actor count for a buffer that doesn't exist \n");
				throw std::exception("trying to change the max actor count for a buffer that doesn't exist");
			}
#endif
			buffers.at(skeletonID).changeMaxActorCount(device, maxActorCount, globalPool);

		}

		static void setPushData(SkeletonID skeletonID, void* pushData, uint8_t pushSize) {
			if (skinnedMainObject->pushConstants.find(skeletonID) == skinnedMainObject->pushConstants.end()) {
				skinnedMainObject->pushConstants.emplace(skeletonID, PushConstantStruct{ pushData, pushSize });
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
			instancedBuffers.emplace(skeletonID, InstancedSkinBufferHandler{ device, boneCount, 2000, globalPool });
		}
		void createBoneBuffer(SkeletonID skeletonID, uint16_t boneCount) {
			if (buffers.find(skeletonID) != buffers.end()) {
				return;
				printf("creating a buffer that already exist \n");
				throw std::runtime_error("creating a buffer that already exist ");
			}
			//buffersCreated += 2;
			buffers.emplace(skeletonID, SkinBufferHandler{ device, boneCount, 1, globalPool });
		}
		void createReferenceBuffer(SkeletonID skeletonID, SkeletonID referenceID) {
			if (buffers.find(skeletonID) != buffers.end()) {
				return;
				printf("creating a buffer that already exist \n");
				throw std::runtime_error("creating a buffer that already exist ");
			}
			buffers.emplace(skeletonID, SkinBufferHandler{ 1, buffers.at(referenceID).getInnerPtr() });
		}

		void createInstancedPipe(SkeletonID instancedFlags, uint16_t boneCount, ShaderFlags textureFlags) {
			instancedData.emplace(instancedFlags,
				PipelineStruct{ boneCount, textureFlags, pipeRenderInfo, globalPool, device }
			);
		}
		void createBonePipe(ShaderFlags boneFlags) {
			boneData.emplace(boneFlags, PipelineStruct{ boneFlags, pipeRenderInfo, globalPool, device });
		}

		uint32_t skinID = 0;

		//key is skeletonID
		std::unordered_map<SkeletonID, SkinBufferHandler> buffers{};
		std::unordered_map<SkeletonID, InstancedSkinBufferHandler> instancedBuffers{};
		std::unordered_map<SkeletonID, PushConstantStruct> pushConstants{};

		VkPipelineRenderingCreateInfo const& pipeRenderInfo;
		std::shared_ptr<EWEDescriptorPool> globalPool;
		EWEDevice& device;

		//uint32_t buffersCreated = 0;
		//uint32_t instancedBuffersCreated = 0;
	};
}

