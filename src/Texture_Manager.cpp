#include "EWEngine/Graphics/Texture/Texture_Manager.h"

#ifndef TEXTURE_DIR
#define TEXTURE_DIR "textures/"
#endif

namespace EWE {


    Texture_Manager* Texture_Manager::textureManagerPtr{ nullptr };


    Texture_Builder::Texture_Builder(EWEDevice& device, bool global) : device(device), global(global) {}

    void Texture_Builder::addComponent(std::string const& texPath, VkShaderStageFlags stageFlags, bool mipmaps) {
        switch (stageFlags) {
            case VK_SHADER_STAGE_VERTEX_BIT: {
                imageCI[0].emplace_back(texPath, mipmaps);
                break;
            }
            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: {
                imageCI[1].emplace_back(texPath, mipmaps);
                break;
            }
            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: {
                imageCI[2].emplace_back(texPath, mipmaps);
                break;
            }
            case VK_SHADER_STAGE_GEOMETRY_BIT: {
                imageCI[3].emplace_back(texPath, mipmaps);
                break;
            }
            case VK_SHADER_STAGE_FRAGMENT_BIT: {
                imageCI[4].emplace_back(texPath, mipmaps);
                break;
            }
            case VK_SHADER_STAGE_COMPUTE_BIT: {
                imageCI[5].emplace_back(texPath, mipmaps);
                break;
            }
            case VK_SHADER_STAGE_ALL_GRAPHICS: {
                //im pretty sure that _ALL_GRAPHICS and _ALL are both noob traps, but ill put them in anyways
                imageCI[6].emplace_back(texPath, mipmaps);
                break;
            }
            case VK_SHADER_STAGE_ALL: {
                imageCI[7].emplace_back(texPath, mipmaps);
                break;
            }
            case VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT: {
                imageCI[8].emplace_back(texPath, mipmaps);
                break;
            }
            default: {
                printf("unsupported texture shader stage flag \n");
                throw std::runtime_error("invalid shader stage flag for textures");
                break;
            }
        }
    }

    

    TextureDesc Texture_Builder::build() {
        auto tmPtr = Texture_Manager::getTextureManagerPtr();
        
        //size_t myHash = Texture_Manager::hashTexture(paths);

        TextureDSLInfo dslInfo{};
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //this is 1
        EWEDescriptorSetLayout::Builder dslBuilder{ device };

        uint16_t imageCount = 0;
        for (uint8_t i = 0; i < 6; i++) {
            dslInfo.setStageTextureCount(stageFlags, imageCI[i].size());
            stageFlags <<= 1;
            imageCount += imageCI[i].size();
        }
        dslInfo.setStageTextureCount(VK_SHADER_STAGE_ALL_GRAPHICS, imageCI[6].size());
        imageCount += imageCI[6].size();
        dslInfo.setStageTextureCount(VK_SHADER_STAGE_ALL, imageCI[7].size());
        imageCount += imageCI[7].size();
        dslInfo.setStageTextureCount(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, imageCI[8].size());
        imageCount += imageCI[8].size();

        std::vector<Texture_Manager::ImageTracker*> imageInfos{};
        imageInfos.resize(imageCount, nullptr);

        bool uniqueDescriptor = false;
        uint16_t currentImage = 0;
        for (uint8_t i = 0; i < imageCI.size(); i++) {
            for (auto& conInfo : imageCI[i]) {
                auto tempImageInfo = tmPtr->imageMap.find(conInfo.path);
                if (tempImageInfo != tmPtr->imageMap.end()) {
                    imageInfos[currentImage] = tempImageInfo->second;
                }
                else {
                    uniqueDescriptor = true;
                    imageInfos[currentImage] = Texture_Manager::constructImageTracker(conInfo.path, conInfo.mipmaps);
                }
                currentImage++;
            }
        }

        if (uniqueDescriptor) {
            EWEDescriptorWriter descBuilder(*dslInfo.getDescSetLayout(device), DescriptorPool_Global);
            for (uint16_t i = 0; i < imageInfos.size(); i++) {
                descBuilder.writeImage(i, imageInfos[i]->imageInfo.getDescriptorImageInfo());
            }
            TextureDesc retDesc = descBuilder.build();
            for (auto& imageInfo : imageInfos) {
                imageInfo->usedInTexture.insert(retDesc);
            }

            tmPtr->textureImages.try_emplace(retDesc, imageInfos);
            //tmPtr->textureMap.emplace(tmPtr->currentTextureCount, descBuilder.build());
            //tmPtr->deletionMap.emplace(tmPtr->currentTextureCount, );
            if (!global) {
                tmPtr->sceneIDs.push_back(retDesc);
            }
            tmPtr->currentTextureCount++;
            return retDesc;
        }
        else {
            

            std::unordered_set<TextureDesc> containedTextures{imageInfos[0]->usedInTexture};
            if (containedTextures.size() == 1) {
                return *imageInfos[0]->usedInTexture.begin();
            }

            for (uint32_t i = 1; i < imageInfos.size(); i++) {
                for (auto iter = containedTextures.begin(); iter != containedTextures.end();) {
                    if (!imageInfos[i]->usedInTexture.contains(*iter)) {
                        iter = containedTextures.erase(iter);

                        break;
                    }
                    else {
                        iter++;
                    }
                }
            }
            if (containedTextures.size() == 0) {
                //its possible that the textures were loaded in a different order
                //not adding a catch for that
                printf("descriptor was declared non-unique, but no match was found \n");
                throw std::runtime_error("was not unique, but no match found");
            }
            else if (containedTextures.size() > 1) {
                printf("more than one match was found for a descriptor \n");
                throw std::runtime_error("descriptor duplication error");
            }
            return *containedTextures.begin();
        }
    }


