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


    ImageID Image_Manager::ConstructImageTracker(std::string const& path, bool mipmap, bool zeroUsageDelete) {
        //ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(imgMgrPtr->imageTrackerBucket.GetDataChunk());

        //new(imageTracker) ImageTracker(path, mipmap, zeroUsageDelete);
        ImageTracker* imageTracker = Construct<ImageTracker>({ path, mipmap, zeroUsageDelete });

        imgMgrPtr->imageTrackerIDMap.try_emplace(imgMgrPtr->currentImageCount, imageTracker);
        return imgMgrPtr->currentImageCount++;
    }
    ImageID Image_Manager::ConstructImageTracker(std::string const& path, ImageInfo& imageInfo, bool zeroUsageDelete) {
        //ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(imgMgrPtr->imageTrackerBucket.GetDataChunk());
        //new(imageTracker) ImageTracker(imageInfo, zeroUsageDelete);
        ImageTracker* imageTracker = Construct<ImageTracker>({imageInfo, zeroUsageDelete });


        imgMgrPtr->imageTrackerIDMap.try_emplace(imgMgrPtr->currentImageCount, imageTracker);
        return imgMgrPtr->currentImageCount++;
    }

    Image_Manager::ImageReturn Image_Manager::ConstructEmptyImageTracker(std::string const& path, bool zeroUsageDelete) {

        //ImageTracker* imageTracker = reinterpret_cast<ImageTracker*>(imgMgrPtr->imageTrackerBucket.GetDataChunk());
        //new(imageTracker) ImageTracker(zeroUsageDelete);
        ImageTracker* imageTracker = Construct<ImageTracker>({ zeroUsageDelete });

        const ImageID tempID = imgMgrPtr->currentImageCount++;

        return ImageReturn{ tempID, imgMgrPtr->imageTrackerIDMap.try_emplace(tempID, imageTracker).first->second };
    }


    ImageID Image_Manager::CreateImageArray(std::vector<PixelPeek> const& pixelPeeks) {

        auto empRet = imgMgrPtr->imageTrackerIDMap.try_emplace(imgMgrPtr->currentImageCount, Construct<ImageTracker>({}));
        ImageInfo* arrayImageInfo = &empRet.first->second->imageInfo;
#if EWE_DEBUG
        printf("before ui image\n");
#endif

        UI_Texture::CreateUIImage(*arrayImageInfo, pixelPeeks, Queue::transfer);
#if EWE_DEBUG
        printf("after ui image\n");
#endif
        UI_Texture::CreateUIImageView(*arrayImageInfo);
        UI_Texture::CreateUISampler(*arrayImageInfo);

        arrayImageInfo->descriptorImageInfo.sampler = arrayImageInfo->sampler;
        arrayImageInfo->descriptorImageInfo.imageView = arrayImageInfo->imageView;
        arrayImageInfo->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;



        //cubeVector.emplace_back(EWETexture(eweDevice, texPath, tType_cube));
        //tmPtr->textureMap.emplace(tmPtr->currentImageCount, );
        return imgMgrPtr->currentImageCount++;
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
        return CreateImageArray(pixelPeeks);
        //GetimgMgrPtr()->UI_image = uiImageInfo;
    }

//    void Image_Manager::AddImageInfo(std::string const& path, ImageInfo& imageTemp, bool global){
//        Image_Manager* tmPtr = Image_Manager::GetImageManagerPtr();
//
//        ImageTracker* imageTracker;
//
//        std::string texPath{ TEXTURE_DIR };
//        texPath += path;
//
//
//#if EWE_DEBUG
//        assert(!tmPtr->imageStringToIDMap.contains(path) && "image should not be constructed outside of the texture manager if it already exist");
//#endif
//        return Image_Manager::ConstructImageTracker(path, imageTemp, global);
//    }
    ImageID Image_Manager::FindByPath(std::string const& path) {
        auto foundImage = imgMgrPtr->imageStringToIDMap.find(path);
        if (foundImage == imgMgrPtr->imageStringToIDMap.end()) {
            return IMAGE_INVALID;
        }
        return foundImage->second;
    }
}