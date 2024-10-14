#pragma once

#include "EWEngine/Systems/Rendering/Rigid/RigidBufferHandler.h"
#include "EWEngine/Graphics/Model/Model.h"  
#include "EWEngine/Data/EngineDataTypes.h"

#include "EWEngine/Systems/Rendering/Pipelines/MaterialPipelines.h"

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
            printf("Default construction of material info??? \n");
        }
        MaterialObjectInfo(TransformComponent* tComp, EWEModel* meshP, bool* drawable) : 
            ownerTransform{ tComp },
            meshPtr{ meshP }, 
            drawable{ drawable } 
        {}
    };

    struct MaterialRenderInfo {
        MaterialPipelines* pipe;
        std::unordered_map<TextureDesc, std::vector<MaterialObjectInfo>> materialMap{};
        MaterialRenderInfo(MaterialFlags flags) : pipe{MaterialPipelines::GetMaterialPipe(flags)} {}
        void Render(uint8_t frameIndex);
    };

    struct InstancedMaterialObjectInfo {
        TextureDesc texture;
        EWEModel* meshPtr;
        RigidInstancedBufferHandler buffer;
        InstancedMaterialObjectInfo(TextureDesc texture, EWEModel* meshPtr, uint32_t entityCount, bool computedTransforms) : 
            texture{ texture },
            meshPtr{ meshPtr }, 
            buffer{ entityCount, computedTransforms } 
        {}
    };
    struct InstancedMaterialRenderInfo {
        MaterialPipelines* pipe;
        std::vector<InstancedMaterialObjectInfo> instancedInfo{};
        InstancedMaterialRenderInfo(MaterialFlags flags) : pipe{ MaterialPipelines::GetMaterialPipe(flags | Material::Flags::Instanced) } {}
        void Render(uint8_t frameIndex);
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
        void AddMaterialObject(MaterialTextureInfo materialInfo, MaterialObjectInfo& renderInfo);
        void AddMaterialObject(MaterialTextureInfo materialInfo, TransformComponent* ownerTransform, EWEModel* modelPtr, bool* drawable);
        void AddInstancedMaterialObject(MaterialTextureInfo materialInfo, EWEModel* modelPtr, uint64_t entityCount, bool computedTransforms);

        void AddMaterialObjectFromTexID(TextureDesc copyID, TransformComponent* ownerTransform, bool* drawablePtr);

        void RemoveByTransform(TextureDesc textureID, TransformComponent* ownerTransform);

        std::vector<TextureDesc> CheckAndClearTextures();

        void Render(FrameInfo const& frameInfo);
    };
}