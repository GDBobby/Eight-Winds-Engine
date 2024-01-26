#include "EWEngine/Systems/Rendering/Rigid/RigidRS.h"

#include "EWEngine/Graphics/PushConstants.h"

namespace EWE {
    void MaterialRenderInfo::render(uint8_t frameIndex) {
        if (!materialMap.size()) {
            return;
        }
        pipe->bindPipeline();
        pipe->bindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameIndex));
        for (auto iterTexID = materialMap.begin(); iterTexID != materialMap.end(); iterTexID++) {

            pipe->bindTextureDescriptor(1, iterTexID->first);

            for (auto& renderInfo : iterTexID->second) {
                if (!renderInfo.drawable) {
                    continue;
                }

                SimplePushConstantData push{ renderInfo.ownerTransform->mat4(), renderInfo.ownerTransform->normalMatrix() };

                pipe->bindModel(renderInfo.meshPtr);
                pipe->pushAndDraw(&push);
            }
        }
    }


    RigidRenderingSystem* RigidRenderingSystem::rigidInstance{nullptr};

    //const std::map<MaterialFlags, std::map<TextureID, std::vector<MaterialObjectInfo>>>& RigidRenderingSystem::cleanAndGetMaterialMap() {
    //    for (auto iter = materialMap.begin(); iter != materialMap.end();) {
    //        if (iter->second.size() == 0) {
    //            iter = materialMap.erase(iter);
    //            //clean up pipeline or leave it be? not sure how expensive it is to maintain while not in use
    //        }
    //        else {
    //            iter++;
    //        }
    //    }
    //    return materialMap;
    //}

    void RigidRenderingSystem::addMaterialObject(EWEDevice& device, MaterialTextureInfo materialInfo, MaterialObjectInfo& renderInfo) {
        if (renderInfo.meshPtr == nullptr) {
            printf("NULLTPR MESH EXCEPTION \n");
            throw std::runtime_error("nullptr mesh");
        }
        if (!materialMap.contains(materialInfo.materialFlags)) {
            materialMap.try_emplace(materialInfo.materialFlags, materialInfo.materialFlags, device);
        }

        materialMap.at(materialInfo.materialFlags).materialMap.at(materialInfo.textureID).push_back(renderInfo);
    }
    void RigidRenderingSystem::addMaterialObject(EWEDevice& device, MaterialTextureInfo materialInfo, TransformComponent* ownerTransform, EWEModel* modelPtr, bool* drawable) {
        if (modelPtr == nullptr) {
            printf("NULLTPR MESH EXCEPTION \n");
            throw std::runtime_error("nullptr mesh");
        }

        if (!materialMap.contains(materialInfo.materialFlags)) {
            materialMap.try_emplace(materialInfo.materialFlags, materialInfo.materialFlags, device);
        }
        materialMap.at(materialInfo.materialFlags).materialMap.at(materialInfo.textureID).emplace_back(ownerTransform, modelPtr, drawable);

    }
    void RigidRenderingSystem::addMaterialObjectFromTexID(TextureID copyID, TransformComponent* ownerTransform, bool* drawablePtr) {
        for (auto iter = materialMap.begin(); iter != materialMap.end(); iter++) {
            for (auto iterTexID = iter->second.materialMap.begin(); iterTexID != iter->second.materialMap.end(); iterTexID++) {
                if (iterTexID->first == copyID) {
                    if (iterTexID->second.size() == 0 || iterTexID->second[0].meshPtr == nullptr) {
                        printf("NULLTPR MESH EXCEPTION or SIZE IS 0 \n");
                        throw std::runtime_error("nullptr mesh or size is 0");
                    }

                    iterTexID->second.push_back(iterTexID->second[0]);
                    iterTexID->second.back().ownerTransform = ownerTransform;
                    iterTexID->second.back().meshPtr = iterTexID->second[0].meshPtr;
                    iterTexID->second.back().drawable = drawablePtr;
                    return;
                }
            }
        }
    }
    void RigidRenderingSystem::removeByTransform(TextureID textureID, TransformComponent* ownerTransform) {
        for (auto iter = materialMap.begin(); iter != materialMap.end(); iter++) {
            for (auto iterTexID = iter->second.materialMap.begin(); iterTexID != iter->second.materialMap.end(); iterTexID++) {
                if (iterTexID->first == textureID) {
                    for (int i = 0; i < iterTexID->second.size(); i++) {
                        if (iterTexID->second[i].ownerTransform == ownerTransform) {
                            iterTexID->second.erase(iterTexID->second.begin() + i);
                            i--;
                        }
                    }
                }
            }

        }
    }
    std::vector<TextureID> RigidRenderingSystem::checkAndClearTextures() {
        std::vector<TextureID> returnVector;
        for (auto iter = materialMap.begin(); iter != materialMap.end(); iter++) {

            //bool removedTexID = false;
            for (auto iterTexID = iter->second.materialMap.begin(); iterTexID != iter->second.materialMap.end();) {
                if (iterTexID->second.size() == 0) {
                    returnVector.push_back(iterTexID->first);
                    iterTexID = iter->second.materialMap.erase(iterTexID);
                    //removedTexID = true;
                }
                else {
                    iterTexID++;
                }
            }
            //if (!removedTexID) {
            //    iter++;
            //}
        }
        return returnVector;
    }
    
    void RigidRenderingSystem::render(FrameInfo const& frameInfo) {
        //ill replace this shit eventually
#ifdef _DEBUG
        assert(rigidInstance != nullptr && "material handler instance is nullptr while trying to render with it");
#endif
        rigidInstance->renderMemberMethod(frameInfo);
    }
    void RigidRenderingSystem::renderMemberMethod(FrameInfo const& frameInfo){

        for (auto iter = materialMap.begin(); iter != materialMap.end(); iter++) {

#if DEBUGGING_DYNAMIC_PIPE || DEBUGGING_PIPELINES
            printf("checking validity of map iter? \n");
            printf("iter->first:second - %d:%d \n", iter->first, iter->second.size());
            uint8_t flags = iter->first;
            printf("Drawing dynamic materials : %d \n", flags);
            if (flags & 128) {
                printf("should not have bonesin static rendering \n");
                throw std::runtime_error("should not have boens here");
            }
#elif _DEBUG

            uint8_t flags = iter->first;
            if (flags & 128) {
                printf("should not have bonesin static rendering \n");
                throw std::runtime_error("should not have boens here");
            }
#endif
            iter->second.render(frameInfo.index);


#if DEBUGGING_DYNAMIC_PIPE
            printf("finished drawing dynamic material flag : %d \n", flags);
#endif
        }
#if DEBUGGING_PIPELINES || DEBUGGING_DYNAMIC_PIPE
        printf("finished dynamic render \n");
#endif
    }
}