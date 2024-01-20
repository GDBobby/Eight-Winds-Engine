#pragma once

#include "EWEngine/Graphics/Model/Model.h"  
#include "EWEngine/Data/EngineDataTypes.h"

//this is still a WIP

namespace EWE {
    struct MaterialRenderInfo {
        TransformComponent* ownerTransform; //if nullptr and not playerOwned, error
        EWEModel* meshPtr;
        bool* drawable;
        //Actor_Type actorType = Actor_None;

        //int32_t textureID;
        MaterialRenderInfo() {
            printf("Default construction of material info??? \n");
            ownerTransform = nullptr;
            meshPtr = nullptr;
            //textureID = 0;
        }
        MaterialRenderInfo(TransformComponent* tComp, EWEModel* meshP, bool* drawable) : ownerTransform{ tComp }, meshPtr{ meshP }, drawable{ drawable } {}
    };


    //singleton
    class MaterialHandler {
    public:
        static std::shared_ptr<MaterialHandler> getMaterialHandlerInstance() {
            static std::shared_ptr<MaterialHandler> materialHandlerInstance{ new MaterialHandler };
            return materialHandlerInstance;
        }
    private:
        //MaterialHandler() {}

        MaterialHandler() = default;
        //~MaterialHandler() = default;
        MaterialHandler(const MaterialHandler&) = delete;
        MaterialHandler& operator=(const MaterialHandler&) = delete;

        std::map<MaterialFlags, std::map<TextureID, std::vector<MaterialRenderInfo>>> materialMap;
    public:
        ~MaterialHandler() {}
        const std::map<MaterialFlags, std::map<TextureID, std::vector<MaterialRenderInfo>>>& getMaterialMap() {
            return materialMap;
        }
        const std::map<MaterialFlags, std::map<TextureID, std::vector<MaterialRenderInfo>>>& cleanAndGetMaterialMap();
        void addMaterialObject(MaterialTextureInfo materialInfo, MaterialRenderInfo& renderInfo);
        void addMaterialObject(MaterialTextureInfo materialInfo, TransformComponent* ownerTransform, EWEModel* modelPtr, bool* drawable);

        void addMaterialObjectFromTexID(TextureID copyID, TransformComponent* ownerTransform, bool* drawablePtr);

        void removeByTransform(TextureID textureID, TransformComponent* ownerTransform);

        std::vector<TextureID> checkAndClearTextures();
    };
}