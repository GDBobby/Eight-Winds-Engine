#pragma once

#include <EWEngine/Data/EngineDataTypes.h>

#include <EWEngine/Graphics/Textures/Texture.h>

#include <EWEngine/Data/MemoryTypeBucket.h>


#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <unordered_set>

namespace EWE {
	struct Texture_Builder {
	protected:
		EWEDevice& device;

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

		Texture_Builder(EWEDevice& device, bool global);

		//MAYBE default stage flags to FRAG stage
		void addComponent(std::string const& texPath, VkShaderStageFlags stageFlags, bool mipmaps);

		TextureID build();

		static TextureID createSimpleTexture(std::string texPath, bool global, bool mipmaps, VkShaderStageFlags shaderStage);
	};

	class Texture_Manager {
	private:
		MemoryTypeBucket<1024> imageTrackerBucket;
		friend struct Texture_Builder;

	protected:
		std::vector<TextureID> sceneIDs; //keeping track so i can remove them later
		EWEDevice& device;

		//int32_t createSimpleVertTexture(EWEDevice& device, std::string texPath);

		//static size_t hashTexture(const std::array<std::vector<std::string>, SUPPORTED_STAGE_COUNT>& paths);


		//std::unordered_map<std::string, TextureID> existingIDs{};

		struct ImageTracker {
			ImageInfo imageInfo;
			std::unordered_set<TextureID> usedInTexture{};
			ImageTracker(EWEDevice& device, std::string const& path, bool mipmap) : imageInfo{device, path, mipmap} {}
			ImageTracker() : imageInfo{} {}
		};

		std::unordered_map<std::string, ImageTracker*> imageMap{};

		//EWEDescriptorPool::freeDescriptors(DescriptorPool_Global, descriptorSets);
		std::unordered_map<TextureID, VkDescriptorSet> textureMap{};
		std::unordered_map<std::string, MaterialTextureInfo> existingMaterials{};
		std::unordered_map<TextureID, ImageTracker*> deletionMap;
		

		TextureID skyboxID;

		TextureID currentTextureCount{ 0 };

		friend class Cube_Texture;
		friend class Material_Texture;
		friend class UI_Texture;
		friend class Texture_Builder;

		static Texture_Manager* textureManagerPtr;

	public:
		Texture_Manager(EWEDevice& device);

		void initStaticVariables();
		void buildSetLayouts();

		void clearSceneTextures();
		void removeMaterialTexture(TextureID removeID);
		void cleanup();

		static Texture_Manager* getTextureManagerPtr() { return textureManagerPtr; }
		static VkDescriptorSet* getDescriptorSet(TextureID textureID);
		static VkDescriptorSet* getSkyboxDescriptorSet() { return &textureManagerPtr->textureMap.at(textureManagerPtr->skyboxID); }


		static ImageTracker* constructImageTracker(std::string const& path, bool mipmap);

		//this is specifically for CubeImage
		static ImageTracker* constructEmptyImageTracker(std::string const& path);

		//EWETexture::TextureData getTextureData(uint32_t textureID) { return textureMap.at(textureID).textureData; }
	};
}

