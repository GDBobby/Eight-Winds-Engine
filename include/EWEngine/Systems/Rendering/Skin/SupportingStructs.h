#pragma once

#include "EWEngine/Data/EngineDataTypes.h"
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

namespace EWE {
	namespace SkinRS {
		struct TextureMeshStruct {
			std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets;
			ImageID imageID;
			std::vector<EWEModel*> meshes;
			TextureMeshStruct(std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets, ImageID imgID) : descriptorSets{ descriptorSets }, imageID{ imgID }, meshes {} {}
			TextureMeshStruct(std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets, std::vector<EWEModel*> meshes, ImageID imgID) : descriptorSets{ descriptorSets }, imageID{ imgID }, meshes{ meshes } {}
		};
		struct PushConstantStruct {
			std::vector<void*> data{};
			uint8_t size{};
			uint8_t count{ 0 };
			PushConstantStruct(void* data, uint8_t size) : data{ data }, size{ size } {
				count++;
			}
			void AddData(void* data, uint8_t pushSize) {
#if EWE_DEBUG
				assert(pushSize == size && "misaligned push size");
#endif
				count++;
				this->data.emplace_back(data);
			}
			void Remove(void* removalData) {

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
			//std::unique_ptr<EWEPipeline> pipeline;
			MaterialPipelines* pipeline;
			std::unordered_map<SkeletonID, std::vector<TextureMeshStruct>> skeletonData; //key is skeletonID

			PipelineStruct(uint16_t boneCount, MaterialFlags materialFlags) :
				//pipeline{ PipelineManager::createInstancedRemote(textureFlags, boneCount, pipeRenderInfo, device) }, 
				pipeline{ MaterialPipelines::GetMaterialPipe(materialFlags, boneCount) },
				skeletonData{}
				//instanced
			{}

			PipelineStruct(MaterialFlags materialFlags) :
				//pipeline{ PipelineManager::createBoneRemote(textureFlags, pipeRenderInfo, device) }, 
				pipeline{ MaterialPipelines::GetMaterialPipe(materialFlags) },
				skeletonData{}
				//non instanced
			{
				printf("~~~~~ constructing skin pipeline : %d:%d  ~~~~\n", materialFlags, materialFlags & Material::Flags::Other::Bones);
			}
		};
	}
}