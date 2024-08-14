#include "EWEngine/Graphics/Texture/Texture_Manager.h"
#include "EWEngine/Graphics/Texture/UI_Texture.h"

#ifndef TEXTURE_DIR
#define TEXTURE_DIR "textures/"
#endif

namespace EWE {


    Texture_Manager* Texture_Manager::textureManagerPtr{ nullptr };


    Texture_Builder::Texture_Builder(bool global) : global(global) {}

    void Texture_Builder::AddComponent(std::string const& texPath, VkShaderStageFlags stageFlags, bool mipmaps) {
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

    

    TextureDesc Texture_Builder::Build() {
        auto tmPtr = Texture_Manager::GetTextureManagerPtr();
        
        //size_t myHash = Texture_Manager::hashTexture(paths);

        TextureDSLInfo dslInfo{};
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //this is 1
        EWEDescriptorSetLayout::Builder dslBuilder{};

        uint16_t imageCount = 0;
        for (uint8_t i = 0; i < 6; i++) {
            dslInfo.SetStageTextureCount(stageFlags, imageCI[i].size());
            stageFlags <<= 1;
            imageCount += imageCI[i].size();
        }
        dslInfo.SetStageTextureCount(VK_SHADER_STAGE_ALL_GRAPHICS, imageCI[6].size());
        imageCount += imageCI[6].size();
        dslInfo.SetStageTextureCount(VK_SHADER_STAGE_ALL, imageCI[7].size());
        imageCount += imageCI[7].size();
        dslInfo.SetStageTextureCount(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, imageCI[8].size());
        imageCount += imageCI[8].size();

        std::vector<ImageTracker*> imageInfos{};
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
                    imageInfos[currentImage] = Texture_Manager::ConstructImageTracker(conInfo.path, conInfo.mipmaps);
                }
                currentImage++;
            }
        }

        if (uniqueDescriptor) {
            EWEDescriptorWriter descBuilder(dslInfo.GetDescSetLayout(), DescriptorPool_Global);
            for (uint16_t i = 0; i < imageInfos.size(); i++) {
                descBuilder.writeImage(i, imageInfos[i]->imageInfo.GetDescriptorImageInfo());
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


    TextureDesc Texture_Builder::CreateSimpleTexture(const std::string path, bool global, bool mipmaps, VkShaderStageFlags shaderStage) {
        Texture_Manager* tmPtr = Texture_Manager::GetTextureManagerPtr();

        ImageTracker* imageInfo;
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
                imageInfo = Texture_Manager::ConstructImageTracker(texPath, mipmaps);

                EWEDescriptorWriter descBuilder(TextureDSLInfo::GetSimpleDSL(shaderStage), DescriptorPool_Global);
                descBuilder.writeImage(0, imageInfo->imageInfo.GetDescriptorImageInfo());
                TextureDesc retDesc = descBuilder.build();

                imageInfo->usedInTexture.insert(retDesc);
                tmPtr->textureImages.try_emplace(retDesc, std::vector<ImageTracker*>{imageInfo});

                //tmPtr->textureMap.emplace(tmPtr->currentTextureCount, descBuilder.build());
                if (!global) {
                    tmPtr->sceneIDs.push_back(retDesc);
                }
                tmPtr->currentTextureCount++;
                return retDesc;
            }
        }
    }



    Texture_Manager::Texture_Manager() : imageTrackerBucket { sizeof(ImageTracker) } {
        textureManagerPtr = this;
    }

    void Texture_Manager::Cleanup() {
        //call at end of program
#if DECONSTRUCTION_DEBUG
        printf("beginning of texture cleanup \n");
#endif
        //uint32_t tracker = 0;

        for (auto& image : imageMap) {
            //printf("%d tracking \n", tracker++);
            image.second->imageInfo.Destroy();
            image.second->~ImageTracker();
            imageTrackerBucket.FreeDataChunk(image.second);
        }
        imageMap.clear();
        textureImages.clear();
        existingMaterials.clear();
        //printf("before clear \n");
        //textureMap.clear();
        //printf("after clear \n");
        //printf("after texture map cleanup \n");
        //tracker = 0;



        //globalPool.reset();
#if DECONSTRUCTION_DEBUG
        printf("end of texture cleanup \n");
#endif
    }

    //return value <flags, textureID>

    void Texture_Manager::ClearSceneTextures() {
        //everythign created with a mode texture needs to be destroyed. if it persist thru modes, it needs to be a global texture
#ifdef _DEBUG
        //DBEUUGGIG TEXUTRE BEING CLEARED INCORRECTLY
        //printf("clearing texutre 29 - %s \n", textureMap.at(29).textureData.path.c_str());
        printf("clearing scene textures \n");
#endif

        for (auto& sceneID : sceneIDs) {

            RemoveMaterialTexture(sceneID);


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
                    imageTracker->imageInfo.Destroy();
                    for (auto iter = imageMap.begin(); iter != imageMap.end(); iter++) {
                        if (iter->second == imageTracker) {
                            
                            imageMap.erase(iter);
                            break;
                        }
                    }
                    imageTracker->~ImageTracker();
                    imageTrackerBucket.FreeDataChunk(imageTracker);

                    textureImages.erase(sceneID);
                }
            }
            //textureMap.erase(sceneID);
        }
        sceneIDs.clear();
        printf("clear mode textures end \n");

    }
    void Texture_Manager::RemoveMaterialTexture(TextureDesc removeID) {
        for (auto iter = existingMaterials.begin(); iter != existingMaterials.end(); iter++) {
            if (iter->second.texture == removeID) {
                existingMaterials.erase(iter);
                break;

            }
        }
    }

    ImageTracker* Texture_Manager::ConstructImageTracker(std::string const& path, bool mipmap) {
        ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(textureManagerPtr->imageTrackerBucket.GetDataChunk());

        new(imageTracker) ImageTracker(path, true);

        return textureManagerPtr->imageMap.try_emplace(path, imageTracker).first->second;
    }
    ImageTracker* Texture_Manager::ConstructImageTracker(std::string const& path, ImageInfo& imageInfo) {
        ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(textureManagerPtr->imageTrackerBucket.GetDataChunk());
        new(imageTracker) ImageTracker(imageInfo);
        return textureManagerPtr->imageMap.try_emplace(path, imageTracker).first->second;
    
    }

    ImageTracker* Texture_Manager::ConstructEmptyImageTracker(std::string const& path) {

        ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(textureManagerPtr->imageTrackerBucket.GetDataChunk());
        new(imageTracker) ImageTracker();

        return textureManagerPtr->imageMap.try_emplace(path, imageTracker).first->second;
    }


    TextureDesc Texture_Manager::CreateTextureArray(std::vector<PixelPeek> const& pixelPeeks) {

        ImageInfo arrayImageInfo{};

        UI_Texture::CreateUIImage(arrayImageInfo, pixelPeeks, Queue::transfer);
        UI_Texture::CreateUIImageView(arrayImageInfo);
        UI_Texture::CreateUISampler(arrayImageInfo);

        arrayImageInfo.descriptorImageInfo.sampler = arrayImageInfo.sampler;
        arrayImageInfo.descriptorImageInfo.imageView = arrayImageInfo.imageView;
        arrayImageInfo.descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        EWEDescriptorWriter descBuilder(TextureDSLInfo::GetSimpleDSL(VK_SHADER_STAGE_FRAGMENT_BIT), DescriptorPool_Global);

        descBuilder.writeImage(0, &arrayImageInfo.descriptorImageInfo);
        TextureDesc retDesc = descBuilder.build();

        //cubeVector.emplace_back(EWETexture(eweDevice, texPath, tType_cube));
        //tmPtr->textureMap.emplace(tmPtr->currentTextureCount, );
        auto tmPtr = GetTextureManagerPtr();
        tmPtr->UI_ID = retDesc;
        tmPtr->currentTextureCount++;
        return retDesc;
    }
    TextureDesc Texture_Manager::CreateUITexture() {
        const std::vector<std::string> uiNames = {
            "NineUI.png",
            "NineFade.png",
            "clickyBox.png",
            "bracketButton.png",
            "bracketSlide.png",
            "unchecked.png",
            "buttonUp.png",
            "checked.png",
            "menuBase.png",
            //"roundButton.png",
        };
        std::vector<PixelPeek> pixelPeeks{};
        pixelPeeks.reserve(6);

        for (auto const& name : uiNames) {
            std::string individualPath = "textures/UI/";
            individualPath += name;

            pixelPeeks.emplace_back(individualPath);

        }
        return CreateTextureArray(pixelPeeks);
    }

    TextureDesc Texture_Manager::AddImageInfo(std::string const& path, ImageInfo& imageTemp, VkShaderStageFlagBits shaderStage, bool global){
        Texture_Manager* tmPtr = Texture_Manager::GetTextureManagerPtr();

        ImageTracker* imageTracker;
        bool uniqueImage = false;

        std::string texPath{ TEXTURE_DIR };
        texPath += path;


#ifdef _DEBUG
        if (tmPtr->imageMap.find(path) != tmPtr->imageMap.end()) {
            assert(false && "image should not be constructed outside of the texture manager if it already exist");
            return VK_NULL_HANDLE;
        }
#endif

        uniqueImage = true;
        imageTracker = Texture_Manager::ConstructImageTracker(path, imageTemp);

        EWEDescriptorWriter descBuilder(TextureDSLInfo::GetSimpleDSL(shaderStage), DescriptorPool_Global);
        descBuilder.writeImage(0, imageTracker->imageInfo.GetDescriptorImageInfo());
        TextureDesc retDesc = descBuilder.build();

        imageTracker->usedInTexture.insert(retDesc);
        tmPtr->textureImages.try_emplace(retDesc, std::vector<ImageTracker*>{imageTracker});

        //tmPtr->textureMap.emplace(tmPtr->currentTextureCount, descBuilder.build());
        if (!global) {
            tmPtr->sceneIDs.push_back(retDesc);
        }
        tmPtr->currentTextureCount++;
        return retDesc;
    }
    ImageTracker* Texture_Manager::FindByPath(std::string const& path) {
        auto foundImage = textureManagerPtr->imageMap.find(path);
        if (foundImage == textureManagerPtr->imageMap.end()) {
            return nullptr;
        }
        return foundImage->second;
    }
    TextureDesc Texture_Manager::EmplaceSkyboxImageTracker(ImageTracker* imageTracker, std::string const& texPath) {

        EWEDescriptorWriter descBuilder(TextureDSLInfo::GetSimpleDSL(VK_SHADER_STAGE_FRAGMENT_BIT), DescriptorPool_Global);

        descBuilder.writeImage(0, imageTracker->imageInfo.GetDescriptorImageInfo());
        TextureDesc retDesc = descBuilder.build();

        textureManagerPtr->textureImages.try_emplace(retDesc, std::vector<ImageTracker*>{imageTracker});
        textureManagerPtr->imageMap.emplace(texPath, imageTracker);

        textureManagerPtr->skyboxID = retDesc;
        textureManagerPtr->currentTextureCount++;

        return retDesc;
    }
}