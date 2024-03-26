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
        MaterialObjectInfo() {
            printf("Default construction of material info??? \n");
            ownerTransform = nullptr;
            meshPtr = nullptr;
            //textureID = 0;
        }
        MaterialObjectInfo(TransformComponent* tComp, EWEModel* meshP, bool* drawable) : ownerTransform{ tComp }, meshPtr{ meshP }, drawable{ drawable } {}
    };

    struct MaterialRenderInfo {
        MaterialPipelines* pipe;
        std::unordered_map<TextureDesc, std::vector<MaterialObjectInfo>> materialMap{};
        MaterialRenderInfo(MaterialFlags flags) : pipe{MaterialPipelines::getMaterialPipe(flags)} {

        }
        void render(uint8_t frameIndex);
    };

    //singleton
    class RigidRenderingSystem {
    private:
        static RigidRenderingSystem* rigidInstance;

    public:
        static RigidRenderingSystem* getRigidRSInstance() {
            //this isn't thread safe
            if (rigidInstance == nullptr) {
                rigidInstance = reinterpret_cast<RigidRenderingSystem*>(ewe_alloc(sizeof(RigidRenderingSystem), 1));
                new(rigidInstance) RigidRenderingSystem();
            }
            return rigidInstance;
        }
        static void destruct() {
            if(rigidInstance){
                rigidInstance->~RigidRenderingSystem();
                ewe_free(rigidInstance);
            }        
        }
    private:
        //RigidRenderingSystem() {}

        RigidRenderingSystem() = default;
        //~RigidRenderingSystem() = default;
        RigidRenderingSystem(const RigidRenderingSystem&) = delete;
        RigidRenderingSystem& operator=(const RigidRenderingSystem&) = delete;

        std::unordered_map<MaterialFlags, MaterialRenderInfo> materialMap{};
    public:
        ~RigidRenderingSystem() {
        }
        /*
        const std::unordered_map<MaterialFlags, std::map<TextureID, std::vector<MaterialObjectInfo>>>& getMaterialMap() {
            return materialMap;
        }
        */
        //const std::map<MaterialFlags, std::map<TextureID, std::vector<MaterialObjectInfo>>>& cleanAndGetMaterialMap();
        void addMaterialObject(MaterialTextureInfo materialInfo, MaterialObjectInfo& renderInfo);
        void addMaterialObject(MaterialTextureInfo materialInfo, TransformComponent* ownerTransform, EWEModel* modelPtr, bool* drawable);

        void addMaterialObjectFromTexID(TextureDesc copyID, TransformComponent* ownerTransform, bool* drawablePtr);

        void removeByTransform(TextureDesc textureID, TransformComponent* ownerTransform);

        std::vector<TextureDesc> checkAndClearTextures();

        static void render(FrameInfo const& frameInfo);
        void renderMemberMethod(FrameInfo const& frameInfo);
    };
}