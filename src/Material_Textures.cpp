#include "EWEngine/Graphics/Texture/Material_Textures.h"


#ifndef TEXTURE_DIR
#define TEXTURE_DIR "textures/"
#endif


namespace EWE {

    MaterialTextureInfo Material_Texture::CreateMaterialTexture(std::string texPath, bool global) {

        const std::array<std::vector<std::string>, MAX_MATERIAL_TEXTURE_COUNT> smartTextureTypes = {
            //ordering it like this is necessary, the engine is set up so that vertex textures are always binded lower than fragment textures.
            // this needs to be in sync with MaterialTexType
            std::vector<std::string>{ "bump", "height"},

            std::vector<std::string>{"Diffuse", "albedo", "diffuse", "Albedo", "BaseColor", "Base_Color"},
            std::vector<std::string>{ "Normal", "normal" },
            std::vector<std::string>{ "roughness", "rough", "Rough", "Roughness"},
            std::vector<std::string>{ "metallic", "metal", "Metallic", "Metal"},
            std::vector<std::string>{ "ao", "ambientOcclusion", "AO", "AmbientOcclusion", "Ao"},
        };

        //printf("creating new MRO Texture : %s \n", texPath.c_str());
        auto tmPtr = Texture_Manager::GetTextureManagerPtr();

        if (tmPtr->existingMaterials.contains(texPath)) {
            return tmPtr->existingMaterials.at(texPath);
        }

        std::vector<bool> foundTypes{};
        foundTypes.resize(smartTextureTypes.size(), false);


        //std::vector<EWETexture::PixelPeek> pixelPeek{};
        std::vector<std::string> materialPaths{};
        //cycling thru extensions, currently png and jpg

        Texture_Builder tBuilder{ global };
        for (int i = 0; i < smartTextureTypes.size(); i++) {
            //foundTypes[i] = true;
            for (int j = 0; j < smartTextureTypes[i].size(); j++) {
                materialPaths.emplace_back(TEXTURE_DIR);
                materialPaths.back() += texPath + smartTextureTypes[i][j];

                //printf("smart material path : %s \n", materialPath.c_str());

                if (std::filesystem::exists(materialPaths.back() + ".png")) {
                    materialPaths.back() += ".png";
                    //pixelPeek.emplace_back(materialPath);
                    foundTypes[i] = true;
                    if (i == 0) {
                        tBuilder.AddComponent(materialPaths.back(), VK_SHADER_STAGE_VERTEX_BIT, false);
                    }
                    else {
                        tBuilder.AddComponent(materialPaths.back(), VK_SHADER_STAGE_FRAGMENT_BIT, true);
                    }

                    break;
                    
                }
                else if (std::filesystem::exists(materialPaths.back() + ".jpg")) {
                    materialPaths.back() += ".jpg";
                    //pixelPeek.emplace_back(materialPath);
                    foundTypes[i] = true;
                    if (i == 0) {
                        tBuilder.AddComponent(materialPaths.back(), VK_SHADER_STAGE_VERTEX_BIT, false);
                    }
                    else {
                        tBuilder.AddComponent(materialPaths.back(), VK_SHADER_STAGE_FRAGMENT_BIT, true);
                    }
                    break;
                }
                else if (std::filesystem::exists(materialPaths.back() + ".tga")) {
                    materialPaths.back() += ".tga";
                    //pixelPeek.emplace_back(materialPaths.back());
                    foundTypes[i] = true;
                    if (i == 0) {
                        tBuilder.AddComponent(materialPaths.back(), VK_SHADER_STAGE_VERTEX_BIT, false);
                    }
                    else {
                        tBuilder.AddComponent(materialPaths.back(), VK_SHADER_STAGE_FRAGMENT_BIT, true);
                    }
                    break;
                }
                materialPaths.pop_back();
            }
        }

        //flag it up
        //albedo only -> throw an error -> idk why but albedo only no longer throws an error. had some good reason for removing it when i did
        //no albedo -> throw an error

        //flags = normal, metal, rough, ao
        MaterialFlags flags = (foundTypes[MT_bump] * MaterialF_hasBump) + (foundTypes[MT_metal] * MaterialF_hasMetal) + (foundTypes[MT_rough] * MaterialF_hasRough) + (foundTypes[MT_ao] * MaterialF_hasAO) + ((foundTypes[MT_normal] * MaterialF_hasNormal));
        //printf("flag values : %d \n", flags);

#if EWE_DEBUG
        if (!foundTypes[MT_albedo]) {
            printf("did not find an albedo or diffuse texture for this MRO set : %s \n", texPath.c_str());
            assert(false);
        }
        if (foundTypes[MT_bump]) {
            printf("found a height map \n");
        }
#endif
        //printf("constructng texture from smart \n");
        //textureMap.emplace(returnID, EWETexture{ texPath, device, pixelPeek, tType_material, flags });

        TextureDesc retID = tBuilder.Build();

        tmPtr->existingMaterials.try_emplace(texPath, flags, retID);
        //existingMaterialIDs[texPath] = std::pair<MaterialFlags, int32_t>{ flags, returnID };

        //printf("returning from smart creation \n");
        return { flags, retID };
    }
}