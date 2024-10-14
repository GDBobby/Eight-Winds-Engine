#include "EWEngine/Systems/Rendering/Rigid/RigidRS.h"

#include "EWEngine/Graphics/PushConstants.h"

namespace EWE {
    void MaterialRenderInfo::Render(uint8_t frameIndex) {
        if (materialMap.size() == 0) {
            return;
        }
        pipe->BindPipeline();
        pipe->BindDescriptor(0, DescriptorHandler::GetDescSet(DS_global, frameIndex));
        for (auto iterTexID = materialMap.begin(); iterTexID != materialMap.end(); iterTexID++) {

            pipe->BindTextureDescriptor(1, iterTexID->first);

            for (auto& renderInfo : iterTexID->second) {
                if (!renderInfo.drawable) {
                    continue;
                }

                SimplePushConstantData push{ renderInfo.ownerTransform->mat4(), renderInfo.ownerTransform->normalMatrix() };

                pipe->BindModel(renderInfo.meshPtr);
                pipe->PushAndDraw(&push);
            }
        }
    }
    void InstancedMaterialRenderInfo::Render(uint8_t frameIndex) {
        if (instancedInfo.size() == 0) { return; }
        pipe->BindPipeline();
        pipe->BindDescriptor(0, DescriptorHandler::GetDescSet(DS_global, frameIndex));
        for (auto const& instanceInfo : instancedInfo) {
            pipe->BindTextureDescriptor(1, instanceInfo.texture);
            const uint32_t instanceCount = instanceInfo.buffer.GetCurrentEntityCount();
            if (instanceCount == 0) {
                continue;
            }
            
            pipe->DrawInstanced(instanceInfo.meshPtr, instanceCount);
        }
    }

    namespace RigidRenderingSystem {
        std::unordered_map<MaterialFlags, MaterialRenderInfo>* materialMap{ nullptr };
        std::unordered_map<MaterialFlags, InstancedMaterialRenderInfo>* instancedMaterialMap{ nullptr };


