#include "EWEngine/Systems/Rendering/Rigid/RigidRS.h"

#include "EWEngine/Graphics/PushConstants.h"
#include "EWEngine/Graphics/Texture/Image_Manager.h"

namespace EWE {
    std::unordered_map<ImageID, std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>> descriptorsByImageInfo{};

    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> GetDescriptor(MaterialInfo materialInfo) {
        auto find = descriptorsByImageInfo.find(materialInfo.imageID);
        if (find != descriptorsByImageInfo.end()) {
            return find->second;
        }
        else {
            EWEDescriptorSetLayout* eDSL = MaterialPipelines::GetDSLFromFlags(materialInfo.materialFlags);
            std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> ret;
            for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                EWEDescriptorWriter descWriter{ eDSL, DescriptorPool_Global };
                DescriptorHandler::AddGlobalsToDescriptor(descWriter, i);
                descWriter.WriteImage(2, Image_Manager::GetDescriptorImageInfo(materialInfo.imageID));
                ret[i] = descWriter.Build();
            }
#if DEBUG_NAMING
            DebugNaming::SetObjectName(ret[0], VK_OBJECT_TYPE_DESCRIPTOR_SET, "rigid buffer desc[0]");
            DebugNaming::SetObjectName(ret[1], VK_OBJECT_TYPE_DESCRIPTOR_SET, "rigid buffer desc[1]");
#endif


            auto empRet = descriptorsByImageInfo.try_emplace(materialInfo.imageID, ret);
            return ret;
        }
    }
    InstancedMaterialObjectInfo::InstancedMaterialObjectInfo(EWEModel* meshPtr, uint32_t entityCount, bool computedTransforms, EWEDescriptorSetLayout* eDSL, ImageID imageID) :
        meshPtr{ meshPtr },
        buffer{ entityCount, computedTransforms, eDSL, imageID },
        descriptorSets{
                EWEDescriptorWriter(eDSL, DescriptorPool_Global)
                .WriteBuffer(0, DescriptorHandler::GetCameraDescriptorBufferInfo(0))
                .WriteBuffer(1, DescriptorHandler::GetLightingDescriptorBufferInfo(0))
                .WriteBuffer(2, buffer.GetDescriptorBufferInfo(0))
                .WriteImage(3, Image_Manager::GetDescriptorImageInfo(imageID))
                .Build()
            ,
                EWEDescriptorWriter(eDSL, DescriptorPool_Global)
                .WriteBuffer(0, DescriptorHandler::GetCameraDescriptorBufferInfo(1))
                .WriteBuffer(1, DescriptorHandler::GetLightingDescriptorBufferInfo(1))
                .WriteBuffer(2, buffer.GetDescriptorBufferInfo(1))
                .WriteImage(3, Image_Manager::GetDescriptorImageInfo(imageID))
                .Build()

        }
    {
#if DEBUG_NAMING
        DebugNaming::SetObjectName(descriptorSets[0], VK_OBJECT_TYPE_DESCRIPTOR_SET, "rigid instanced descriptor[0]");
        DebugNaming::SetObjectName(descriptorSets[0], VK_OBJECT_TYPE_DESCRIPTOR_SET, "rigid instanced descriptor[1]");
#endif
    }

    void MaterialRenderInfo::Render() {
        if (materialVec.size() == 0) {
            return;
        }
        pipe->BindPipeline();
        for (auto& material : materialVec) {
            if (material.objectVec.size() == 0) {
                continue;
            }
#if EWE_DEBUG
            assert(material.desc[VK::Object->frameIndex] != VK_NULL_HANDLE);
#endif
            pipe->BindDescriptor(0, &material.desc[VK::Object->frameIndex]);
            for (auto& obj : material.objectVec) {
                if (!obj.drawable) {
                    continue;
                }
                SimplePushConstantData push{ obj.ownerTransform->mat4(), obj.ownerTransform->normalMatrix() };

                pipe->BindModel(obj.meshPtr);
                pipe->PushAndDraw(&push);
            }
        }
    }
    void InstancedMaterialRenderInfo::Render() {
        if (instancedInfo.size() == 0) { return; }
        pipe->BindPipeline();

        //pipe->BindDescriptor(0, DescriptorHandler::GetDescSet(DS_global));
        for (auto const& instanceInfo : instancedInfo) {
           // pipe->BindDescriptor(1, instanceInfo.buffer.GetDescriptor());
            //pipe->BindTextureDescriptor(2, instanceInfo.texture);
            pipe->BindDescriptor(0, &instanceInfo.descriptorSets[VK::Object->frameIndex]);
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
            instancedMaterialMap = Construct<std::unordered_map<MaterialFlags, InstancedMaterialRenderInfo>>({});
        }
        void Destruct() {
            materialMap->clear();
            instancedMaterialMap->clear();
            Deconstruct(materialMap);
            Deconstruct(instancedMaterialMap);
        }
        void AddInstancedMaterialObject(MaterialInfo materialInfo, EWEModel* modelPtr, uint32_t entityCount, bool computedTransforms) {

            assert(modelPtr != nullptr);
            const uint16_t pipeLayoutIndex = MaterialPipelines::GetPipeLayoutIndex(materialInfo.materialFlags);
            auto findRet = instancedMaterialMap->find(materialInfo.materialFlags);
            if (findRet == instancedMaterialMap->end()) {
                auto empRet = instancedMaterialMap->try_emplace(materialInfo.materialFlags, materialInfo.materialFlags, entityCount);
                empRet.first->second.instancedInfo.emplace_back(modelPtr, entityCount, computedTransforms, MaterialPipelines::GetDSL(pipeLayoutIndex), materialInfo.imageID);
            }
            else {
                findRet->second.instancedInfo.emplace_back(modelPtr, entityCount, computedTransforms, MaterialPipelines::GetDSL(pipeLayoutIndex), materialInfo.imageID);
            }
        }

        void AddMaterialObject(MaterialInfo materialInfo, MaterialObjectInfo& renderInfo) {
        #if EWE_DEBUG
            assert(renderInfo.meshPtr != nullptr);
        #endif
            auto findRet = materialMap->find(materialInfo.materialFlags);
            if (findRet == materialMap->end()) {
                auto empRet = materialMap->try_emplace(materialInfo.materialFlags, materialInfo.materialFlags);

                //need to create the descriptor here
                std::array<VkDescriptorSet, 2> desc = GetDescriptor(materialInfo);
                for (auto& material : empRet.first->second.materialVec) {
                    if (material.desc == desc) {
                        material.objectVec.push_back(renderInfo);
                        break;
                    }
                }
            }
            else {
                std::array<VkDescriptorSet, 2> desc = GetDescriptor(materialInfo);
                for (auto& material : findRet->second.materialVec) {
                    if (material.desc == desc) {
                        material.objectVec.push_back(renderInfo);
                        break;
                    }
                }
            }
        }
        void AddMaterialObject(MaterialInfo materialInfo, TransformComponent* ownerTransform, EWEModel* modelPtr, bool* drawable) {
            MaterialObjectInfo paramPass{ ownerTransform, modelPtr, drawable };
            AddMaterialObject(materialInfo, paramPass);
        }
        void RemoveByTransform(TransformComponent* ownerTransform) {
            for (auto iter = materialMap->begin(); iter != materialMap->end(); iter++) {
                for (auto& material : iter->second.materialVec) {
                    for (auto iter = material.objectVec.begin(); iter != material.objectVec.end(); iter++) {
                        if (iter->ownerTransform == ownerTransform) {
                            material.objectVec.erase(iter);
                            return;
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
            return nullptr; // error silencing
#else
            //unreachable
            __assume(false);
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
            __assume(false);
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
            __assume(false);
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
            __assume(false);
#endif
        }
    }//namespace RigidRenderingSystem
} //namespace EWE