#include "EWEngine/graphics/EWE_Texture.h"

#include <string>
#include <iostream>
#include <filesystem>

#ifndef TEXTURE_DIR
#define TEXTURE_DIR "textures/"
#endif

#ifndef SKYBOX_DIR
#define SKYBOX_DIR "textures/skybox/"
#endif

#define MIPMAP_ENABLED true

namespace EWE {
    std::unordered_map<std::string, TextureID> EWETexture::existingIDs;
    std::unordered_map<std::string, std::pair<ShaderFlags, TextureID>> EWETexture::existingSmartIDs;
    std::unordered_map<std::string, TextureID> EWETexture::existingUIs;

    std::unordered_map<TextureID, EWETexture> EWETexture::textureMap;
    std::unordered_map<TextureID, EWETexture> EWETexture::uiMap;

    TextureID EWETexture::returnID = 0;
    TextureID EWETexture::uiID = 0;
    TextureID EWETexture::skyboxID;
    std::vector<TextureID> EWETexture::sceneIDs;
    std::array<std::string, 6> EWETexture::fileNames = {
        "px", "nx", "py", "ny", "pz", "nz"
    };

    std::vector<std::vector<std::string>> EWETexture::smartTextureTypes = {
            {"_Diffuse", "_albedo", "_diffuse", "_Albedo", "_BaseColor", "_Base_Color", ""},
            { "_Normal", "_normal" },
            { "_roughness", "_rough", "_Rough", "_Roughness"},
            { "_metallic", "_metal", "_Metallic", "_Metal"},
            { "_ao", "_ambientOcclusion", "_AO", "_AmbientOcclusion", "_Ao"},
            { "_bump", "_height", "_parallax"},
    };

    std::unique_ptr<EWEDescriptorSetLayout> EWETexture::simpleDescSetLayout;
    std::unique_ptr<EWEDescriptorSetLayout> EWETexture::spriteDescSetLayout;
    std::unique_ptr<EWEDescriptorSetLayout> EWETexture::simpleVertDescSetLayout;
    std::unique_ptr<EWEDescriptorSetLayout> EWETexture::orbDescSetLayout;

    std::vector<std::unique_ptr<EWEDescriptorSetLayout>> EWETexture::dynamicDescSetLayout;

    std::shared_ptr<EWEDescriptorPool> EWETexture::globalPool;


    uint32_t EWETexture::spriteFrameCounter = 0;

