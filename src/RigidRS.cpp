#include "EWEngine/Systems/Rendering/Rigid/RigidRS.h"

#include "EWEngine/Graphics/PushConstants.h"

namespace EWE {
    void MaterialRenderInfo::Render(uint8_t frameIndex) {
        if (!materialMap.size()) {
            return;
        }
        pipe->BindPipeline();
        pipe->BindDescriptor(0, DescriptorHandler::getDescSet(DS_global, frameIndex));
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

    namespace RigidRenderingSystem {
        std::unordered_map<MaterialFlags, MaterialRenderInfo>* materialMap;


        void Initialize() {
            materialMap = new std::unordered_map<MaterialFlags, MaterialRenderInfo>();
        }
        void Destruct() {
            materialMap->clear();
            delete materialMap;
        }

        void AddMaterialObject(MaterialTextureInfo materialInfo, MaterialObjectInfo& renderInfo) {
#if EWE_DEBUG
            assert(renderInfo.meshPtr != nullptr);
#endif
            if (!materialMap->contains(materialInfo.materialFlags)) {
                auto empRet = materialMap->try_emplace(materialInfo.materialFlags, materialInfo.materialFlags);
                empRet.first->second.materialMap.try_emplace(materialInfo.texture, std::vector<MaterialObjectInfo>{renderInfo});
            }
            else {
                materialMap->at(materialInfo.materialFlags).materialMap.at(materialInfo.texture).push_back(renderInfo);
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

        void Render(FrameInfo const& frameInfo) {
            //ill replace this shit eventually
            MaterialPipelines::SetFrameInfo(frameInfo);

            RenderMemberMethod(frameInfo);
        }
        void RenderMemberMethod(FrameInfo const& frameInfo) {

            for (auto iter = materialMap->begin(); iter != materialMap->end(); iter++) {

#if DEBUGGING_DYNAMIC_PIPE || DEBUGGING_PIPELINES
                printf("checking validity of map iter? \n");
                printf("iter->first:second - %d:%d \n", iter->first, iter->second.size());
                uint8_t flags = iter->first;
                printf("Drawing dynamic materials : %d \n", flags);
                assert(((flags & 128) == 0) && "should not have bones here");
#elif _DEBUG

                uint8_t flags = iter->first;
                assert(((flags & 128) == 0) && "should not have bones here");
#endif
                iter->second.Render(frameInfo.index);


#if DEBUGGING_DYNAMIC_PIPE
                printf("finished drawing dynamic material flag : %d \n", flags);
#endif
            }
#if DEBUGGING_PIPELINES || DEBUGGING_DYNAMIC_PIPE
            printf("finished dynamic render \n");
#endif
        }
    }//namespace RigidRenderingSystem
} //namespace EWE