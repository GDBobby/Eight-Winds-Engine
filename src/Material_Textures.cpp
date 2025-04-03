#include "EWEngine/Graphics/Texture/Material_Textures.h"

#include <filesystem>

#ifndef TEXTURE_DIR
#define TEXTURE_DIR "textures/"
#endif


namespace EWE {

    MaterialInfo Material_Image::CreateMaterialImage(std::string texPath, bool mipmapping, bool global) {

        auto imPtr = Image_Manager::GetImageManagerPtr();
        {
            ImageID imgID = imPtr->FindByPath(texPath);
            if (imgID != IMAGE_INVALID) {
                std::unique_lock<std::mutex> imgLock(imPtr->imageMutex);
                auto findRet = imPtr->existingMaterialsByID.find(imgID);
                if (findRet != imPtr->existingMaterialsByID.end()) {
                    return findRet->second;
                }
#if EWE_DEBUG
                printf("image manager contains texPath, but it's not a material?\n");
#endif
            }
        }

        const std::array<std::vector<std::string>, MAX_MATERIAL_TEXTURE_COUNT> matImgTypes = {
            //ordering it like this is necessary, the engine is set up so that vertex textures are always binded lower than fragment textures.
            // this needs to be in sync with MaterialImgType
            std::vector<std::string>{ "bump", "height"},

            std::vector<std::string>{"Diffuse", "albedo", "diffuse", "Albedo", "BaseColor", "Base_Color"},
            std::vector<std::string>{ "Normal", "normal" },
            std::vector<std::string>{ "roughness", "rough", "Rough", "Roughness"},
            std::vector<std::string>{ "metallic", "metal", "Metallic", "Metal"},
            std::vector<std::string>{ "ao", "ambientOcclusion", "AO", "AmbientOcclusion", "Ao"},
        };
        std::vector<std::string> matPaths{};
        std::array<bool, MAX_MATERIAL_TEXTURE_COUNT> foundTypes{false};

        for (int i = 0; i < matImgTypes.size(); i++) {
            //foundTypes[i] = true;
            for (int j = 0; j < matImgTypes[i].size(); j++) {
                auto& empRet = matPaths.emplace_back(TEXTURE_DIR);
                empRet += texPath + matImgTypes[i][j];

                printf("smart material path : %s \n", matPaths.back().c_str());

                if (std::filesystem::exists(matPaths.back() + ".png")) {
                    empRet += ".png";
                    //pixelPeek.emplace_back(materialPath);
                    foundTypes[i] = true;

                    break;

                }
                else if (std::filesystem::exists(matPaths.back() + ".jpg")) {
                    empRet += ".jpg";
                    //pixelPeek.emplace_back(materialPath);
                    foundTypes[i] = true;
                    break;
                }
                else if (std::filesystem::exists(matPaths.back() + ".tga")) {
                    empRet += ".tga";
                    //pixelPeek.emplace_back(materialPaths.back());
                    foundTypes[i] = true;
                    break;
                }
                matPaths.pop_back();
            }
        }

        std::vector<PixelPeek> pixelPeeks{};
        for (int i = 0; i < matPaths.size(); i++) {
            pixelPeeks.emplace_back(matPaths[i]);
        }

        const ImageID imgID = imPtr->CreateImageArray(pixelPeeks, mipmapping);

        //flags = normal, metal, rough, ao
        const MaterialFlags flags = (foundTypes[MT_albedo] * Material::Albedo) + (foundTypes[MT_bump] * Material::Bump) + (foundTypes[MT_metal] * Material::Metal) + (foundTypes[MT_rough] * Material::Rough) + (foundTypes[MT_ao] * Material::AO) + ((foundTypes[MT_normal] * Material::Normal));
        //printf("flag values : %d \n", flags);
        assert(flags != 0 && "found zero images in material texture, needs at least one");

#if EWE_DEBUG
        if (!foundTypes[MT_albedo]) {
            printf("did not find an albedo or diffuse texture for this MRO set : %s \n", texPath.c_str());
            assert(false);
        }
        if (foundTypes[MT_bump]) {
            printf("found a height map \n");
        }
#endif
        std::unique_lock<std::mutex> imgLock(imPtr->imageMutex);
        imPtr->imageStringToIDMap.try_emplace(texPath, imgID);
        imPtr->existingMaterialsByID.try_emplace(imgID, flags, imgID);
        //existingMaterialIDs[texPath] = std::pair<MaterialFlags, int32_t>{ flags, returnID };

        //printf("returning from smart creation \n");
        return { flags, imgID };
    }
}