#pragma once

#include "EWEngine/Data/EngineDataTypes.h"
#include "EWEngine/Graphics/Model/Model.h"
#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

namespace EWE {
	namespace SkinRS {
		struct TextureMeshStruct {
			TextureDesc texture;
			std::vector<EWEModel*> meshes;
			TextureMeshStruct(TextureDesc texture) : texture{ texture }, meshes{} {}
			TextureMeshStruct(TextureDesc texture, std::vector<EWEModel*> meshes) : texture{ texture }, meshes{ meshes } {}
		};
		struct PushConstantStruct {
			std::vector<void*> data{};
			uint8_t size{};
			uint8_t count{ 0 };
			PushConstantStruct(void* data, uint8_t size) : data{ data }, size{ size } {
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
			//std::unique_ptr<EWEPipeline> pipeline;
			MaterialPipelines* pipeline;
			std::unordered_map<SkeletonID, std::vector<TextureMeshStruct>> skeletonData; //key is skeletonID

			PipelineStruct(uint16_t boneCount, MaterialFlags materialFlags) :
				//pipeline{ PipelineManager::createInstancedRemote(textureFlags, boneCount, pipeRenderInfo, device) }, 
				pipeline{ MaterialPipelines::getInstancedSkinMaterialPipe(boneCount, materialFlags) },
				skeletonData{}
				//instanced
			{}

			PipelineStruct(MaterialFlags materialFlags) :
				//pipeline{ PipelineManager::createBoneRemote(textureFlags, pipeRenderInfo, device) }, 
				pipeline{ MaterialPipelines::getMaterialPipe(materialFlags) },
				skeletonData{}
				//non instanced
			{
				printf("~~~~~ constructing skin pipeline : %d:%d  ~~~~\n", materialFlags, materialFlags & MaterialF_hasBones);
			}
		};
	}
}