    TextureID EWETexture::addUITexture(EWEDevice& eweDevice, std::string texPath, texture_type tType) {

        if (tType == tType_orbOverlay) {
            std::vector<PixelPeek> pixelPeek(2);
            std::string enginePath = TEXTURE_DIR + texPath + "globeComponent.png";
            //stbi_loadf_from_memory();
            pixelPeek[0].pixels = stbi_load(enginePath.c_str(), &pixelPeek[0].width, &pixelPeek[0].height, &pixelPeek[0].channels, STBI_rgb_alpha);
            if ((!pixelPeek[0].pixels) || ((pixelPeek[0].width * pixelPeek[0].height) <= 0)) {
                printf("failed to load orb: %s \n", texPath.c_str());
                throw std::runtime_error("failed to load orb texture");
            }

            enginePath = TEXTURE_DIR + texPath + "scrollFilled.png";
            pixelPeek[1].pixels = stbi_load(enginePath.c_str(), &pixelPeek[1].width, &pixelPeek[1].height, &pixelPeek[1].channels, STBI_rgb_alpha);
            if ((!pixelPeek[1].pixels) || ((pixelPeek[1].width * pixelPeek[1].height) <= 0)) {
                printf("failed to load orb scroll : %s \n", texPath.c_str());
                throw std::runtime_error("failed to load orb scroll texture");
            }

            //textureMap.emplace(std::make_pair(returnID, EWETexture{ texPath, eweDevice, pixelPeek, EWETexture::tType_orbOverlay }));
            uiMap.emplace(std::make_pair(uiID, EWETexture{ texPath, eweDevice, pixelPeek, EWETexture::tType_orbOverlay }));
            existingUIs[texPath] = uiID;
            return uiID++;
        }
        else {
            if (existingUIs.find(texPath) != existingUIs.end()) {
                //printf("this UI path arleady exist! ~ %s \n", texPath.c_str());
                return existingUIs[texPath];
            }
            std::vector<PixelPeek> pixelPeek(1);
            std::string enginePath = TEXTURE_DIR + texPath;
            pixelPeek[0].pixels = stbi_load(enginePath.c_str(), &pixelPeek[0].width, &pixelPeek[0].height, &pixelPeek[0].channels, STBI_rgb_alpha);
            if ((!pixelPeek[0].pixels) || ((pixelPeek[0].width * pixelPeek[0].height) <= 0)) {
                printf("failed to load UI texture: %s \n", texPath.c_str());
                throw std::runtime_error("faield to load UI texture");
                return -1;
            }
            uiMap.emplace(std::make_pair(uiID, EWETexture{ texPath, eweDevice, pixelPeek }));
            existingUIs[texPath] = uiID;
            return uiID++;
        }
    }
    TextureID EWETexture::addGlobalTexture(EWEDevice& eweDevice, std::string texPath, texture_type tType) {
        if (existingIDs.find(texPath) != existingIDs.end()) {
            //printf("this texture path arleady exist! ~ %s \n", texPath.c_str());
            return existingIDs[texPath];
        }

        //printf("adding global texture : %s \n", texPath.c_str());
        if (tType == tType_simple) {
            return createSimpleTexture(eweDevice, texPath);
        }
        else if (tType == tType_simpleVert) {
            return createSimpleTexture(eweDevice, texPath, tType_simpleVert);
        }
        else if (tType == tType_cube) {
            return createCubeTexture(eweDevice, texPath);
        }
        else if (tType == tType_sprite) {
            return createSimpleTexture(eweDevice, texPath, tType_sprite);
        }
        else if (tType == tType_MRO) {
            return createMaterialTexture(eweDevice, texPath, false).second;
        }
        else if (tType == tType_orbOverlay) {
            printf("can't throw rob in global \n");
            throw std::runtime_error("cant create orb in global \n");
        }
        throw std::runtime_error("invalid texture type??? USE THE ENUM \n");
        return 0;
    }
    TextureID EWETexture::addSceneTexture(EWEDevice& eweDevice, std::string texPath, texture_type tType) {
        uint32_t sceneID = -1;
        if (existingIDs.find(texPath) != existingIDs.end()) {
            //printf("this texture path arleady exist! ~ %s \n", texPath.c_str());
            return existingIDs[texPath];
        }
        //printf("addmin mode texture : %s \n", texPath.c_str());
        if (tType == tType_simple) {
            sceneID = createSimpleTexture(eweDevice, texPath);
        }
        else if (tType == tType_simpleVert) {
            return createSimpleTexture(eweDevice, texPath, tType_simpleVert);
        }
        else if (tType == tType_cube) {
            sceneID = createCubeTexture(eweDevice, texPath);
        }
        else if (tType == tType_sprite) {
            printf("before creating texture with tType_sprite \n");
            sceneID = createSimpleTexture(eweDevice, texPath, tType_sprite);
        }
        else if (tType == tType_MRO) {
            sceneID = createMaterialTexture(eweDevice, texPath, false).second;
        }
        else if (tType == tType_orbOverlay) {
            printf("can't throw orb in mode \n");
            throw std::runtime_error("cant create orb in mode \n");
        }

        if (sceneID >= 0) {
            sceneIDs.push_back(sceneID);
        }
        if (sceneID == -1) {
            printf("failed to load mode texture \n");
        }
#ifdef _DEBUG
        if (sceneID == 29) {
            printf("texPath at 29 : %s \n", texPath.c_str());
        }
#endif
        return sceneID;
    }

    //return value <flags, textureID>
    std::pair<ShaderFlags, TextureID> EWETexture::addSmartGlobalTexture(EWEDevice& eweDevice, std::string texPath, texture_type tType) {
        if (existingSmartIDs.find(texPath) != existingSmartIDs.end()) {
            printf("this smart path already exist! ~ %s \n", texPath.c_str());

            return existingSmartIDs[texPath];
        }

        if (tType == tType_smart) {
            return createMaterialTexture(eweDevice, texPath, true);
        }
        else {
            printf("trying to add a smart texture type that is not tType_smart, no support currently \n");
            throw std::runtime_error("trying to add a smart texture type that is not tType_smart, no support currently");
        }
    }

    std::pair<ShaderFlags, TextureID> EWETexture::addSmartSceneTexture(EWEDevice& eweDevice, std::string texPath, texture_type tType) {
        if (existingSmartIDs.find(texPath) != existingSmartIDs.end()) {
            printf("this smart path already exist! ~ %s \n", texPath.c_str());
            return existingSmartIDs[texPath];
        }

        if (tType == tType_smart) {
            auto smartReturn = createMaterialTexture(eweDevice, texPath, true);
            sceneIDs.push_back(smartReturn.second);
            return smartReturn;
        }
        else {
            printf("trying to add a smart texture type that is not tType_smart, no support currently \n");
            throw std::runtime_error("trying to add a smart texture type that is not tType_smart, no support currently");
        }
    }
    void EWETexture::clearSceneTextures() {
        //everythign created with a mode texture needs to be destroyed. if it persist thru modes, it needs to be a global texture
        printf("clear mode textures beginning \n");
#ifdef _DEBUG
        //DBEUUGGIG TEXUTRE BEING CLEARED INCORRECTLY
        //printf("clearing texutre 29 - %s \n", textureMap.at(29).textureData.path.c_str());
#endif

        for (TextureID i = 0; i < sceneIDs.size(); i++) {
            printf("clearing scene texture : %d \n", i);
#ifdef _DEBUG

            printf("removing mode id iterator? i : modeID : type %d : %d : %d \n", i, sceneIDs[i], textureMap.at(sceneIDs[i]).textureData.tType);
            if (textureMap.at(sceneIDs[i]).textureData.tType) {
                removeSmartTexture(sceneIDs[i]);
                continue;
            }
#endif
            existingIDs.erase(textureMap.at(sceneIDs[i]).textureData.path);
            textureMap.at(sceneIDs[i]).destroy();
            textureMap.erase(sceneIDs[i]);
        }
        sceneIDs.clear();
        printf("clear mode textures end \n");

    }
    void EWETexture::removeSmartTexture(TextureID removeID) {
#ifdef _DEBUG
        //if (removeID == 21) {
        //    printf("texture 21? : %s \n", getTextureData(removeID).path.c_str());
        //}
       // printf("removing smart ID : %d \n", removeID);
#endif

        if (textureMap.find(removeID) != textureMap.end()) {
            printf("before erasing smart texture \n");
            textureMap.at(removeID).destroy();
            existingSmartIDs.erase(textureMap.at(removeID).textureData.path);
            textureMap.erase(removeID);
            printf("after erasing smart texture \n");
            //textureMap.at(removeID).destroy();
            //textureMap.erase(removeID);
        }
        for (int i = 0; i < sceneIDs.size(); i++) {
            if (sceneIDs[i] == removeID) {
                sceneIDs.erase(sceneIDs.begin() + i);
                i--;
            }
        }
    }

