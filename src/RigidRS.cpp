#include "EWEngine/Systems/Rendering/Rigid/RigidRS.h"

#include "EWEngine/Graphics/PushConstants.h"

namespace EWE {
    void MaterialRenderInfo::Render() {
        if (materialMap.size() == 0) {
            return;
        }
        pipe->BindPipeline();
        pipe->BindDescriptor(0, DescriptorHandler::GetDescSet(DS_global));
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
    void InstancedMaterialRenderInfo::Render() {
        if (instancedInfo.size() == 0) { return; }
        pipe->BindPipeline();

        pipe->BindDescriptor(0, DescriptorHandler::GetDescSet(DS_global));
        for (auto const& instanceInfo : instancedInfo) {
            pipe->BindDescriptor(1, instanceInfo.buffer.GetDescriptor());
            pipe->BindTextureDescriptor(2, instanceInfo.texture);
            const uint32_t instanceCount = instanceInfo.buffer.GetCurrentEntityCount();
            if (instanceCount == 0) {
                continue;
            }
            else{
                pipe->DrawInstanced(instanceInfo.meshPtr, instanceCount);
            }
        }
    }

    namespace RigidRenderingSystem {
        std::unordered_map<MaterialFlags, MaterialRenderInfo>* materialMap{ nullptr };
        std::unordered_map<MaterialFlags, InstancedMaterialRenderInfo>* instancedMaterialMap{ nullptr };


        void Initialize() {
            materialMap = Construct<std::unordered_map<MaterialFlags, MaterialRenderInfo>>({});
            instancedMaterialMap = Construct< std::unordered_map<MaterialFlags, InstancedMaterialRenderInfo>>({});
        }
        void Destruct() {
            materialMap->clear();
            instancedMaterialMap->clear();
            Deconstruct(materialMap);
            Deconstruct(instancedMaterialMap);
        }
        void AddInstancedMaterialObject(MaterialTextureInfo materialInfo, EWEModel* modelPtr, uint32_t entityCount, bool computedTransforms) {
            assert(modelPtr != nullptr);
            auto findRet = instancedMaterialMap->find(materialInfo.materialFlags);
            if (findRet == instancedMaterialMap->end()) {
                auto empRet = instancedMaterialMap->try_emplace(materialInfo.materialFlags, materialInfo.materialFlags, entityCount);
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
            MaterialObjectInfo paramPass{ ownerTransform, modelPtr, drawable };
            AddMaterialObject(materialInfo, paramPass);
        }
        void AddMaterialObjectFromTexID(TextureDesc copyID, TransformComponent* ownerTransform, bool* drawablePtr) {
            //this should probably 

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
#if EWE_DEBUG
            assert(false && "failed to find matching texture");
#else
            //unreachable
#endif
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
#if EWE_DEBUG
            assert(false && "failed to find matching texture");
#else
            //unreachable
#endif
        }
        void RemoveInstancedMaterialObject(EWEModel* meshPtr) {
            for (auto iter = instancedMaterialMap->begin(); iter != instancedMaterialMap->end(); iter++) {
                for (auto instancedIter = iter->second.instancedInfo.begin(); instancedIter != iter->second.instancedInfo.end(); instancedIter++) {
                    if (meshPtr == instancedIter->meshPtr) {
                        iter->second.instancedInfo.erase(instancedIter);
                        if (iter->second.instancedInfo.size() == 0) {
                            instancedMaterialMap->erase(iter);
                        }
                        return;
                    }
                }
            }
#if EWE_DEBUG
            assert(false && "failed to find mesh");
#else
            //unreachable
#endif
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
#if EWE_DEBUG
            if (returnVector.size() == 0) {
                printf("WARNING : failed to remove textures\n");
            }
#endif
            return returnVector;
        }
        void RemoveInstancedObject(EWEModel* modelPtr){
            for (auto iter = instancedMaterialMap->begin(); iter != instancedMaterialMap->end(); iter++) {
                for (auto instanceIter = iter->second.instancedInfo.begin(); instanceIter != iter->second.instancedInfo.end(); instanceIter++) {
                    if (modelPtr == instanceIter->meshPtr) {
                        iter->second.instancedInfo.erase(instanceIter);
                        if (iter->second.instancedInfo.size() == 0) {
                            printf("WARNING: I'm not sure how i want to handle this yet\n");
                            instancedMaterialMap->erase(iter);
                        }
                    }
                }
            }
        }

        void RenderInstancedMemberMethod() {
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
                iter->second.Render();
            }
        }
        void RenderMemberMethod() {

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
                iter->second.Render();


#if DEBUGGING_MATERIAL_PIPE
                printf("finished drawing dynamic material flag : %d \n", flags);
#endif
            }
#if DEBUGGING_PIPELINES || DEBUGGING_MATERIAL_PIPE
            printf("finished dynamic render \n");
#endif
        }

        void Render() {
            //ill replace this shit eventually

            RenderMemberMethod();
            RenderInstancedMemberMethod();
        }

        const EWEBuffer* GetTransformBuffer(EWEModel* meshPtr) {
            for (auto iter = instancedMaterialMap->begin(); iter != instancedMaterialMap->end(); iter++) {
                for (auto const& instanced : iter->second.instancedInfo) {
                    if (meshPtr == instanced.meshPtr) {
                        return instanced.buffer.GetBuffer();
                    }
                }
            }
#if EWE_DEBUG
            assert(false && "failed to find buffer");
#else
            //unreachable
#endif
        }
        const EWEBuffer* GetTransformBuffer(MaterialFlags materialFlags, EWEModel* meshPtr) {
#if EWE_DEBUG
            assert(instancedMaterialMap->contains(materialFlags));
#endif
            auto& ref = instancedMaterialMap->at(materialFlags);
            for (auto const& instanced : ref.instancedInfo) {
                if (meshPtr == instanced.meshPtr) {
                    return instanced.buffer.GetBuffer();
                }
            }
#if EWE_DEBUG
            assert(false && "failed to find buffer");
#else
            //unreachable
#endif
        }

        std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothTransformBuffers(EWEModel* meshPtr) {
            for (auto iter = instancedMaterialMap->begin(); iter != instancedMaterialMap->end(); iter++) {
                for (auto const& instanced : iter->second.instancedInfo) {
                    if (meshPtr == instanced.meshPtr) {
                        return instanced.buffer.GetBothBuffers();
                    }
                }
            }
#if EWE_DEBUG
            assert(false && "failed to find buffer");
#else
            //unreachable
#endif
        }
        std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothTransformBuffers(MaterialFlags materialFlags, EWEModel* meshPtr) {
#if EWE_DEBUG
            assert(instancedMaterialMap->contains(materialFlags));
#endif
            auto& ref = instancedMaterialMap->at(materialFlags);
            for (auto const& instanced : ref.instancedInfo) {
                if (meshPtr == instanced.meshPtr) {
                    return instanced.buffer.GetBothBuffers();
                }
            }
#if EWE_DEBUG
            assert(false && "failed to find buffer");
#else
            //unreachable
#endif
        }
    }//namespace RigidRenderingSystem
} //namespace EWE