    TextureDesc Texture_Builder::createSimpleTexture(std::string path, bool global, bool mipmaps, VkShaderStageFlags shaderStage) {
        Texture_Manager* tmPtr = Texture_Manager::getTextureManagerPtr();

        Texture_Manager::ImageTracker* imageInfo;
        bool uniqueImage = false;

        std::string texPath{ TEXTURE_DIR };
        texPath += path;

        {
            auto tempImageInfo = tmPtr->imageMap.find(texPath);
            if (tempImageInfo != tmPtr->imageMap.end()) {
                imageInfo = tempImageInfo->second;
                for (auto iter = imageInfo->usedInTexture.begin(); iter != imageInfo->usedInTexture.end(); iter++) {
                    auto const& imageData = tmPtr->textureImages.at(*iter);
                    for (auto const& existingImage : imageData) {
                        if (existingImage->usedInTexture.size() == 1) {
                            return *iter;
                        }
                    }
                }

                if (imageInfo->usedInTexture.size() == 0) {
                    //its possible that the textures were loaded in a different order
                    //not adding a catch for that
                    printf("descriptor was declared non-unique, but no match was found \n");
                }
                throw std::runtime_error("was not unique, but no match found");
                //warnign silencing, this should never happen
                return 0;
            }
            else {
                uniqueImage = true;
                imageInfo = Texture_Manager::constructImageTracker(texPath, mipmaps);

                EWEDescriptorWriter descBuilder(*TextureDSLInfo::getSimpleDSL(tmPtr->device, shaderStage), DescriptorPool_Global);
                descBuilder.writeImage(0, imageInfo->imageInfo.getDescriptorImageInfo());
                TextureDesc retDesc = descBuilder.build();

                imageInfo->usedInTexture.insert(retDesc);
                tmPtr->textureImages.try_emplace(retDesc, std::vector<Texture_Manager::ImageTracker*>{imageInfo});

                //tmPtr->textureMap.emplace(tmPtr->currentTextureCount, descBuilder.build());
                if (!global) {
                    tmPtr->sceneIDs.push_back(retDesc);
                }
                tmPtr->currentTextureCount++;
                return retDesc;
            }
        }
    }



    Texture_Manager::Texture_Manager(EWEDevice& device) : device{ device }, imageTrackerBucket { sizeof(ImageTracker) } {
        textureManagerPtr = this;
    }



