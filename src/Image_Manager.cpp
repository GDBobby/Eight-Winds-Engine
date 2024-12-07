#include "EWEngine/Graphics/Texture/Image_Manager.h"
#include "EWEngine/Graphics/Texture/UI_Texture.h"

#ifndef TEXTURE_DIR
#define TEXTURE_DIR "textures/"
#endif

namespace EWE {


    Image_Manager* Image_Manager::imgMgrPtr{ nullptr };


    Image_Manager::Image_Manager() {// : imageTrackerBucket{sizeof(ImageTracker)} {
        assert(imgMgrPtr == nullptr);
        imgMgrPtr = this;
    }

    void Image_Manager::Cleanup() {
        //call at end of program
#if DECONSTRUCTION_DEBUG
        printf("beginning of texture cleanup \n");
#endif
        //uint32_t tracker = 0;

        for (auto& image : imageTrackerIDMap) {
            //printf("%d tracking \n", tracker++);
            Image::Destroy(image.second->imageInfo);
            image.second->~ImageTracker();
            Deconstruct(image.second);
            //imageTrackerBucket.FreeDataChunk(image.second);
        }
        for (auto& texDSL : simpleTextureLayouts) {
            Deconstruct(texDSL.second);
        }
        simpleTextureLayouts.clear();

        //globalPool.reset();
#if DECONSTRUCTION_DEBUG
        printf("end of texture cleanup \n");
#endif
    }

    //return value <flags, textureID>
    /*
    void Image_Manager::ClearSceneTextures() {
        //everythign created with a mode texture needs to be destroyed. if it persist thru modes, it needs to be a global texture
#if EWE_DEBUG
        //DBEUUGGIG TEXUTRE BEING CLEARED INCORRECTLY
        //printf("clearing texutre 29 - %s \n", textureMap.at(29).textureData.path.c_str());
        printf("clearing scene textures \n");
#endif

        for (auto& sceneID : sceneIDs) {

            RemoveMaterialTexture(sceneID);


            std::vector<ImageTracker*>& imageTrackers = textureImages.at(sceneID);
            for (auto& imageTracker : imageTrackers) {
#if EWE_DEBUG
                assert(imageTracker->usedInTexture.erase(sceneID) > 0 && "descriptor is using an image that isn't used in descriptor?");
#else
                imageTracker->usedInTexture.erase(sceneID);
#endif
                if (imageTracker->usedInTexture.size() == 0) {
                    Image::Destroy(imageTracker->imageInfo);
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
    void Image_Manager::RemoveMaterialTexture(TextureDesc removeID) {
        for (auto iter = existingMaterials.begin(); iter != existingMaterials.end(); iter++) {
            if (iter->second.texture == removeID) {
                existingMaterials.erase(iter);
                break;

            }
        }
    }
    */

    void Image_Manager::RemoveImage(ImageID imgID) {
        std::unique_lock<std::mutex> uniq_lock(imgMgrPtr->imageMutex);
#if EWE_DEBUG
        assert(imgMgrPtr->imageTrackerIDMap.contains(imgID));
#endif
        auto& atRet = imgMgrPtr->imageTrackerIDMap.at(imgID);
#if EWE_DEBUG
        assert(atRet->usageCount > 1);
#endif
        atRet->usageCount--;
        if (atRet->usageCount == 0 && atRet->zeroUsageDelete) {
            imgMgrPtr->imageTrackerIDMap.erase(imgID);
            auto materialFindRet = imgMgrPtr->existingMaterialsByID.find(imgID);
            if (materialFindRet != imgMgrPtr->existingMaterialsByID.end()) {
                imgMgrPtr->existingMaterialsByID.erase(imgID);
            }
        }
    }


