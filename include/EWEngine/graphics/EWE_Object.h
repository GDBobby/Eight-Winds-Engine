#pragma once

#include "../Data/EWEImport.h"
#include "../EWE_GameObject.h"
#include "EWE_Texture.h"
#include "../systems/RigidRendering/RigidRenderSystem.h"

namespace EWE {
    struct TextureMapping {
        std::vector<std::pair<ShaderFlags, TextureID>> meshNames;
        std::vector<std::pair<ShaderFlags, TextureID>> meshNTNames;
        std::vector<std::pair<ShaderFlags, TextureID>> meshSimpleNames;
        std::vector<std::pair<ShaderFlags, TextureID>> meshNTSimpleNames;
    };

	class EweObject {
	public:
        EweObject(const EweObject& other) : transform{}, ownedTextureIDs{ other.ownedTextureIDs } {
            printf("ewe copy construction \n");
            auto matInstance = MaterialHandler::getMaterialHandlerInstance();
            for (auto iter = ownedTextureIDs.begin(); iter != ownedTextureIDs.end(); iter++) {
                //printf("adding material from tex id %d \n", *iter);
                matInstance->addMaterialObjectFromTexID(*iter, &transform);
            }
        }
        EweObject& operator=(const EweObject&& other) noexcept {
			printf("ewe copy operator \n");
            if (this != &other) {
                this->transform = other.transform;
                this->ownedTextureIDs = other.ownedTextureIDs;
                this->meshes = other.meshes;
                this->drawable = other.drawable;
			}
			return *this;
		}
        EweObject(EweObject&& other) noexcept {
            printf("move operation \n");
            this->transform = other.transform;
            this->ownedTextureIDs = other.ownedTextureIDs;
            this->meshes = other.meshes;
            this->drawable = other.drawable;
        }

        EweObject(std::string objectPath, EWEDevice& device, bool globalTextures);
        EweObject(std::string objectPath, EWEDevice& device, bool globalTextures, SkeletonID ownerID);
        ~EweObject();

		TransformComponent transform{};
        std::vector<std::shared_ptr<EWEModel>> meshes{};
        bool drawable = true;
        std::list<int32_t> ownedTextureIDs{};


        //void deTexturize();
        uint32_t getSkeletonID() {
            return mySkinID;
        }
	private:
        uint32_t mySkinID = 0;

        void addToMaterialHandler(EWEDevice& device, ImportData& tempData, TextureMapping& textureTracker);
        void addToSkinHandler(EWEDevice& device, ImportData& tempData, TextureMapping& textureTracker, uint32_t skeletonOwner);

        void loadTextures(EWEDevice& device, std::string objectPath, ImportData::NameExportData& importData, TextureMapping& textureTracker, bool globalTextures);
	};
}