    void Texture_Manager::cleanup() {
        //call at end of program
        //printf("beginning of texture cleanup \n");
        //uint32_t tracker = 0;
        for (auto& image : imageMap) {
            //printf("%d tracking \n", tracker++);
            image.second->imageInfo.destroy(device);
            image.second->~ImageTracker();
            imageTrackerBucket.freeDataChunk(image.second);
        }
        imageMap.clear();
        textureImages.clear();
        //printf("before clear \n");
        //textureMap.clear();
        //printf("after clear \n");
        //printf("after texture map cleanup \n");
        //tracker = 0;



        //globalPool.reset();
        printf("end of texture cleanup \n");
    }

    //return value <flags, textureID>

    void Texture_Manager::clearSceneTextures() {
        //everythign created with a mode texture needs to be destroyed. if it persist thru modes, it needs to be a global texture
#ifdef _DEBUG
        //DBEUUGGIG TEXUTRE BEING CLEARED INCORRECTLY
        //printf("clearing texutre 29 - %s \n", textureMap.at(29).textureData.path.c_str());
        printf("clearing scene textures \n");
#endif

        for (auto& sceneID : sceneIDs) {

            removeMaterialTexture(sceneID);


            std::vector<ImageTracker*>& imageTrackers = textureImages.at(sceneID);
            for (auto& imageTracker : imageTrackers) {
#ifdef _DEBUG
                if (imageTracker->usedInTexture.erase(sceneID) == 0) {
                    printf("descriptor is using an image that isn't used in descriptor? \n");
                    throw std::runtime_error("tracked texture doesn't exist");
                }
#else
                imageTracker->usedInTexture.erase(sceneID);
#endif
                if (imageTracker->usedInTexture.size() == 0) {
                    imageTracker->imageInfo.destroy(device);
                    for (auto iter = imageMap.begin(); iter != imageMap.end(); iter++) {
                        if (iter->second == imageTracker) {
                            imageMap.erase(iter);
                            break;
                        }
                    }
                    imageTracker->~ImageTracker();
                    imageTrackerBucket.freeDataChunk(imageTracker);

                    textureImages.erase(sceneID);
                }
            }
            //textureMap.erase(sceneID);
        }
        sceneIDs.clear();
        printf("clear mode textures end \n");

    }
    void Texture_Manager::removeMaterialTexture(TextureDesc removeID) {
        for (auto iter = existingMaterials.begin(); iter != existingMaterials.end(); iter++) {
            if (iter->second.texture == removeID) {
                existingMaterials.erase(iter);
                break;

            }
        }
    }
    /*
    VkDescriptorSet* Texture_Manager::getDescriptorSet(TextureDesc textureID) {
        //printf("descriptor set ~ %d \n", textureID);

#ifdef _DEBUG
            //if (textureID == 13) {
            //	printf("texture string : %s \n", getTextureData(textureID).first.c_str());
            //}
        if (!textureManagerPtr->textureMap.contains(textureID)) {
            printf("TEXTURE DOES NOT EXIST  : %d \n", textureID);
            //printf("texture string : %s \n", getTextureData(textureID).path.c_str());
            throw std::exception("texture descriptor set doesnt exist");
        }
#endif
        return &textureManagerPtr->textureMap.at(textureID);
    }
    */
    Texture_Manager::ImageTracker* Texture_Manager::constructImageTracker(std::string const& path, bool mipmap) {
        ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(textureManagerPtr->imageTrackerBucket.getDataChunk());

        new(imageTracker) ImageTracker(textureManagerPtr->device, path, true);

        return textureManagerPtr->imageMap.try_emplace(path, imageTracker).first->second;
    }
    Texture_Manager::ImageTracker* Texture_Manager::constructEmptyImageTracker(std::string const& path) {

        ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(textureManagerPtr->imageTrackerBucket.getDataChunk());
        new(imageTracker) ImageTracker();

        return textureManagerPtr->imageMap.try_emplace(path, imageTracker).first->second;
    }
}