    TextureID EWETexture::createSimpleTexture(EWEDevice& eweDevice, std::string texPath, texture_type tType) {
        std::vector<PixelPeek> pixelPeek(1);
        std::string enginePath = TEXTURE_DIR + texPath;
        pixelPeek[0].pixels = stbi_load(enginePath.c_str(), &pixelPeek[0].width, &pixelPeek[0].height, &pixelPeek[0].channels, STBI_rgb_alpha);
        if ((!pixelPeek[0].pixels) || ((pixelPeek[0].width * pixelPeek[0].height) <= 0)) {
            printf("failed to load simple : %s \n", texPath.c_str());
            throw std::runtime_error("failed to load texture");
        }
        //globalTracker.push_back(returnID);
        //printf("before constructing simple texture, tType : %d \n", tType);
        textureMap.emplace(std::make_pair(returnID, EWETexture{ texPath, eweDevice, pixelPeek, tType }));
        //globaEWEctor.emplace_back(EWETexture(eweDevice, texPath));
        existingIDs.emplace(texPath, returnID);
        return returnID++;
    }

    TextureID EWETexture::createCubeTexture(EWEDevice& eweDevice, std::string texPath) {
        //printf("tCubeID : %d \n", tCubeID);
        std::vector<PixelPeek> pixelPeeks(6);
        for (int i = 0; i < 6; i++) {
            std::string individualPath = SKYBOX_DIR;
            individualPath += texPath;
            individualPath += fileNames[i];
            individualPath += ".png";
            pixelPeeks[i].pixels = stbi_load(individualPath.c_str(), &pixelPeeks[i].width, &pixelPeeks[i].height, &pixelPeeks[i].channels, STBI_rgb_alpha);
            if (!pixelPeeks[i].pixels) {
                throw std::runtime_error("failed to load cube texture");
                return -1;
            }
            if (i > 0) {
                if ((pixelPeeks[i].width != pixelPeeks[i - 1].width) || (pixelPeeks[i].height != pixelPeeks[i - 1].height)) {
                    throw std::runtime_error("failed to load smart texture, bad dimensions");
                    return -1;
                }
            }
        }
        textureMap.emplace(std::make_pair((uint32_t)returnID, EWETexture{ texPath, eweDevice, pixelPeeks, tType_cube }));
        //cubeVector.emplace_back(EWETexture(eweDevice, texPath, tType_cube));
        existingIDs.emplace(texPath, returnID);
        skyboxID = returnID;
        return returnID++;
    }
    std::pair<ShaderFlags, TextureID> EWETexture::createMaterialTexture(EWEDevice& device, std::string texPath, bool smart) {
        //printf("creating new MRO Texture : %s \n", texPath.c_str());

        std::vector<bool> foundTypes = {
            false,
            false,
            false,
            false,
            false,
            false,
        };
        /*
        * the future
        * but it searches the entire directory for every texture,
        * too many textures in the full directory rn,
        * and need to filter out opacity, or make opacity work

        std::string basePath = TEXTURE_DIR;
        basePath += texPath.substr(0, texPath.find_first_of("\\") + 1);
        //printf("basePath : %s \n", basePath.c_str());
        std::string texIndPath = texPath.substr(texPath.find_first_of("\\") + 1);
        printf("texIndPath : %s \n", texIndPath.c_str());

        std::vector<std::string> finalPaths;
        if (std::filesystem::exists(basePath)) {
            for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
                //printf("music string : %s \n", entry.path().string().c_str());
                std::string tempString = entry.path().string();
                if (tempString.find(texIndPath) != tempString.npos) {
                    printf("found texIndPath : %s \n", tempString.c_str());
                    finalPaths.push_back(tempString);
                }
            }
        }
        else {
            printf("base path doesnt even exist? \n");
        }
        */


        std::vector<PixelPeek> pixelPeek;
        //cycling thru extensions, currently png and jpg
        for (int i = 0; i < smartTextureTypes.size(); i++) {
            //foundTypes[i] = true;
            for (int j = 0; j < smartTextureTypes[i].size(); j++) {
                std::string materialPath = TEXTURE_DIR;
                materialPath += texPath + smartTextureTypes[i][j];

                //printf("smart material path : %s \n", materialPath.c_str());

                if (std::filesystem::exists(materialPath + ".png")) {
                    materialPath += ".png";
                    PixelPeek tempPeek;
                    tempPeek.pixels = stbi_load(materialPath.c_str(), &tempPeek.width, &tempPeek.height, &tempPeek.channels, STBI_rgb_alpha);
                    if ((!tempPeek.pixels) || ((tempPeek.width * tempPeek.height) <= 0)) {
                        printf("failed to load smart MRO texture %d : %s \n", i, materialPath.c_str());
                        throw std::runtime_error("failed to load smart material");
                    }
                    else {
                        //printf("found texture, png - %s \n", materialPath.c_str());
                        foundTypes[i] = true;
                        pixelPeek.push_back(tempPeek);
                        break;
                    }
                }
                else if (std::filesystem::exists(materialPath + ".jpg")) {
                    materialPath += ".jpg";
                    PixelPeek tempPeek;
                    tempPeek.pixels = stbi_load(materialPath.c_str(), &tempPeek.width, &tempPeek.height, &tempPeek.channels, STBI_rgb_alpha);
                    if ((!tempPeek.pixels) || ((tempPeek.width * tempPeek.height) <= 0)) {
                        printf("failed to load smart MRO texture %d : %s \n", i, materialPath.c_str());
                        throw std::runtime_error("failed to load smart material");
                    }
                    else {
                        //printf("found texture, jpg - %s \n", materialPath.c_str());
                        foundTypes[i] = true;
                        pixelPeek.push_back(tempPeek);
                        break;
                    }
                }
                else if (std::filesystem::exists(materialPath + ".tga")) {
                    materialPath += ".tga";
                    PixelPeek tempPeek;
                    tempPeek.pixels = stbi_load(materialPath.c_str(), &tempPeek.width, &tempPeek.height, &tempPeek.channels, STBI_rgb_alpha);
                    if ((!tempPeek.pixels) || ((tempPeek.width * tempPeek.height) <= 0)) {
                        printf("failed to load smart MRO texture %d : %s \n", i, materialPath.c_str());
                        throw std::runtime_error("failed to load smart material");
                    }
                    else {
                        //printf("found texture, jpg - %s \n", materialPath.c_str());
                        foundTypes[i] = true;
                        pixelPeek.push_back(tempPeek);
                        break;
                    }
                }
                /*
                else {
                    printf("could not find materialpathg with any extensions : %s \n", materialPath.c_str());
                }
                */
            }
        }

        //flag it up
        //albedo only -> throw an error
        //no albedo -> throw an error

        //flags = normal, metal, rough, ao
        ShaderFlags flags = (foundTypes[5] << 4) + (foundTypes[1] << 3) + (foundTypes[2] << 2) + (foundTypes[3] << 1) + (foundTypes[4]);
        //printf("flag values : %d \n", flags);
        if (!foundTypes[0]) {
            printf("did not find an albedo or diffuse texture for this MRO set : %s \n", texPath.c_str());
            throw std::runtime_error("no albedo in dynamic material");
            //std::throw 
        }
        if (foundTypes[5]) {
            printf("found a parallax map \n");
        }
        //printf("constructng texture from smart \n");
        textureMap.emplace(returnID, EWETexture{ texPath, device, pixelPeek, tType_smart, flags });
#ifdef _DEBUG
        
        //printf("texPath of texture %d : %s \n", returnID, texPath.c_str());
       
#endif
        existingSmartIDs[texPath] = std::pair<ShaderFlags, int32_t>{ flags, returnID };
        
        //printf("returning from smart creation \n");
        return std::pair<ShaderFlags, int32_t>{ flags, returnID++ };
    }


