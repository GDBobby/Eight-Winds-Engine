#include "EWEngine/Graphics/Textures/Texture_Manager.h"

#ifndef TEXTURE_DIR
#define TEXTURE_DIR "textures/"
#endif

namespace EWE {


    Texture_Manager* Texture_Manager::textureManagerPtr{ nullptr };


    Texture_Builder::Texture_Builder(EWEDevice& device, bool global, bool mipmaps) : device(device), global(global), mipmaps{ mipmaps } {}

    void Texture_Builder::addComponent(std::string const& texPath, VkShaderStageFlags stageFlags) {
        switch (stageFlags) {
            case VK_SHADER_STAGE_VERTEX_BIT: {
                paths[0].emplace_back(texPath);
                break;
            }
            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: {
                paths[1].emplace_back(texPath);
                break;
            }
            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: {
                paths[2].emplace_back(texPath);
                break;
            }
            case VK_SHADER_STAGE_GEOMETRY_BIT: {
                paths[3].emplace_back(texPath);
                break;
            }
            case VK_SHADER_STAGE_FRAGMENT_BIT: {
                paths[4].emplace_back(texPath);
                break;
            }
            case VK_SHADER_STAGE_COMPUTE_BIT: {
                paths[5].emplace_back(texPath);
                break;
            }
            case VK_SHADER_STAGE_ALL_GRAPHICS: {
                //im pretty sure that _ALL_GRAPHICS and _ALL are both noob traps, but ill put them in anyways
                paths[6].emplace_back(texPath);
                break;
            }
            case VK_SHADER_STAGE_ALL: {
                paths[7].emplace_back(texPath);
                break;
            }
            case VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT: {
                paths[8].emplace_back(texPath);
                break;
            }
            default: {
                printf("unsupported texture shader stage flag \n");
                throw std::runtime_error("invalid shader stage flag for textures");
                break;
            }
        }
    }

    

    TextureID Texture_Builder::build() {
        auto tmPtr = Texture_Manager::getTextureManagerPtr();
        
        size_t myHash = Texture_Manager::hashTexture(paths);

        TextureDSLInfo dslInfo{};
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //this is 1
        EWEDescriptorSetLayout::Builder dslBuilder{ device };

        uint16_t imageCount = 0;
        for (uint8_t i = 0; i < 6; i++) {
            dslInfo.setStageTextureCount(stageFlags, paths[i].size());
            stageFlags <<= 1;
            imageCount += paths[i].size();
        }
        dslInfo.setStageTextureCount(VK_SHADER_STAGE_ALL_GRAPHICS, paths[6].size());
        imageCount += paths[6].size();
        dslInfo.setStageTextureCount(VK_SHADER_STAGE_ALL, paths[7].size());
        imageCount += paths[7].size();
        dslInfo.setStageTextureCount(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, paths[8].size());
        imageCount += paths[8].size();

        std::vector<Texture_Manager::ImageTracker*> imageInfos{};
        imageInfos.resize(imageCount, nullptr);

        bool uniqueDescriptor = false;
        uint16_t currentImage = 0;
        for (uint8_t i = 0; i < paths.size(); i++) {
            for (auto& path : paths[i]) {
                auto tempImageInfo = tmPtr->imageMap.find(path);
                if (tempImageInfo != tmPtr->imageMap.end()) {
                    imageInfos[currentImage] = tempImageInfo->second;
                }
                else {
                    uniqueDescriptor = true;
                    PixelPeek pixelPeek{ path };
                    imageInfos[currentImage] = tmPtr->imageMap.try_emplace(path, device, pixelPeek, true).first->second;
                }
                currentImage++;
            }
        }

        if (uniqueDescriptor) {
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            EWEDescriptorWriter descBuilder(*dslInfo.getDescSetLayout(device), DescriptorPool_Global);
            for (uint16_t i = 0; i < imageInfos.size(); i++) {
                descBuilder.writeImage(i, imageInfos[i]->imageInfo.getDescriptorImageInfo());
                imageInfos[i]->usedInTexture.insert(tmPtr->currentTextureCount);
            }
            if (!descBuilder.build(descriptorSet)) {
                //returnValue = false;
                printf("failed to construct texture descriptor\n");
                throw std::runtime_error("failed to construct texture descriptor");
            }
            tmPtr->textureMap.emplace(tmPtr->currentTextureCount, descriptorSet);
            if (!global) {
                tmPtr->sceneIDs.push_back(tmPtr->currentTextureCount);
            }
            return tmPtr->currentTextureCount++;
        }
        else {
            

            std::unordered_set<TextureID> containedTextures{imageInfos[0]->usedInTexture};
            if (containedTextures.size() == 1) {
                return *imageInfos[0]->usedInTexture.begin();
            }

            for (TextureID i = 1; i < imageInfos.size(); i++) {
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
    Texture_Manager::Texture_Manager() : imageTrackerBucket{ sizeof(ImageTracker) } {

    }


    void Texture_Manager::cleanup(EWEDevice& device) {
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
        deletionMap.clear();
        //printf("before clear \n");
        textureMap.clear();
        //printf("after clear \n");
        //printf("after texture map cleanup \n");
        //tracker = 0;



        //globalPool.reset();
        printf("end of texture cleanup \n");
    }

    //return value <flags, textureID>

    void Texture_Manager::clearSceneTextures(EWEDevice& device) {
        //everythign created with a mode texture needs to be destroyed. if it persist thru modes, it needs to be a global texture
#ifdef _DEBUG
        //DBEUUGGIG TEXUTRE BEING CLEARED INCORRECTLY
        //printf("clearing texutre 29 - %s \n", textureMap.at(29).textureData.path.c_str());
        printf("clearing scene textures \n");
#endif

        for (auto& sceneID : sceneIDs) {

            removeMaterialTexture(sceneID);


            ImageTracker* imageTracker = deletionMap.at(sceneID);
#ifdef _DEBUG
            if (imageTracker->usedInTexture.erase(sceneID) != 0) {
                printf("descriptor is using an image that isn't used in descriptor? \n");
                throw std::runtime_error("tracked texture doesn't exist");
            }
#else
            deletionmap.at(sceneID)->usedInTexture.erase(sceneID);
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

                deletionMap.erase(sceneID);
            }

            textureMap.erase(sceneID);
        }
        sceneIDs.clear();
        printf("clear mode textures end \n");

    }
    void Texture_Manager::removeMaterialTexture(TextureID removeID) {
        for (auto iter = existingMaterials.begin(); iter != existingMaterials.end(); iter++) {
            if (iter->second.textureID == removeID) {
                existingMaterials.erase(iter);
                break;

            }
        }
    }
    VkDescriptorSet* Texture_Manager::getDescriptorSet(TextureID textureID) {
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
}