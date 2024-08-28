#pragma once

#include "EWEngine/Data/EWE_Import.h"
#include "EWEngine/EWE_GameObject.h"
#include "EWEngine/Systems/Rendering/Rigid/RigidRS.h"

#include <unordered_set>

//need instancing here

namespace EWE {

	class EweObject {
	public:

        EweObject(const EweObject& other) : transform{}, ownedTextures{ other.ownedTextures } {
            printf("ewe copy construction \n");
            for (auto iter = ownedTextures.begin(); iter != ownedTextures.end(); iter++) {
                //printf("adding material from tex id %d \n", *iter);
                RigidRenderingSystem::AddMaterialObjectFromTexID(*iter, &transform, &drawable);
            }
        }

        EweObject& operator=(const EweObject&& other) noexcept {
			printf("ewe copy operator \n");
            if (this != &other) {
                this->transform = other.transform;
                this->ownedTextures = other.ownedTextures;
                this->meshes = other.meshes;
                this->drawable = other.drawable;
			}
			return *this;
		}

        EweObject(EweObject&& other) noexcept {
            printf("move operation \n");
            this->transform = other.transform;
            this->ownedTextures = other.ownedTextures;
            this->meshes = other.meshes;
            this->drawable = other.drawable;
        }

        EweObject(std::string objectPath, bool globalTextures, Queue::Enum queue);
        EweObject(std::string objectPath, bool globalTextures, SkeletonID ownerID, Queue::Enum queue);
        ~EweObject();

		TransformComponent transform{};
        std::vector<EWEModel*> meshes{};
        bool drawable = true;
        std::unordered_set<TextureDesc> ownedTextures{};


        //void deTexturize();
        uint32_t GetSkeletonID() {
            return mySkinID;
        }
	private:
        struct TextureMapping {
            std::vector<MaterialTextureInfo> meshNames;
            std::vector<MaterialTextureInfo> meshNTNames;
            std::vector<MaterialTextureInfo> meshSimpleNames;
            std::vector<MaterialTextureInfo> meshNTSimpleNames;
        };

        uint32_t mySkinID = 0;

        void AddToRigidRenderingSystem(ImportData const& tempData, TextureMapping const& textureTracker, Queue::Enum queue);
        void AddToSkinHandler(ImportData& tempData, TextureMapping& textureTracker, uint32_t skeletonOwner, Queue::Enum queue);

        void LoadTextures(std::string objectPath, ImportData::NameExportData& importData, TextureMapping& textureTracker, bool globalTextures);
	
#if DEBUG_NAMING
        void AddDebugNames(std::string const& name);
#endif
    };
}