    ImageID Image_Manager::ConstructImageTracker(std::string const& path, bool mipmap, Queue::Enum whichQueue, bool zeroUsageDelete) {
        //ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(imgMgrPtr->imageTrackerBucket.GetDataChunk());

        //new(imageTracker) ImageTracker(path, mipmap, zeroUsageDelete);
        ImageTracker* imageTracker = Construct<ImageTracker>({ path, mipmap, whichQueue, zeroUsageDelete });
        std::unique_lock<std::mutex> uniq_lock(imgMgrPtr->imageMutex);
        imgMgrPtr->imageTrackerIDMap.try_emplace(imgMgrPtr->currentImageCount, imageTracker);
        return imgMgrPtr->currentImageCount++;
    }
    ImageID Image_Manager::ConstructImageTracker(std::string const& path, VkSampler sampler, bool mipmap, Queue::Enum whichQueue, bool zeroUsageDelete) {
        //ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(imgMgrPtr->imageTrackerBucket.GetDataChunk());

        //new(imageTracker) ImageTracker(path, mipmap, zeroUsageDelete);
        ImageTracker* imageTracker = Construct<ImageTracker>({ path, sampler, mipmap, whichQueue, zeroUsageDelete });

        std::unique_lock<std::mutex> uniq_lock(imgMgrPtr->imageMutex);
        imgMgrPtr->imageTrackerIDMap.try_emplace(imgMgrPtr->currentImageCount, imageTracker);
        return imgMgrPtr->currentImageCount++;
    }
    ImageID Image_Manager::ConstructImageTracker(std::string const& path, ImageInfo& imageInfo, bool zeroUsageDelete) {
        //ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(imgMgrPtr->imageTrackerBucket.GetDataChunk());
        //new(imageTracker) ImageTracker(imageInfo, zeroUsageDelete);
        ImageTracker* imageTracker = Construct<ImageTracker>({ imageInfo, zeroUsageDelete });

        std::unique_lock<std::mutex> uniq_lock(imgMgrPtr->imageMutex);
        imgMgrPtr->imageTrackerIDMap.try_emplace(imgMgrPtr->currentImageCount, imageTracker);
        return imgMgrPtr->currentImageCount++;
    }

    Image_Manager::ImageReturn Image_Manager::ConstructEmptyImageTracker(std::string const& path, bool zeroUsageDelete) {

        //ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(imgMgrPtr->imageTrackerBucket.GetDataChunk());
        //new(imageTracker) ImageTracker(zeroUsageDelete);
        ImageTracker* imageTracker = Construct<ImageTracker>({ zeroUsageDelete });

        std::unique_lock<std::mutex> uniq_lock(imgMgrPtr->imageMutex);
        const ImageID tempID = imgMgrPtr->currentImageCount++;
        return ImageReturn{ tempID, imgMgrPtr->imageTrackerIDMap.try_emplace(tempID, imageTracker).first->second };
    }


