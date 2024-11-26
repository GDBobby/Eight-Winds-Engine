#pragma once

#include <EWEngine/Data/EngineDataTypes.h>

#include <EWEngine/Graphics/Texture/ImageFunctions.h>

#include <EWEngine/Data/MemoryTypeBucket.h>


#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <unordered_set>

namespace EWE {

	struct ImageTracker {
		ImageInfo imageInfo;
		uint8_t usageCount;
		bool zeroUsageDelete;
		ImageTracker(std::string const& path, bool mipmap, bool zeroUsageDelete) : imageInfo{ Image::CreateImage(path, mipmap) }, usageCount{ 1 }, zeroUsageDelete{ zeroUsageDelete } {}
		ImageTracker(ImageInfo& imageInfo, bool zeroUsageDelete) : imageInfo{ imageInfo }, usageCount{ 1 }, zeroUsageDelete{ zeroUsageDelete } {}
		ImageTracker(bool zeroUsageDelete = false) : imageInfo{}, usageCount{ 0 }, zeroUsageDelete { zeroUsageDelete} {}
	};

	class Image_Manager {
	private:
		//MemoryTypeBucket<1024> imageTrackerBucket;

	protected:

		std::vector<ImageID> sceneIDs; //keeping track so i can remove them later

		std::unordered_map<ImageID, ImageTracker*> imageTrackerIDMap{};
		std::unordered_map<std::string, ImageID> imageStringToIDMap{};
		std::unordered_map<ImageID, MaterialInfo> existingMaterialsByID{};
		
		//ImageInfo* skybox_image;
		//ImageInfo* UI_image;
#if EWE_DEBUG
		ImageID currentImageCount{ 0 };
#endif

		friend class Material_Image;

		static Image_Manager* imgMgrPtr;
		static ImageID ConstructImageTracker(std::string const& imagePath, bool mipmap, bool zeroUsageDelete = false);

	public:
		struct ImageReturn {
			ImageID imgID;
			ImageTracker* imgTracker;
		};
		//this is specifically for CubeImage
		static ImageReturn ConstructEmptyImageTracker(std::string const& path, bool zeroUsageDelete = false);
		static ImageID ConstructImageTracker(std::string const& path, ImageInfo& imageInfo, bool zeroUsageDelete = false);

		Image_Manager();

		static ImageID GetCreateImageID(std::string const& imagePath, bool mipmap, bool zeroUsageDelete = false) {
			auto findRet = imgMgrPtr->imageStringToIDMap.find(imagePath);
			if (findRet == imgMgrPtr->imageStringToIDMap.end()) {
				return ConstructImageTracker(imagePath, mipmap, zeroUsageDelete);
			}
			else {
				return findRet->second;
			}
		}

		static VkDescriptorImageInfo* GetDescriptorImageInfo(ImageID imgID) {
			return imgMgrPtr->imageTrackerIDMap.at(imgID)->imageInfo.GetDescriptorImageInfo();
		}
		static ImageID FindByPath(std::string const& path);

		//void ClearSceneImages();
		//void RemoveMaterialImage(TextureDesc removeID);
		static void RemoveImage(ImageID imgID);
		static void RemoveMaterialImage(ImageID imgID);
		void Cleanup();

		static Image_Manager* GetImageManagerPtr() { return imgMgrPtr; }
		static ImageID CreateUIImage();
		static ImageID CreateImageArray(std::vector<PixelPeek> const& pixelPeeks);
		//static void AddImageInfo(std::string const& path, ImageInfo& imageTemp, bool global);
	};
}