    EWETexture::EWETexture(std::string texPath, EWEDevice& device, std::vector<PixelPeek>& pixelPeek, texture_type tType) : eweDevice{ device } {
        //imageLayout{ descriptorCount }, image{ descriptorCount }, imageMemory{ descriptorCount }, texPath{ texPath }
        textureData.path = texPath;
        textureData.tType = tType;
        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        mipLevels.resize(pixelPeek.size(), 1);
#if GPU_LOGGING
    {
        std::ofstream textureLogger{ GPU_LOG_FILE, std::ios::app };
        textureLogger << "creating texture : " << texPath << "\n";
        textureLogger.close();
    }
#endif
        //printf("beginning texture constructor, tType : %d \n", tType);
        if (tType == tType_simple) {
            //printf("constructing texture type simple \n");
            sampler.resize(1);
            imageView.resize(1);
            imageLayout.resize(1);
            image.resize(1);
            imageMemory.resize(1);
            //printf("size of simple pixel peek : %d \n", pixelPeek.size());
            createTextureImage(pixelPeek); //strange to pass in the first, btu whatever
            //printf("after create image \n");
            createTextureImageView(tType);
            //printf("after image view \n");
            createTextureSampler(tType);
            //printf("before descriptors \n");

            descriptor.resize(1);
            descriptor[0].sampler = sampler[0];
            descriptor[0].imageView = imageView[0];
            descriptor[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            //printf("before create simple desc \n");
            createSimpleDescriptor();
            //printf("after create simple desc \n");
        }
        else if (tType == tType_orbOverlay) {
            sampler.resize(2);
            imageView.resize(2);
            imageLayout.resize(2);
            image.resize(2);
            imageMemory.resize(2);
            //printf("size of simple pixel peek : %d \n", pixelPeek.size());
            createTextureImage(pixelPeek); //strange to pass in the first, btu whatever
            //printf("after create image \n");
            createTextureImageView(tType);
            //printf("after image view \n");
            createTextureSampler(tType);
            //printf("before descriptors \n");

            descriptor.resize(2);
            for (int i = 0; i < 2; i++) {
                descriptor[i].sampler = sampler[i];
                descriptor[i].imageView = imageView[i];
                descriptor[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            printf("before create orb desc \n");
            createOrbDescriptor();
            printf("after create orb desc \n");
        }
        else if (tType == tType_simpleVert) {
            sampler.resize(1);
            imageView.resize(1);
            imageLayout.resize(1);
            image.resize(1);
            imageMemory.resize(1);
            //printf("size of pixel peek : %d \n", pixelPeek.size());
            createTextureImage(pixelPeek); //strange to pass in the first, btu whatever
            //printf("after create image \n");
            createTextureImageView(tType);
            //printf("after image view \n");
            createTextureSampler(tType);
            //printf("before descriptors \n");

            descriptor.resize(1);
            descriptor[0].sampler = sampler[0];
            descriptor[0].imageView = imageView[0];
            descriptor[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            createSimpleVertDescriptor();
        }
        else if (tType == tType_cube) {
            sampler.resize(1);
            imageView.resize(1);
            imageLayout.resize(1);
            image.resize(1);
            imageMemory.resize(1);
            createCubeImage(pixelPeek);
            createTextureImageView(tType);
            createTextureSampler(tType);

            descriptor.resize(1);
            descriptor[0].sampler = sampler[0];
            descriptor[0].imageView = imageView[0];
            descriptor[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            createSimpleDescriptor();
        }
        else if (tType == tType_sprite) {
            //printf("inside texture constructor, tType sprite \n");
            sampler.resize(1);
            imageView.resize(1);
            imageLayout.resize(1);
            image.resize(1);
            imageMemory.resize(1);
            //printf("size of pixel peek : %d \n", pixelPeek.size());
            createTextureImage(pixelPeek); //strange to pass in the first, btu whatever
            //printf("after create image \n");
            createTextureImageView(tType);
            //printf("after image view \n");
            createTextureSampler(tType);
            //printf("before descriptors \n");

            descriptor.resize(1);
            descriptor[0].sampler = sampler[0];
            descriptor[0].imageView = imageView[0];
            descriptor[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            //printf("before creating sprite descriptor \n");
            createSimpleDescriptor();
            //createSpriteDescriptor();
            //printf("after creating sprite descriptor, end of texture constructor \n");
        }
        //printf("ending texture constructor");
#if GPU_LOGGING
    {
        std::ofstream textureLogger{ GPU_LOG_FILE, std::ios::app };
        textureLogger << "texture created successfully : " << texPath << "\n";
        textureLogger.close();
    }
#endif
        
    }

    EWETexture::EWETexture(std::string texPath, EWEDevice& device, std::vector<PixelPeek>& pixelPeek, texture_type tType, ShaderFlags flags) : eweDevice{ device } {
        //printf("beginning smart texture construcotr, size of pixelPeek : %d  \n", pixelPeek.size());
        textureData.path = texPath;
        textureData.tType = tType;
        textureData.materialFlags = flags;
        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

#if GPU_LOGGING
    {
        std::ofstream textureLogger{ GPU_LOG_FILE, std::ios::app };
        textureLogger << "creating texture : " << texPath << "\n";
        textureLogger.close();
    }
#endif

        mipLevels.resize(pixelPeek.size(), 1);
        if (tType == tType_smart) {
           // printf("creating a smart texture sized : %d \n", pixelPeek.size());
            sampler.resize(pixelPeek.size());
            imageView.resize(pixelPeek.size());
            imageLayout.resize(pixelPeek.size());
            image.resize(pixelPeek.size());
            imageMemory.resize(pixelPeek.size());

            //printf("before texture image \n");
            createTextureImage(pixelPeek);
            //printf("before image view \n");
            createTextureImageView(tType);
            //printf("before sampelr \n");
            createTextureSampler(tType);

            //printf("after sampler \n");
            descriptor.resize(pixelPeek.size());
            //printf("getting into descriptor \n");
            for (int i = 0; i < pixelPeek.size(); i++) {
                descriptor[i].sampler = sampler[i];
                descriptor[i].imageView = imageView[i];
                descriptor[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            //printf("before dynamic descriptor creation \n");
            createDynamicDescriptor(static_cast<uint16_t>(pixelPeek.size()));
            //printf("after dynamic descriptor \n");
        }
        else {
            printf("trying to create a non-smart texture with a smart texture constructor \n");
            //std::thrwo
        }

#if GPU_LOGGING
    {
        std::ofstream textureLogger{ GPU_LOG_FILE, std::ios::app };
        textureLogger << "texture created successfully? : " << texPath << "\n";
        textureLogger.close();
    }
#endif
    }

    void EWETexture::cleanup() {
        //call at end of program
        //printf("beginning of texture cleanup \n");
        //uint32_t tracker = 0;
        for (auto iter = textureMap.begin(); iter != textureMap.end(); iter++) {
            //printf("%d tracking \n", tracker++);
            iter->second.destroy();

            //textureMap.erase(iter);
        }
        //printf("before clear \n");
        textureMap.clear();
        //printf("after clear \n");
        //printf("after texture map cleanup \n");
        //tracker = 0;
        for (auto iter = uiMap.begin(); iter != uiMap.end(); iter++) {
            //printf("%d tracking \n", tracker++);
            iter->second.destroy();
        }
        uiMap.clear();
        //printf("after uimap cleanup \n");
        simpleDescSetLayout.reset();
        simpleVertDescSetLayout.reset();
        orbDescSetLayout.reset();
        spriteDescSetLayout.reset();
        dynamicDescSetLayout.clear();

        globalPool.reset();
        printf("end of texture cleanup \n");
    }

    void EWETexture::destroy() {
        //std::cout << "destroying" << std::endl;
        globalPool->freeDescriptors(descriptorSets);

        for (int i = 0; i < sampler.size(); i++) {
            vkDestroySampler(eweDevice.device(), sampler[i], nullptr);
        }
        //printf("after sampler destruction \n");
        for (int i = 0; i < imageView.size(); i++) {
            vkDestroyImageView(eweDevice.device(), imageView[i], nullptr);
        }
        //printf("after image view destruction \n");
        for (int i = 0; i < image.size(); i++) {
            vkDestroyImage(eweDevice.device(), image[i], nullptr);
        }
        //printf("after image destruction \n");
        for (int i = 0; i < imageMemory.size(); i++) {
            vkFreeMemory(eweDevice.device(), imageMemory[i], nullptr);
        }
        //this->~EWETexture();
        //std::cout << "finished destroying" << std::endl;
    }

    void EWETexture::createTextureImage(std::vector<PixelPeek>& pixelPeek) {
        width.resize(pixelPeek.size());
        height.resize(pixelPeek.size());
        for (int i = 0; i < pixelPeek.size(); i++) {
            width[i] = pixelPeek[i].width;
            height[i] = pixelPeek[i].height;
            VkDeviceSize imageSize = width[i] * height[i] * 4;
            //printf("image dimensions : %d:%d \n", width[i], height[i]);
            //printf("beginning of create image, dimensions - %d : %d : %d \n", width[i], height[i], pixelPeek[i].channels);
            if (MIPMAP_ENABLED) {
                mipLevels[i] = static_cast<uint32_t>(std::floor(std::log2(std::max(width[i], height[i]))) + 1);
            }
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            //printf("before creating buffer \n");

            eweDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
            //printf("before memory mapping \n");
            void* data;
            vkMapMemory(eweDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
            //printf("memcpy \n");
            memcpy(data, pixelPeek[i].pixels, static_cast<size_t>(imageSize));
            //printf("unmapping \n");
            vkUnmapMemory(eweDevice.device(), stagingBufferMemory);
            //printf("freeing pixels \n");
            stbi_image_free(pixelPeek[i].pixels);
            //printf("after memory mapping \n");

            VkImageCreateInfo imageInfo;
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.pNext = nullptr;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = width[i];
            imageInfo.extent.height = height[i];
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = mipLevels[i];
            imageInfo.arrayLayers = 1;

            imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.flags = 0; // Optional

            //printf("before image info \n");
            eweDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image[i], imageMemory[i]);
            //printf("before transition \n");
            eweDevice.transitionImageLayout(image[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels[i]);
            //printf("before copy buffer to image \n");
            eweDevice.copyBufferToImage(stagingBuffer, image[i], width[i], height[i], 1);
            //printf("after copy buffer to image \n");
            //i gotta do this a 2nd time i guess
            //eweDevice.transitionImageLayout(image[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels[i]);

            vkDestroyBuffer(eweDevice.device(), stagingBuffer, nullptr);
            vkFreeMemory(eweDevice.device(), stagingBufferMemory, nullptr);
            //printf("end of create texture image loop %d \n", i);
        }
        //printf("before generate mip maps \n");
        generateMipmaps(VK_FORMAT_R8G8B8A8_SRGB);
        //printf("after generate mip maps \n");
    }

    void EWETexture::createCubeImage(std::vector<PixelPeek>& pixelPeek) {
        width.resize(1);
        height.resize(1);
        width[0] = pixelPeek[0].width;
        height[0] = pixelPeek[0].height;
        uint64_t layerSize = width[0] * height[0] * 4;
        VkDeviceSize imageSize = layerSize * 6;

        void* data;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        eweDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        vkMapMemory(eweDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
        uint64_t memAddress = reinterpret_cast<uint64_t>(data);

        mipLevels.resize(6, 1);
        for (int i = 0; i < 6; i++) {
            memcpy(reinterpret_cast<void*>(memAddress), pixelPeek[i].pixels, static_cast<size_t>(layerSize)); //static_cast<void*> unnecessary>?
            stbi_image_free(pixelPeek[i].pixels);
            memAddress += layerSize;
        }
        vkUnmapMemory(eweDevice.device(), stagingBufferMemory);

        VkImageCreateInfo imageInfo;
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width[0];
        imageInfo.extent.height = height[0];
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 6;

        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        eweDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image[0], imageMemory[0]);

        eweDevice.transitionImageLayout(image[0], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels[0], 6);
        eweDevice.copyBufferToImage(stagingBuffer, image[0], width[0], height[0], 6);



        vkDestroyBuffer(eweDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(eweDevice.device(), stagingBufferMemory, nullptr);

        //i gotta do this a 2nd time i guess
        eweDevice.transitionImageLayout(image[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels[0], 6, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }

    void EWETexture::createTextureImageView(texture_type tType) {
        for (int i = 0; i < image.size(); i++) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = mipLevels[i];
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (tType == tType_cube) {
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
                viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
                viewInfo.subresourceRange.layerCount = 6;
            }


            if (vkCreateImageView(eweDevice.device(), &viewInfo, nullptr, &imageView[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }
        }
    }

    void EWETexture::createTextureSampler(texture_type tType) {
        for (int i = 0; i < image.size(); i++) {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

            //if(tType == tType_2d){
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;

            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            if (tType == tType_cube) {
                samplerInfo.magFilter = VK_FILTER_NEAREST;
                samplerInfo.minFilter = VK_FILTER_NEAREST;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            }
            samplerInfo.addressModeV = samplerInfo.addressModeU;
            samplerInfo.addressModeW = samplerInfo.addressModeU;

            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = eweDevice.getProperties().limits.maxSamplerAnisotropy;

            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;

            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.mipLodBias = 0.0f;

            //force sampler to not use lowest level by changing this value
            // i.e. samplerInfo.minLod = static_cast<float>(mipLevels / 2);
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = static_cast<float>(mipLevels[i]);

            if (vkCreateSampler(eweDevice.device(), &samplerInfo, nullptr, &sampler[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }
    }

    void EWETexture::generateMipmaps(VkFormat imageFormat) {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(eweDevice.getPhysicalDevice(), imageFormat, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }
        //printf("before mip map loop? size of image : %d \n", image.size());
        for (int j = 0; j < image.size(); j++) {

            VkCommandBuffer commandBuffer = SyncHub::getSyncHubInstance()->beginSingleTimeCommands();
            //printf("after beginning single time command \n");

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = image[j];
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;

            int32_t mipWidth = width[j];
            int32_t mipHeight = height[j];

            for (uint32_t i = 1; i < mipLevels[j]; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                //printf("before cmd pipeline barrier \n");
                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);
                //printf("after cmd pipeline barreir \n");
                VkImageBlit blit{};
                blit.srcOffsets[0] = { 0, 0, 0 };
                blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = { 0, 0, 0 };
                blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;
                //printf("before blit image \n");
                vkCmdBlitImage(commandBuffer,
                    image[j], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    image[j], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit,
                    VK_FILTER_LINEAR);
                //printf("after blit image \n");
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);
                //printf("after pipeline barrier 2 \n");
                if (mipWidth > 1) { mipWidth /= 2; }
                if (mipHeight > 1) { mipHeight /= 2; }
            }

            barrier.subresourceRange.baseMipLevel = mipLevels[j] - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            //printf("before pipeline barrier 3 \n");
            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
            //printf("after pipeline barrier 3 \n");
            eweDevice.endSingleTimeCommands(commandBuffer);
            //printf("after end single time commands \n");
        }
        //printf("end of mip maps \n");
    }
    void EWETexture::initStaticVariables() {
        //printf("initting EWEtexture static variables \n");

        assert(smartTextureTypes.size() == MAX_SMART_TEXTURE_COUNT);
    }

    void EWETexture::buildSetLayouts(EWEDevice& eweDevice) {
        initStaticVariables();
        //printf("building set layouts \n");

        simpleDescSetLayout = EWEDescriptorSetLayout::Builder(eweDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //texture
            .build();

        orbDescSetLayout = EWEDescriptorSetLayout::Builder(eweDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //orbs
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //scroll
            .build();

        spriteDescSetLayout = EWEDescriptorSetLayout::Builder(eweDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        simpleVertDescSetLayout = EWEDescriptorSetLayout::Builder(eweDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT)
            .build();


        for (int i = 0; i < MAX_SMART_TEXTURE_COUNT; i++) { //MAX_TEXTURE_COUNT + 1
            //printf("building dynamic descritpor layout, i : %d \n", i);
            auto tempLayout = EWEDescriptorSetLayout::Builder(eweDevice);
            for (int j = 0; j < (i + 1); j++) {
               // printf("\t adding binding to dynamic descritpor layout, j : %d \n", j);
                tempLayout.addBinding(j, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
            }
            dynamicDescSetLayout.push_back(tempLayout.build());
        }
    }
    void EWETexture::createSimpleDescriptor() {
        //printf("creating simple descriptor \n");
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (!
                EWEDescriptorWriter(*simpleDescSetLayout, *globalPool)
                .writeImage(0, &descriptor[0])
                .build(descriptorSets[i]))
            {
                //returnValue = false;
                printf("simple descriptor failure at back \n");
            }
        }
    }
    void EWETexture::createSimpleVertDescriptor() {
        //printf("creating simple vert descriptor \n");
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (!
                EWEDescriptorWriter(*simpleVertDescSetLayout, *globalPool)
                .writeImage(0, &descriptor[0])
                .build(descriptorSets[i]))
            {
                //returnValue = false;
                printf("simple vert descriptor failure at back \n");
            }
        }
    }
    void EWETexture::createOrbDescriptor() {
        //printf("creating orb descriptor \n");
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (!
                EWEDescriptorWriter(*orbDescSetLayout, *globalPool)
                .writeImage(0, &descriptor[0])
                .writeImage(1, &descriptor[1])
                .build(descriptorSets[i]))
            {
                //returnValue = false;
                printf("orb descriptor failure at back \n");
            }
        }
    }

    void EWETexture::createDynamicDescriptor(uint16_t imageCount) {
        //printf("creating dynamic descriptor set, imageCount - %d \n", imageCount);
        if (imageCount == 0) {
            printf("why do we have 0 imageCount in dynamic descriptor construction? \n");
        }
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            //printf("creating dynamic descriptor, imageCount : %d \n", imageCount);
            auto tempHolder = EWEDescriptorWriter(*dynamicDescSetLayout[imageCount - 1], *globalPool);
            for (int j = 0; j < imageCount; j++) {
                //printf("writing image to dynamic desc set layout : %d \n", j);
                tempHolder.writeImage(j, &descriptor[j]);
            }

            if(!tempHolder.build(descriptorSets[i])) {
				//returnValue = false;
				printf("dynamic descriptor failure at back, imageCount : %d \n", imageCount);
			}
		}
    }

    VkDescriptorSet* EWETexture::getDescriptorSets(TextureID textureID, uint8_t frame) {
        //printf("descriptor set ~ %d \n", textureID);

#ifdef _DEBUG
            //if (textureID == 13) { //match this with whatever fucking
            //	printf("texture string : %s \n", getTextureData(textureID).first.c_str());
            //}
        if (textureMap.find(textureID) == textureMap.end()) {
            printf("TEXTURE DOES NOT EXIST  : %d \n", textureID);
            printf("texture string : %s \n", getTextureData(textureID).path.c_str());
            throw std::exception("texture descriptor set doesnt exist");
        }
#endif
        return &textureMap.at(textureID).descriptorSets[frame];
    }
    void EWETexture::setSpriteValues(TextureID spriteID, uint32_t widthInPixels, uint32_t heightInPixels) {



        assert((widthInPixels < textureMap.at(spriteID).width[0]) && ((textureMap.at(spriteID).width[0] % widthInPixels) == 0));

        textureMap.at(spriteID).spriteWidth = static_cast<float>(widthInPixels) / static_cast<float>(textureMap.at(spriteID).width[0]);
        printf("sprite Width : %.2f \n", textureMap.at(spriteID).spriteWidth);
        textureMap.at(spriteID).spriteHorizontalCount = textureMap.at(spriteID).width[0] / widthInPixels;
        printf("sprite count : %d \n", textureMap.at(spriteID).spriteHorizontalCount);
        textureMap.at(spriteID).spriteHeight = static_cast<float>(heightInPixels) / static_cast<float>(textureMap.at(spriteID).height[0]);
        textureMap.at(spriteID).spriteVerticalCount = textureMap.at(spriteID).height[0] / heightInPixels;
    };
    glm::vec4 EWETexture::getSpritePush(TextureID spriteID, uint16_t spriteFrame) {

        uint8_t horizontalIndex = spriteFrame % textureMap.at(spriteID).spriteHorizontalCount;
        uint8_t verticalIndex = (spriteFrame - horizontalIndex) / textureMap.at(spriteID).spriteVerticalCount;

#ifdef _DEBUG
        if (spriteFrame >= textureMap.at(spriteID).spriteHorizontalCount * textureMap.at(spriteID).spriteVerticalCount) {
			printf("sprite frame out of bounds : %d \n", spriteFrame);
            throw std::exception("sprite frame out of bounds");
		}
#endif

        return { 
            textureMap.at(spriteID).spriteWidth, horizontalIndex, 
            textureMap.at(spriteID).spriteHeight, verticalIndex
        
        };
        //spriteBuffers[frameIndex]->writeToBuffer(spriteBuffer);
        //spriteBuffers[frameIndex]->flush();
    }
}