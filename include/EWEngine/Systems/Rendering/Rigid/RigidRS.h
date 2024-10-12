#pragma once

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
    struct MaterialRenderInfoInstanced {
        MaterialPipelines* pipe;
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

        void AddMaterialObjectFromTexID(TextureDesc copyID, TransformComponent* ownerTransform, bool* drawablePtr);

        void RemoveByTransform(TextureDesc textureID, TransformComponent* ownerTransform);

        std::vector<TextureDesc> CheckAndClearTextures();

        void Render(FrameInfo const& frameInfo);
        void RenderMemberMethod(FrameInfo const& frameInfo);
    };
}