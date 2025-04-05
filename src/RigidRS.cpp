#include "EWEngine/Systems/Rendering/Rigid/RigidRS.h"

#include "EWEngine/Graphics/PushConstants.h"
#include "EWEngine/Graphics/Texture/Image_Manager.h"

namespace EWE {
    std::unordered_map<ImageID, std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>> descriptorsByImageInfo{};

    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> CreateDescriptor(EWEDescriptorSetLayout* eDSL, MaterialInfo materialInfo, EWEBuffer* materialBuffer) {

        std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> ret;
        for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            EWEDescriptorWriter descWriter{ eDSL, DescriptorPool_Global };
            DescriptorHandler::AddGlobalsToDescriptor(descWriter, i);

            descWriter.WriteBuffer(materialBuffer->DescriptorInfo());

            //need to get a full texture count here
            if (materialInfo.materialFlags & Material::Flags::Texture::Albedo) {
                descWriter.WriteImage(materialInfo.imageID);
            }
            ret[i] = descWriter.Build();
        }
#if DEBUG_NAMING
        DebugNaming::SetObjectName(ret[0], VK_OBJECT_TYPE_DESCRIPTOR_SET, "rigid buffer desc[0]");
        DebugNaming::SetObjectName(ret[1], VK_OBJECT_TYPE_DESCRIPTOR_SET, "rigid buffer desc[1]");
#endif
        if (materialInfo.imageID != IMAGE_INVALID) {
#if EWE_DEBUG
            assert(descriptorsByImageInfo.try_emplace(materialInfo.imageID, ret).second);
#else
            descriptorsByImageInfo.try_emplace(materialInfo.imageID, ret);
#endif
        }
        return ret;
    }
    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> CreateDescriptor(EWEDescriptorSetLayout* eDSL, MaterialInfo materialInfo, std::array<EWEBuffer*, 2> materialBuffer) {

        std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> ret;
        for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            EWEDescriptorWriter descWriter{ eDSL, DescriptorPool_Global };
            DescriptorHandler::AddGlobalsToDescriptor(descWriter, i);
#if DEBUGGING_MATERIAL_NORMALS
            if ((materialInfo.materialFlags & Material::Flags::GenerateNormals) == 0) {
#endif
                descWriter.WriteBuffer(materialBuffer[i]->DescriptorInfo());

                //need to get a full texture count here
                if (materialInfo.materialFlags & Material::Flags::Texture::Albedo) {
                    descWriter.WriteImage(materialInfo.imageID);
                }
#if DEBUGGING_MATERIAL_NORMALS
            }
            else if (materialInfo.materialFlags & Material::Flags::Bump) {
                descWriter.WriteImage(materialInfo.imageID);
            }
#endif
            ret[i] = descWriter.Build();
        }
#if DEBUG_NAMING
        DebugNaming::SetObjectName(ret[0], VK_OBJECT_TYPE_DESCRIPTOR_SET, "rigid buffer desc[0]");
        DebugNaming::SetObjectName(ret[1], VK_OBJECT_TYPE_DESCRIPTOR_SET, "rigid buffer desc[1]");