    ImageID Image_Manager::CreateImageArray(std::vector<PixelPeek> const& pixelPeeks, bool mipmapping, Queue::Enum whichQueue) {

        ImageTracker* imageTracker = Construct<ImageTracker>({});
        ImageInfo* arrayImageInfo = &imageTracker->imageInfo;
#if EWE_DEBUG
        printf("before ui image\n");
#endif

        if (whichQueue == Queue::graphics) {
            arrayImageInfo->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            arrayImageInfo->destinationImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        else {
            arrayImageInfo->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            arrayImageInfo->destinationImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        arrayImageInfo->width = pixelPeeks[0].width;
        arrayImageInfo->height = pixelPeeks[0].height;
        UI_Texture::CreateUIImage(*arrayImageInfo, pixelPeeks, mipmapping, whichQueue);
#if EWE_DEBUG
        printf("after ui image\n");
#endif
        UI_Texture::CreateUIImageView(*arrayImageInfo);
        UI_Texture::CreateUISampler(*arrayImageInfo);

        arrayImageInfo->descriptorImageInfo.sampler = arrayImageInfo->sampler;
        arrayImageInfo->descriptorImageInfo.imageView = arrayImageInfo->imageView;
        //arrayImageInfo->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        std::unique_lock<std::mutex> uniq_lock(imgMgrPtr->imageMutex);
        ImageID ret = imgMgrPtr->currentImageCount;
        imgMgrPtr->imageTrackerIDMap.try_emplace(ret, imageTracker);
        imgMgrPtr->currentImageCount++;
        return ret;
        //return arrayImageInfo;
    }
    ImageID Image_Manager::CreateUIImage() {
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
        pixelPeeks.reserve(uiNames.size());

        for (auto const& name : uiNames) {
            std::string individualPath = "textures/UI/";
            individualPath += name;

            pixelPeeks.emplace_back(individualPath);

        }
        return CreateImageArray(pixelPeeks, false, Queue::graphics);
        //GetimgMgrPtr()->UI_image = uiImageInfo;
    }

    ImageID Image_Manager::FindByPath(std::string const& path) {
        std::unique_lock<std::mutex> uniq_lock(imgMgrPtr->imageMutex);
        auto foundImage = imgMgrPtr->imageStringToIDMap.find(path);
        if (foundImage == imgMgrPtr->imageStringToIDMap.end()) {
            return IMAGE_INVALID;
        }
        return foundImage->second;
    }

    EWEDescriptorSetLayout* Image_Manager::GetSimpleTextureDSL(VkShaderStageFlags stageFlags) {
        EWEDescriptorSetLayout* simpleTextureDSL;
        std::unique_lock<std::mutex> uniq_lock(imgMgrPtr->imageMutex);
        auto findRet = imgMgrPtr->simpleTextureLayouts.find(stageFlags);
        if (findRet == imgMgrPtr->simpleTextureLayouts.end()) {
            EWEDescriptorSetLayout::Builder builder{};
            builder.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stageFlags);
            simpleTextureDSL = builder.Build();
            imgMgrPtr->simpleTextureLayouts.emplace(stageFlags, simpleTextureDSL);
        }
        else {
            simpleTextureDSL = findRet->second;
        }
        return simpleTextureDSL;
    }

    VkDescriptorSet Image_Manager::CreateSimpleTexture(std::string const& imagePath, bool mipMap, Queue::Enum whichQueue, VkShaderStageFlags stageFlags, bool zeroUsageDelete) {
        assert(false && "this needs to be fixed up, it's not waiting for the transfer to graphics transition");

        ImageID imgID = GetCreateImageID(imagePath, mipMap, whichQueue, zeroUsageDelete);

        EWEDescriptorWriter descWriter{ GetSimpleTextureDSL(stageFlags), DescriptorPool_Global };
#if DESCRIPTOR_IMAGE_IMPLICIT_SYNCHRONIZATION
        descWriter.WriteImage(imgID);
#else
        descWriter.WriteImage(0, GetDescriptorImageInfo(imgID));
#endif
        return descWriter.Build();
    }

#if DESCRIPTOR_IMAGE_IMPLICIT_SYNCHRONIZATION
    VkDescriptorSet Image_Manager::CreateSimpleTexture(ImageID imageID, VkShaderStageFlags stageFlags) {
#else
    VkDescriptorSet Image_Manager::CreateSimpleTexture(VkDescriptorImageInfo* imageInfo, VkShaderStageFlags stageFlags) {
#endif

        EWEDescriptorWriter descWriter{ GetSimpleTextureDSL(stageFlags), DescriptorPool_Global };
#if DESCRIPTOR_IMAGE_IMPLICIT_SYNCHRONIZATION
        descWriter.WriteImage(imageID);
#else
        descWriter.WriteImage(0, imageInfo);
#endif
        return descWriter.Build();
    }

    ImageID Image_Manager::GetCreateImageID(std::string const& imagePath, bool mipmap, Queue::Enum whichQueue, bool zeroUsageDelete) {
        imgMgrPtr->imageMutex.lock();
        auto findRet = imgMgrPtr->imageStringToIDMap.find(imagePath);
        if (findRet == imgMgrPtr->imageStringToIDMap.end()) {
            imgMgrPtr->imageMutex.unlock();
            return ConstructImageTracker(imagePath, mipmap, whichQueue, zeroUsageDelete);
        }
        else {
            imgMgrPtr->imageMutex.unlock();
            return findRet->second;
        }
    }
    ImageID Image_Manager::GetCreateImageID(std::string const& imagePath, VkSampler sampler, bool mipmap, Queue::Enum whichQueue, bool zeroUsageDelete) {
        imgMgrPtr->imageMutex.lock();
        auto findRet = imgMgrPtr->imageStringToIDMap.find(imagePath);
        if (findRet == imgMgrPtr->imageStringToIDMap.end()) {
            imgMgrPtr->imageMutex.unlock();
            return ConstructImageTracker(imagePath, sampler, mipmap, whichQueue, zeroUsageDelete);
        }
        else {
            imgMgrPtr->imageMutex.unlock();
            return findRet->second;
        }
    }
}