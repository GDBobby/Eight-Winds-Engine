#pragma once
#include "Device.hpp"
#include "Descriptors.h"
#include "Device_Buffer.h"

#include <stb/stb_image.h>

#include <unordered_map>
#include <map>
#include "EWEngine/Data/EngineDataTypes.h"	

/*
~EWETexture() {
	printf("texture destruction \n");
	//why was this not already here
	// nvm taking it back out, can't figure out how to stop textures from getting moved in the vectors
	destroy();
}
*/

#define MAX_SMART_TEXTURE_COUNT 6

namespace EWE {

	class EWETexture {
// ~~~~~~~~~~~~~~ rules for the class? idk ~~~~~~~~~~~~~~~
		//EWETexture(const EWETexture&) = delete; //why
		//EWETexture& operator=(const EWETexture&) = delete;
		//EWETexture(EWETexture&&) = default;
		//EWETexture& operator=(EWETexture&&) = default;

// ~~~~~~~~~~~~~~~~~ data structures ~~~~~~~~~~~~~~~~~~~~
	public:

		enum texture_type {
			tType_simple,
			tType_cube,

			tType_material,

			tType_none,
		};

		struct TextureData {
			std::string path;
			texture_type tType;
			ShaderFlags materialFlags;
		};
	protected:
		struct PixelPeek {
			stbi_uc* pixels = nullptr;
			int width = 0;
			int height = 0;
			int channels = 0;

			PixelPeek() {}
			//dont currrently care about desired channels, but maybe one day
		};

// ~~~~~~~~~~~~~~~ static portion ~~~~~~~~~~~~~~~~~~~~~~
	private:
		static std::array<std::string, 6> fileNames;

		//static std::vector<uint32_t> globalIDs; //shouldnt need to keep track of global IDs, so not necessary
		static std::vector<TextureID> sceneIDs; //keeping track so i can remove them later

		//static int32_t createSimpleVertTexture(EWEDevice& device, std::string texPath);
		static TextureID createSimpleTexture(EWEDevice& device, std::string texPath, texture_type tType = tType_simple);
		static TextureID createCubeTexture(EWEDevice& device, std::string texPath);
		static std::pair<ShaderFlags, TextureID> createMaterialTexture(EWEDevice& device, std::string texPath, bool smart);
		//static int32_t createOrbTexture(EWEDevice& eweDevice, std::string texPath);

		static std::unordered_map<std::string, TextureID> existingIDs;
		static std::unordered_map<std::string, std::pair<uint8_t, TextureID>> existingMaterialIDs;
		static std::unordered_map<std::string, TextureID> existingUIs;
		static std::unordered_map<TextureID, EWETexture> textureMap;
		static std::unordered_map<TextureID, EWETexture> uiMap;

		static std::unique_ptr<EWEDescriptorSetLayout> simpleDescSetLayout;

		static std::vector<std::unique_ptr<EWEDescriptorSetLayout>> dynamicDescSetLayout;

		static std::vector<std::vector<std::string>> smartTextureTypes;

		static TextureID skyboxID;

		static TextureID returnID;
		static TextureID uiID;
	public:

		static void initStaticVariables();
		static void buildSetLayouts(EWEDevice& eweDevice);

		static void clearSceneTextures();
		static void removeSmartTexture(TextureID removeID);
		static void cleanup();

		static TextureID addUITexture(EWEDevice& eweDevice, std::string texPath, texture_type tType = tType_simple);
		static TextureID addGlobalTexture(EWEDevice& eweDevice, std::string texPath, texture_type tType = tType_simple);
		static TextureID addSceneTexture(EWEDevice& eweDevice, std::string texPath, texture_type tType = tType_simple);


		static std::pair<ShaderFlags, TextureID> addGlobalMaterialTexture(EWEDevice& eweDevice, std::string texPath);
		static std::pair<ShaderFlags, TextureID> addSceneMaterialTexture(EWEDevice& eweDevice, std::string texPath);

		static VkDescriptorSet* getDescriptorSets(TextureID textureID, uint8_t frame);
		static VkDescriptorSet* getSkyboxDescriptorSets(uint8_t frame) { return &textureMap.at(skyboxID).descriptorSets[frame]; }
		static VkDescriptorSet* getUIDescriptorSets(TextureID textureID, uint8_t frame) {
#ifdef _DEBUG
			if (uiMap.find(textureID) == uiMap.end()) {
				printf("trying to find a UI descriptor set that doesnt exist \n");
				throw std::exception("ui texture does not exist");
			}
#endif
			return &uiMap.at(textureID).descriptorSets[frame]; 
		}
		static VkDescriptorSet* getSpriteDescriptorSets(uint32_t textureID, uint8_t frame) {
			//textureMap.at(textureID).spriteFrameUpdate(frame);
			return &textureMap.at(textureID).descriptorSets[frame];
		}

		static VkDescriptorSetLayout getSimpleDescriptorSetLayout() { return simpleDescSetLayout->getDescriptorSetLayout(); }
		static VkDescriptorSetLayout getDynamicDescriptorSetLayout(uint8_t imageCount) { return dynamicDescSetLayout[imageCount]->getDescriptorSetLayout(); }

		static size_t getUIMapSize() { return uiMap.size(); }
		static TextureData getTextureData(uint32_t textureID) { return textureMap.at(textureID).textureData; }

// ~~~~~~~~~~~~~~~~~~~~~ individual class portion ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	protected:
		//EWETexture(EWEDevice& device) : EWEDevice{ device } {}
		EWETexture(std::string texPath, EWEDevice& device, std::vector<PixelPeek>& pixelPeek, texture_type tType = tType_simple);
		EWETexture(std::string texPath, EWEDevice& device, std::vector<PixelPeek>& pixelPeek, texture_type tType, ShaderFlags flags);
		void destroy();

		void createTextureImage(std::vector<PixelPeek>& pixelPeek);
		//void createTextureImage(std::string fileName);

		void createCubeImage(std::vector<PixelPeek>& pixelPeek);
		//void createCubeImage(std::string filePath);

		void createTextureImageView(texture_type tType = tType_simple);
		void createTextureSampler(texture_type tType = tType_simple);
		void generateMipmaps(VkFormat imageFormat);

		void createSimpleVertDescriptor();
		void createSimpleDescriptor();
		void createDynamicDescriptor(uint16_t imageCount);
	private:
		TextureData textureData;
		//std::pair<std::string, texture_type> textureData; //change to a struct, if i need to return more data later

		EWEDevice& eweDevice;

		//std::vector<std::unique_ptr<EWEBuffer>> spriteBuffers;
		std::vector<VkImageLayout> imageLayout;
		std::vector<VkImage> image;
		std::vector<VkDeviceMemory> imageMemory;
		std::vector<VkImageView> imageView;
		std::vector<VkSampler> sampler;

		std::vector<uint32_t> width;
		std::vector<uint32_t> height;
		std::vector<uint32_t> mipLevels;

		std::vector<VkDescriptorImageInfo> descriptor;
		std::vector<VkDescriptorSet> descriptorSets;
		//std::vector<VkDescriptorSet> 
	};
}