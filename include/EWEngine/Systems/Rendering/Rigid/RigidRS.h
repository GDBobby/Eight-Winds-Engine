#pragma once

#include "EWEngine/Systems/Rendering/Rigid/RigidBufferHandler.h"
#include "EWEngine/Graphics/Model/Model.h"  
#include "EWEngine/Data/EngineDataTypes.h"

#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

#include <array>

//this is still a WIP

namespace EWE {
    struct MaterialObjectInfo {
        TransformComponent* ownerTransform; //if nullptr and not playerOwned, error
        EWEModel* meshPtr;
        bool* drawable;
        //Actor_Type actorType = Actor_None;

        //int32_t textureID;
        MaterialObjectInfo() : 
            ownerTransform{ nullptr },
            meshPtr{ nullptr },
            drawable{ nullptr } 
        {
#if EWE_DEBUG
            printf("Default construction of material info??? \n");
#endif
        }
        MaterialObjectInfo(TransformComponent* tComp, EWEModel* meshP, bool* drawable) : 
            ownerTransform{ tComp },
            meshPtr{ meshP }, 
            drawable{ drawable } 
        {}
    };
    struct MaterialObjectByDesc {
        std::array<VkDescriptorSet, 2> desc{VK_NULL_HANDLE, VK_NULL_HANDLE};
        EWEDescriptorSetLayout* eDSL{ eDSL };
        std::vector<MaterialObjectInfo> objectVec{};
        MaterialObjectByDesc(std::array<VkDescriptorSet, 2> desc, EWEDescriptorSetLayout* eDSL, MaterialObjectInfo& objectVec);
        MaterialObjectByDesc(MaterialObjectByDesc&&) noexcept;
        MaterialObjectByDesc(MaterialObjectByDesc&);
        MaterialObjectByDesc& operator=(MaterialObjectByDesc&&) noexcept;
        MaterialObjectByDesc& operator=(MaterialObjectByDesc&);

        ~MaterialObjectByDesc();
    };

    struct MaterialRenderInfo {
        MaterialPipelines* pipe;
        std::vector<MaterialObjectByDesc> materialVec{};
        MaterialRenderInfo(MaterialFlags flags) : pipe{MaterialPipelines::GetMaterialPipe(flags)} {}
        void Render();
    };

    struct InstancedMaterialObjectInfo {
        EWEModel* meshPtr;
        RigidInstancedBufferHandler buffer;
        EWEDescriptorSetLayout* eDSL{ nullptr };
        std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets{ VK_NULL_HANDLE, VK_NULL_HANDLE };
        //i need to combine the texture and bfufer descriptor into 1
        InstancedMaterialObjectInfo(EWEModel* meshPtr, uint32_t entityCount, bool computedTransforms, EWEDescriptorSetLayout* eDSL, ImageID imageID);
        InstancedMaterialObjectInfo(InstancedMaterialObjectInfo&&) noexcept;
        InstancedMaterialObjectInfo(InstancedMaterialObjectInfo&);
        InstancedMaterialObjectInfo& operator=(InstancedMaterialObjectInfo&);
        InstancedMaterialObjectInfo& operator=(InstancedMaterialObjectInfo&&) noexcept;
        ~InstancedMaterialObjectInfo();
    };
    struct InstancedMaterialRenderInfo {
        MaterialPipelines* pipe;
        std::vector<InstancedMaterialObjectInfo> instancedInfo{};
        InstancedMaterialRenderInfo(MaterialFlags flags, uint32_t entityCount) : pipe{ MaterialPipelines::GetMaterialPipe(flags, entityCount)} {
        }
        void Render();
    };

    //singleton
    namespace RigidRenderingSystem {
        void Initialize();
        void Destruct();
        /*
        const std::unordered_map<MaterialFlags, std::map<TextureID, std::vector<MaterialObjectInfo>>>& getMaterialMap() {
            return materialMap;
        }
        */
        //const std::map<MaterialFlags, std::map<TextureID, std::vector<MaterialObjectInfo>>>& cleanAndGetMaterialMap();
        void AddMaterialObject(MaterialInfo materialInfo, MaterialObjectInfo& renderInfo);
        void AddMaterialObject(MaterialInfo materialInfo, TransformComponent* ownerTransform, EWEModel* modelPtr, bool* drawable);
        void AddInstancedMaterialObject(MaterialInfo materialInfo, EWEModel* modelPtr, uint32_t entityCount, bool computedTransforms);

        void RemoveByTransform(TransformComponent* ownerTransform);
        void RemoveInstancedMaterialObject(EWEModel* modelPtr);

        void Render();

        const EWEBuffer* GetTransformBuffer(EWEModel* meshPtr);
        //providing the materialInfo doesn't need to iterate through every material in the map
        const EWEBuffer* GetTransformBuffer(MaterialFlags materialFlags, EWEModel* meshPtr);

        std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothTransformBuffers(EWEModel* meshPtr);
        std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothTransformBuffers(MaterialFlags materialFlags, EWEModel* meshPtr);

        const EWEBuffer* GetMaterialBuffer(EWEModel* meshPtr);
        //providing the materialInfo doesn't need to iterate through every material in the map
        const EWEBuffer* GetMaterialBuffer(MaterialFlags materialFlags, EWEModel* meshPtr);

        std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothMaterialBuffers(EWEModel* meshPtr);
        std::array<EWEBuffer*, MAX_FRAMES_IN_FLIGHT> GetBothMaterialBuffers(MaterialFlags materialFlags, EWEModel* meshPtr);
    };
}