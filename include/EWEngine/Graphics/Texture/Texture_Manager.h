#pragma once

#include <EWEngine/Data/EngineDataTypes.h>

#include <EWEngine/Graphics/Texture/Texture.h>

#include <EWEngine/Data/MemoryTypeBucket.h>


#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <unordered_set>

namespace EWE {
	struct Texture_Builder {
	protected:

		//scene textures are automatically cleaned upon exit of the current scene. 
		//For more fine tuned control, use global and define your own deconstruction points.
		bool global; //false for scene
		struct ImageConstructionInfo {
			std::string path;
			bool mipmaps;

			ImageConstructionInfo(std::string const& path, bool mipmaps) : path{ path }, mipmaps{mipmaps} {}
		};

	public:
		std::array<std::vector<ImageConstructionInfo>, 16> imageCI{}; //these are paths, split up into which stage it is going to

		Texture_Builder(bool global);

		//MAYBE default stage flags to FRAG stage
		void AddComponent(std::string const& texPath, VkShaderStageFlags stageFlags, bool mipmaps);

		TextureDesc Build();

		static TextureDesc CreateSimpleTexture(std::string texPath, bool global, bool mipmaps, VkShaderStageFlags shaderStage);
	};

	struct ImageTracker {
		ImageInfo imageInfo;
		std::unordered_set<TextureDesc> usedInTexture{};
		ImageTracker(std::string const& path, bool mipmap) : imageInfo{path, mipmap} {}
		ImageTracker() : imageInfo{} {}
	};

	class Texture_Manager {
	private:
		MemoryTypeBucket<1024> imageTrackerBucket;
		friend struct Texture_Builder;

	protected:
		std::vector<TextureDesc> sceneIDs; //keeping track so i can remove them later

		std::unordered_map<std::string, ImageTracker*> imageMap{};

		//EWEDescriptorPool::freeDescriptors(DescriptorPool_Global, descriptorSets);
		//std::unordered_map<TextureID, VkDescriptorSet> textureMap{};
		std::unordered_map<std::string, MaterialTextureInfo> existingMaterials{};
		std::unordered_map<TextureDesc, std::vector<ImageTracker*>> textureImages;
		

		TextureDesc skyboxID;
		TextureDesc UI_ID;

		uint32_t currentTextureCount{ 0 };

		friend class Cube_Texture;
		friend class Material_Texture;
		friend class Texture_Builder;

		static Texture_Manager* textureManagerPtr;
		static ImageTracker* ConstructImageTracker(std::string const& path, bool mipmap);

		//this is specifically for CubeImage
		static ImageTracker* ConstructEmptyImageTracker(std::string const& path);

	public:
		Texture_Manager();

		static VkDescriptorImageInfo* GetDescriptorImageInfo(std::string const& imageName) {
			return textureManagerPtr->imageMap.at(imageName)->imageInfo.GetDescriptorImageInfo();
		}

		void ClearSceneTextures();
		void RemoveMaterialTexture(TextureDesc removeID);
		void Cleanup();

		static Texture_Manager* GetTextureManagerPtr() { return textureManagerPtr; }
		static TextureDesc CreateUITexture();
		static TextureDesc CreateTextureArray(std::vector<PixelPeek> const& pixelPeeks);
		static TextureDesc AddImageInfo(std::string const& path, ImageInfo& imageInfo);
		//static VkDescriptorSet* getDescriptorSet(TextureID textureID);
		//static VkDescriptorSet* getSkyboxDescriptorSet() { return &textureManagerPtr->textureMap.at(textureManagerPtr->skyboxID); }



		//EWETexture::TextureData getTextureData(uint32_t textureID) { return textureMap.at(textureID).textureData; }
	};
}

