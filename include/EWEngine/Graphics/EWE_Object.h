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
            auto matInstance = RigidRenderingSystem::getRigidRSInstance();
            for (auto iter = ownedTextures.begin(); iter != ownedTextures.end(); iter++) {
                //printf("adding material from tex id %d \n", *iter);
                matInstance->addMaterialObjectFromTexID(*iter, &transform, &drawable);
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

        EweObject(std::string objectPath, EWEDevice& device, bool globalTextures);
        EweObject(std::string objectPath, EWEDevice& device, bool globalTextures, SkeletonID ownerID);
        ~EweObject();

		TransformComponent transform{};
        std::vector<std::shared_ptr<EWEModel>> meshes{};
        bool drawable = true;
        std::unordered_set<TextureDesc> ownedTextures{};


        //void deTexturize();
        uint32_t getSkeletonID() {
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

        void addToRigidRenderingSystem(EWEDevice& device, ImportData& tempData, TextureMapping& textureTracker);
        void addToSkinHandler(EWEDevice& device, ImportData& tempData, TextureMapping& textureTracker, uint32_t skeletonOwner);

        void loadTextures(EWEDevice& device, std::string objectPath, ImportData::NameExportData& importData, TextureMapping& textureTracker, bool globalTextures);
	};
}