#pragma once

#include "EWEngine/graphics/model/EWE_Model.h"  
#include "EWEngine/Data/EngineDataTypes.h"

//this is still a WIP

namespace EWE {
    struct MaterialInfo {
        TransformComponent* ownerTransform; //if nullptr and not playerOwned, error
        EWEModel* meshPtr;
        bool* drawable;
        //Actor_Type actorType = Actor_None;

        //int32_t textureID;
        MaterialInfo() {
            printf("Default construction of material info??? \n");
            ownerTransform = nullptr;
            meshPtr = nullptr;
            //textureID = 0;
        }
        MaterialInfo(TransformComponent* tComp, EWEModel* meshP, bool* drawable) : ownerTransform{ tComp }, meshPtr{ meshP }, drawable{ drawable } {}
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

        std::map<ShaderFlags, std::map<TextureID, std::vector<MaterialInfo>>> materialMap;
    public:
        ~MaterialHandler() {}
        const std::map<ShaderFlags, std::map<TextureID, std::vector<MaterialInfo>>>& getMaterialMap() {
            return materialMap;
        }
        const std::map<ShaderFlags, std::map<TextureID, std::vector<MaterialInfo>>>& cleanAndGetMaterialMap();
        void addMaterialObject(ShaderFlags flags, TextureID textureID, MaterialInfo& materialInfo);
        void addMaterialObject(ShaderFlags flags, TransformComponent* ownerTransform, EWEModel* modelPtr, uint32_t textureID, bool* drawable);

        void addMaterialObjectFromTexID(TextureID copyID, TransformComponent* ownerTransform, bool* drawablePtr);

        void removeByTransform(TextureID textureID, TransformComponent* ownerTransform);

        std::vector<TextureID> checkAndClearTextures();
    };
}