        void Initialize() {
            materialMap = Construct<std::unordered_map<MaterialFlags, MaterialRenderInfo>>({});
        }
        void Destruct() {
            materialMap->clear();
            delete materialMap;
            Deconstruct(materialMap);
        }
        void AddInstancedMaterialObject(MaterialTextureInfo materialInfo, EWEModel* modelPtr, uint32_t entityCount, bool computedTransforms) {
            assert(modelPtr != nullptr);
            auto findRet = instancedMaterialMap->find(materialInfo.materialFlags);
            if (findRet == instancedMaterialMap->end()) {
                auto empRet = instancedMaterialMap->try_emplace(materialInfo.materialFlags, materialInfo.materialFlags);
                empRet.first->second.instancedInfo.emplace_back(materialInfo.texture, modelPtr, entityCount, computedTransforms);
            }
            else {
#if EWE_DEBUG
                //texture shouldn't be used in two separate models. unless it's like a ubertexture or texturearray i guess
                //if this becomes an issue, I'll need an unordered map of textures paired with vectors of the rest of the InstancedMaterialRenderInfo struct
                for (auto const& instance : findRet->second.instancedInfo) {
                    assert(instance.texture != materialInfo.texture && "duplicating textures");
                }
#endif
                findRet->second.instancedInfo.emplace_back(materialInfo.texture, modelPtr, entityCount, computedTransforms);
            }
        }

void AddMaterialObject(MaterialTextureInfo materialInfo, MaterialObjectInfo& renderInfo) {
#if EWE_DEBUG
    assert(renderInfo.meshPtr != nullptr);
#endif
    auto findRet = materialMap->find(materialInfo.materialFlags);
    if (findRet == materialMap->end()) {
        auto empRet = materialMap->try_emplace(materialInfo.materialFlags, materialInfo.materialFlags);
        empRet.first->second.materialMap.try_emplace(materialInfo.texture, std::vector<MaterialObjectInfo>{renderInfo});
    }
    else {
        auto textureFindRet = findRet->second.materialMap.find(materialInfo.texture);
        if (textureFindRet == findRet->second.materialMap.end()) {
            findRet->second.materialMap.try_emplace(materialInfo.texture, std::vector<MaterialObjectInfo>{renderInfo});
        }
        else {
            textureFindRet->second.push_back(renderInfo);
        }
                
    }
}
        void AddMaterialObject(MaterialTextureInfo materialInfo, TransformComponent* ownerTransform, EWEModel* modelPtr, bool* drawable) {
#if EWE_DEBUG
            assert(modelPtr != nullptr);
#endif

            if (!materialMap->contains(materialInfo.materialFlags)) {
                auto empRet = materialMap->try_emplace(materialInfo.materialFlags, materialInfo.materialFlags);
                empRet.first->second.materialMap.try_emplace(materialInfo.texture, std::vector<MaterialObjectInfo>{MaterialObjectInfo{ ownerTransform, modelPtr, drawable }});
            }
            else {
                materialMap->at(materialInfo.materialFlags).materialMap.at(materialInfo.texture).emplace_back(ownerTransform, modelPtr, drawable);
            }
        }
        void AddMaterialObjectFromTexID(TextureDesc copyID, TransformComponent* ownerTransform, bool* drawablePtr) {
            for (auto iter = materialMap->begin(); iter != materialMap->end(); iter++) {
                for (auto iterTexID = iter->second.materialMap.begin(); iterTexID != iter->second.materialMap.end(); iterTexID++) {
                    if (iterTexID->first == copyID) {
#if EWE_DEBUG
                        assert(iterTexID->second.size() == 0 || iterTexID->second[0].meshPtr == nullptr);
#endif
                        iterTexID->second.push_back(iterTexID->second[0]);
                        iterTexID->second.back().ownerTransform = ownerTransform;
                        iterTexID->second.back().meshPtr = iterTexID->second[0].meshPtr;
                        iterTexID->second.back().drawable = drawablePtr;
                        return;
                    }
                }
            }
        }
        void RemoveByTransform(TextureDesc textureID, TransformComponent* ownerTransform) {
            for (auto iter = materialMap->begin(); iter != materialMap->end(); iter++) {
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
        std::vector<TextureDesc> CheckAndClearTextures() {
            std::vector<TextureDesc> returnVector;
            for (auto iter = materialMap->begin(); iter != materialMap->end(); iter++) {

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

        void RenderInstancedMemberMethod(FrameInfo const& frameInfo) {
            for (auto iter = instancedMaterialMap->begin(); iter != instancedMaterialMap->end(); iter++) {

#if DEBUGGING_MATERIAL_PIPE || DEBUGGING_PIPELINES
                printf("checking validity of map iter? \n");
                printf("iter->first:second - %d:%d \n", iter->first, iter->second.size());
                uint8_t flags = iter->first;
                printf("Drawing dynamic materials : %d \n", flags);
                assert(((flags & 128) == 0) && "should not have bones here");
#elif EWE_DEBUG

                uint8_t flags = iter->first;
                assert(((flags & 128) == 0) && "should not have bones here");
#endif
            }
        }
        void RenderMemberMethod(FrameInfo const& frameInfo) {

            for (auto iter = materialMap->begin(); iter != materialMap->end(); iter++) {

#if DEBUGGING_MATERIAL_PIPE || DEBUGGING_PIPELINES
                printf("checking validity of map iter? \n");
                printf("iter->first:second - %d:%d \n", iter->first, iter->second.size());
                uint8_t flags = iter->first;
                printf("Drawing dynamic materials : %d \n", flags);
                assert(((flags & 128) == 0) && "should not have bones here");
#elif EWE_DEBUG

                uint8_t flags = iter->first;
                assert(((flags & 128) == 0) && "should not have bones here");
#endif
                iter->second.Render(frameInfo.index);


#if DEBUGGING_MATERIAL_PIPE
                printf("finished drawing dynamic material flag : %d \n", flags);
#endif
            }
#if DEBUGGING_PIPELINES || DEBUGGING_MATERIAL_PIPE
            printf("finished dynamic render \n");
#endif
        }

        void Render(FrameInfo const& frameInfo) {
            //ill replace this shit eventually
            MaterialPipelines::SetFrameInfo(frameInfo);

            RenderMemberMethod(frameInfo);
            RenderInstancedMemberMethod(frameInfo);
        }
    }//namespace RigidRenderingSystem
} //namespace EWE