#endif
        if (materialInfo.imageID != IMAGE_INVALID) {
#if EWE_DEBUG
            assert(descriptorsByImageInfo.try_emplace(materialInfo.imageID, ret).second);
#else
            descriptorsByImageInfo.try_emplace(materialInfo.imageID, ret);
#endif
        }
        return ret;
    }

    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> GetDescriptor(EWEDescriptorSetLayout* eDSL, MaterialInfo materialInfo, EWEBuffer* materialBuffer) {
        auto find = descriptorsByImageInfo.find(materialInfo.imageID);
        if (find != descriptorsByImageInfo.end()) {
            return find->second;
        }
        else {
#if EWE_DEBUG
            printf("WARNING: creating descriptor in get descriptor\n");
            //potentially force a breakpoint here in debug mode?
#endif
            return CreateDescriptor(eDSL, materialInfo, materialBuffer);
        }
    }
    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> GetDescriptor(EWEDescriptorSetLayout* eDSL, MaterialInfo materialInfo, std::array<EWEBuffer*, 2> materialBuffer) {
        auto find = descriptorsByImageInfo.find(materialInfo.imageID);
        if (find != descriptorsByImageInfo.end()) {
            return find->second;
        }
        else {
#if EWE_DEBUG
            printf("WARNING: creating descriptor in get descriptor\n");
            //potentially force a breakpoint here in debug mode?
#endif
            return CreateDescriptor(eDSL, materialInfo, materialBuffer);
        }
    }
    void RemoveDescriptor(VkDescriptorSet firstDescriptor) {

        for (auto iter = descriptorsByImageInfo.begin(); iter != descriptorsByImageInfo.end(); iter++) {
            if (iter->second[0] == firstDescriptor) {
                Image_Manager::RemoveImage(iter->first);
                //potentially free the descriptors here, but right now im freeing them outside this function
                descriptorsByImageInfo.erase(iter);
                return;
            }
        }
        EWE_UNREACHABLE;
    }


    MaterialObjectByDesc::MaterialObjectByDesc(std::array<VkDescriptorSet, 2> desc, EWEDescriptorSetLayout* eDSL, MaterialObjectInfo& obj)
        : desc{desc},
        eDSL{eDSL},
        objectVec{obj}
    {
    }
    MaterialObjectByDesc::MaterialObjectByDesc(MaterialObjectByDesc&& other) noexcept
        :desc{ other.desc },
        eDSL{ other.eDSL },
        objectVec{ other.objectVec }
    {
        other.desc[0] = VK_NULL_HANDLE;
        other.desc[1] = VK_NULL_HANDLE;
    }
    MaterialObjectByDesc::MaterialObjectByDesc(MaterialObjectByDesc& other)
        :desc{ other.desc },
        eDSL{ other.eDSL },
        objectVec{ other.objectVec }
    {
        other.desc[0] = VK_NULL_HANDLE;
        other.desc[1] = VK_NULL_HANDLE;
    }
    MaterialObjectByDesc& MaterialObjectByDesc::operator=(MaterialObjectByDesc&& other) noexcept {
        desc = other.desc;
        eDSL = other.eDSL;
        objectVec = other.objectVec;
        other.desc[0] = VK_NULL_HANDLE;
        other.desc[1] = VK_NULL_HANDLE;
        return *this;
    }
    MaterialObjectByDesc& MaterialObjectByDesc::operator=(MaterialObjectByDesc& other) {
        desc = other.desc;
        eDSL = other.eDSL;
        objectVec = other.objectVec;
        other.desc[0] = VK_NULL_HANDLE;
        other.desc[1] = VK_NULL_HANDLE;
        return *this;
    }


    MaterialObjectByDesc::~MaterialObjectByDesc() {

        if (desc[0] != VK_NULL_HANDLE) {
            RemoveDescriptor(desc[0]);
            assert(eDSL != nullptr);
            EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, eDSL, &desc[0]);

            if (desc[1] != VK_NULL_HANDLE) {
                assert(eDSL != nullptr);
                EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, eDSL, &desc[1]);
            }
        }
        else {
            assert(desc[1] == VK_NULL_HANDLE && "not allowed to define the second desc only");
        }
    }


    InstancedMaterialObjectInfo::InstancedMaterialObjectInfo(EWEModel* meshPtr, uint32_t entityCount, bool computedTransforms, EWEDescriptorSetLayout* eDSL, ImageID imageID) :
        meshPtr{ meshPtr },
        buffer{ entityCount, computedTransforms, eDSL, imageID },
        eDSL{ eDSL }
    {
        for (uint8_t i = 0; i < 2; i++) {
            EWEDescriptorWriter descWriter(eDSL, DescriptorPool_Global);
            descWriter.WriteBuffer(DescriptorHandler::GetCameraDescriptorBufferInfo(i));
            descWriter.WriteBuffer(DescriptorHandler::GetLightingDescriptorBufferInfo(i));
            descWriter.WriteBuffer(buffer.GetTransformDescriptorBufferInfo(i)); //instancing
            auto const& bindings = eDSL->GetBindings();
            if (bindings.size() > 3) {
                descWriter.WriteBuffer(buffer.GetMaterialDescriptorBufferInfo(i)); //instancing

                if (bindings.size() > 4) {
                    descWriter.WriteImage(imageID);
                }
            }
            descriptorSets[i] = descWriter.Build();
        }
#if DEBUG_NAMING
        DebugNaming::SetObjectName(descriptorSets[0], VK_OBJECT_TYPE_DESCRIPTOR_SET, "rigid instanced descriptor[0]");
        DebugNaming::SetObjectName(descriptorSets[0], VK_OBJECT_TYPE_DESCRIPTOR_SET, "rigid instanced descriptor[1]");
#endif
    }
    InstancedMaterialObjectInfo::~InstancedMaterialObjectInfo() {
        if (descriptorSets[0] != VK_NULL_HANDLE) {
            assert(eDSL != nullptr);
            EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, eDSL, &descriptorSets[0]);
        }
        if (descriptorSets[1] != VK_NULL_HANDLE) {
            assert(eDSL != nullptr);
            EWEDescriptorPool::FreeDescriptor(DescriptorPool_Global, eDSL, &descriptorSets[1]);
        }
    }


    InstancedMaterialObjectInfo::InstancedMaterialObjectInfo(InstancedMaterialObjectInfo&& other) noexcept
        : descriptorSets{ other.descriptorSets },
        buffer{ other.buffer },
        meshPtr{ other.meshPtr }
    {
        other.descriptorSets[0] = VK_NULL_HANDLE;
        other.descriptorSets[1] = VK_NULL_HANDLE;

        other.meshPtr = nullptr;
    }
    InstancedMaterialObjectInfo::InstancedMaterialObjectInfo(InstancedMaterialObjectInfo& other) 
        : descriptorSets{ other.descriptorSets },
        buffer{other.buffer},
        meshPtr{other.meshPtr}
    {
        other.descriptorSets[0] = VK_NULL_HANDLE;
        other.descriptorSets[1] = VK_NULL_HANDLE;
        other.meshPtr = nullptr;
    }
    InstancedMaterialObjectInfo& InstancedMaterialObjectInfo::operator=(InstancedMaterialObjectInfo& other) {
        descriptorSets = other.descriptorSets;
        other.descriptorSets[0] = VK_NULL_HANDLE;
        other.descriptorSets[1] = VK_NULL_HANDLE;

        meshPtr = other.meshPtr;
        other.meshPtr = nullptr;
        return *this;
    }
    InstancedMaterialObjectInfo& InstancedMaterialObjectInfo::operator=(InstancedMaterialObjectInfo&& other) noexcept {
        descriptorSets = other.descriptorSets;
        other.descriptorSets[0] = VK_NULL_HANDLE;
        other.descriptorSets[1] = VK_NULL_HANDLE;

        meshPtr = other.meshPtr;
        other.meshPtr = nullptr;
        return *this;
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

            ModelAndNormalPushData push;
            for (auto& obj : material.objectVec) {
                if (!obj.drawable) {
                    continue;
                }
                push.modelMatrix = obj.ownerTransform->mat4();
                push.normalMatrix = obj.ownerTransform->normalMatrix();

                pipe->BindModel(obj.meshPtr);
                pipe->PushAndDraw(&push);
            }
        }
    }
    void InstancedMaterialRenderInfo::Render() {
        if (instancedInfo.size() == 0) { return; }
        pipe->BindPipeline();

        for (auto const& instanceInfo : instancedInfo) {
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
            const uint16_t pipeLayoutIndex = Material::GetPipeLayoutIndex(materialInfo.materialFlags);
            auto findRet = instancedMaterialMap->find(materialInfo.materialFlags);
            if (findRet == instancedMaterialMap->end()) {
                auto empRet = instancedMaterialMap->try_emplace(materialInfo.materialFlags, materialInfo.materialFlags, entityCount);
                empRet.first->second.instancedInfo.emplace_back(modelPtr, entityCount, computedTransforms, MaterialPipelines::GetDSL(pipeLayoutIndex), materialInfo.imageID);
            }
            else {
                findRet->second.instancedInfo.emplace_back(modelPtr, entityCount, computedTransforms, MaterialPipelines::GetDSL(pipeLayoutIndex), materialInfo.imageID);
            }
        }

        void AddMaterialObject(MaterialInfo materialInfo, MaterialObjectInfo& renderInfo, EWEBuffer* materialBuffer) {
            assert(renderInfo.meshPtr != nullptr);


            auto findRet = materialMap->find(materialInfo.materialFlags);
            if (findRet == materialMap->end()) {
                auto empRet = materialMap->try_emplace(materialInfo.materialFlags, materialInfo.materialFlags);

                //need to create the descriptor here
                assert(!descriptorsByImageInfo.contains(materialInfo.imageID)); //somethign wasn't cleaned correctly
                EWEDescriptorSetLayout* eDSL = MaterialPipelines::GetDSLFromFlags(materialInfo.materialFlags);
                std::array<VkDescriptorSet, 2> desc = CreateDescriptor(eDSL, materialInfo, materialBuffer);
                empRet.first->second.materialVec.emplace_back(desc, eDSL, renderInfo);
            }
            else {
                if (descriptorsByImageInfo.contains(materialInfo.imageID)) {
                    EWEDescriptorSetLayout* eDSL = MaterialPipelines::GetDSLFromFlags(materialInfo.materialFlags);
                    std::array<VkDescriptorSet, 2> desc = GetDescriptor(eDSL, materialInfo, materialBuffer);
#if EWE_DEBUG
                    bool foundMatch = false;
#endif
                    for (auto& material : findRet->second.materialVec) {
                        if (material.desc == desc) {
                            material.objectVec.push_back(renderInfo);
#if EWE_DEBUG
                            foundMatch = true;
#endif
                            break;
                        }
                    }
#if EWE_DEBUG
                    assert(foundMatch); //something was cleaned incorrectly
#endif
                }
                else {
                    EWEDescriptorSetLayout* eDSL = MaterialPipelines::GetDSLFromFlags(materialInfo.materialFlags);
                    assert(!descriptorsByImageInfo.contains(materialInfo.imageID)); //somethign wasn't cleaned correctly
                    std::array<VkDescriptorSet, 2> desc = CreateDescriptor(eDSL, materialInfo, materialBuffer); //this should return a new descriptor
                    findRet->second.materialVec.emplace_back(desc, eDSL, renderInfo);
                }
            }
        }
        void AddMaterialObject(MaterialInfo materialInfo, MaterialObjectInfo& renderInfo, std::array<EWEBuffer*, 2> materialBuffer) {
            assert(renderInfo.meshPtr != nullptr);


            auto findRet = materialMap->find(materialInfo.materialFlags);
            if (findRet == materialMap->end()) {
                auto empRet = materialMap->try_emplace(materialInfo.materialFlags, materialInfo.materialFlags);

                //need to create the descriptor here
                assert(!descriptorsByImageInfo.contains(materialInfo.imageID)); //somethign wasn't cleaned correctly
                EWEDescriptorSetLayout* eDSL = MaterialPipelines::GetDSLFromFlags(materialInfo.materialFlags);
                std::array<VkDescriptorSet, 2> desc = CreateDescriptor(eDSL, materialInfo, materialBuffer);
                empRet.first->second.materialVec.emplace_back(desc, eDSL, renderInfo);
            }
            else {
                if (descriptorsByImageInfo.contains(materialInfo.imageID)) {
                    EWEDescriptorSetLayout* eDSL = MaterialPipelines::GetDSLFromFlags(materialInfo.materialFlags);
                    std::array<VkDescriptorSet, 2> desc = GetDescriptor(eDSL, materialInfo, materialBuffer);
#if EWE_DEBUG
                    bool foundMatch = false;
#endif
                    for (auto& material : findRet->second.materialVec) {
                        if (material.desc == desc) {
                            material.objectVec.push_back(renderInfo);
#if EWE_DEBUG
                            foundMatch = true;
#endif
                            break;
                        }
                    }
#if EWE_DEBUG
                    assert(foundMatch); //something was cleaned incorrectly
#endif
                }
                else {
                    EWEDescriptorSetLayout* eDSL = MaterialPipelines::GetDSLFromFlags(materialInfo.materialFlags);
                    assert(!descriptorsByImageInfo.contains(materialInfo.imageID)); //somethign wasn't cleaned correctly
                    std::array<VkDescriptorSet, 2> desc = CreateDescriptor(eDSL, materialInfo, materialBuffer); //this should return a new descriptor
                    findRet->second.materialVec.emplace_back(desc, eDSL, renderInfo);
                }
            }
        }
        void AddMaterialObject(MaterialInfo materialInfo, TransformComponent* ownerTransform, EWEModel* modelPtr, bool* drawable, EWEBuffer* materialBuffer) {
            MaterialObjectInfo paramPass{ ownerTransform, modelPtr, drawable };
            assert((modelPtr != nullptr) && (ownerTransform != nullptr) && (drawable != nullptr));
            AddMaterialObject(materialInfo, paramPass, materialBuffer);
        }
        void AddMaterialObject(MaterialInfo materialInfo, TransformComponent* ownerTransform, EWEModel* modelPtr, bool* drawable, std::array<EWEBuffer*, 2> materialBuffer) {
            MaterialObjectInfo paramPass{ ownerTransform, modelPtr, drawable };
            assert((modelPtr != nullptr) && (ownerTransform != nullptr) && (drawable != nullptr));
            AddMaterialObject(materialInfo, paramPass, materialBuffer);
        }
        void RemoveByTransform(TransformComponent* ownerTransform) {

            for (auto& mat : *materialMap) {
                for (auto& material : mat.second.materialVec) {
                    for (auto objIter = material.objectVec.begin(); objIter != material.objectVec.end(); objIter++) {
                        if (objIter->ownerTransform == ownerTransform) {
                            material.objectVec.erase(objIter);
                            return;
                        }
                    }
                }
            }
            EWE_UNREACHABLE;
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
                assert(((flags & Material::Bones) == 0) && "should not have bones here");
#elif EWE_DEBUG

                uint8_t flags = iter->first;
                assert(((flags & Material::Flags::Other::Bones) == 0) && "should not have bones here");
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
                assert(((flags & Material::Flags::Other::Bones) == 0) && "should not have bones here");
#elif EWE_DEBUG

                uint8_t flags = iter->first;
                assert(((flags & Material::Flags::Other::Bones) == 0) && "should not have bones here");
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
                        return instanced.buffer.GetTransformBuffer();
                    }
                }
            }
            EWE_UNREACHABLE;
        }
        const EWEBuffer* GetTransformBuffer(MaterialFlags materialFlags, EWEModel* meshPtr) {
#if EWE_DEBUG
            assert(instancedMaterialMap->contains(materialFlags));
#endif
            auto& ref = instancedMaterialMap->at(materialFlags);
            for (auto const& instanced : ref.instancedInfo) {
                if (meshPtr == instanced.meshPtr) {
                    return instanced.buffer.GetTransformBuffer();
                }
            }
            EWE_UNREACHABLE;
        }
        std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothTransformBuffers(EWEModel* meshPtr) {
            for (auto iter = instancedMaterialMap->begin(); iter != instancedMaterialMap->end(); iter++) {
                for (auto const& instanced : iter->second.instancedInfo) {
                    if (meshPtr == instanced.meshPtr) {
                        return instanced.buffer.GetBothTransformBuffers();
                    }
                }
            }
            EWE_UNREACHABLE;
        }
        std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothTransformBuffers(MaterialFlags materialFlags, EWEModel* meshPtr) {
#if EWE_DEBUG
            assert(instancedMaterialMap->contains(materialFlags));
#endif
            auto& ref = instancedMaterialMap->at(materialFlags);
            for (auto const& instanced : ref.instancedInfo) {
                if (meshPtr == instanced.meshPtr) {
                    return instanced.buffer.GetBothTransformBuffers();
                }
            }
            EWE_UNREACHABLE;
        }

        const EWEBuffer* GetMaterialBuffer(EWEModel* meshPtr) {
            for (auto iter = instancedMaterialMap->begin(); iter != instancedMaterialMap->end(); iter++) {
                for (auto const& instanced : iter->second.instancedInfo) {
                    if (meshPtr == instanced.meshPtr) {
                        return instanced.buffer.GetMaterialBuffer();
                    }
                }
            }
            EWE_UNREACHABLE;
        }
        const EWEBuffer* GetMaterialBuffer(MaterialFlags materialFlags, EWEModel* meshPtr) {
#if EWE_DEBUG
            assert(instancedMaterialMap->contains(materialFlags));
#endif
            auto& ref = instancedMaterialMap->at(materialFlags);
            for (auto const& instanced : ref.instancedInfo) {
                if (meshPtr == instanced.meshPtr) {
                    return instanced.buffer.GetMaterialBuffer();
                }
            }
            EWE_UNREACHABLE;
        }
        std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothMaterialBuffers(EWEModel* meshPtr) {
            for (auto iter = instancedMaterialMap->begin(); iter != instancedMaterialMap->end(); iter++) {
                for (auto const& instanced : iter->second.instancedInfo) {
                    if (meshPtr == instanced.meshPtr) {
                        return instanced.buffer.GetBothMaterialBuffers();
                    }
                }
            }
            EWE_UNREACHABLE;
        }
        std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothMaterialBuffers(MaterialFlags materialFlags, EWEModel* meshPtr) {
#if EWE_DEBUG
            assert(instancedMaterialMap->contains(materialFlags));
#endif
            auto& ref = instancedMaterialMap->at(materialFlags);
            for (auto const& instanced : ref.instancedInfo) {
                if (meshPtr == instanced.meshPtr) {
                    return instanced.buffer.GetBothMaterialBuffers();
                }
            }
            EWE_UNREACHABLE;
        }
    }//namespace RigidRenderingSystem
} //namespace EWE