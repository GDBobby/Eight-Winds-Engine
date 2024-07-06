#include "EWEngine/ObjectManager.h"

#include <iostream>
#include "EWEngine/Graphics/Texture/Texture_Manager.h"


namespace EWE {

#if LEVEL_BUILDER
	void ObjectManager::ClearBuilders() {
		lBuilderObjects.clear();
		spriteBuildObjects.clear();
		resetBuilders();
	}
	void ObjectManager::ResetBuilders() {
		auto RigidRenderingSystem = RigidRenderingSystem::getRigidRSInstance();
		for (int i = 0; i < dynamicBuildObjects.size(); i++) {
			materialHandler->removeByTransform(dynamicBuildObjects[i].textureID, &dynamicBuildObjects[i].transform);
		}
		dynamicBuildObjects.clear();
		auto clearTextures = RigidRenderingSystem::getRigidRSInstance()->checkAndClearTextures();
		for (int i = 0; i < clearTextures.size(); i++) {
			EWETexture::removeSmartTexture(clearTextures[i]);
		}
	}

#endif
	void ObjectManager::ClearSceneObjects() {
		texturedGameObjects.clear();
		grassField.clear();
		printf("clearing ewe objects \n");


		for (int i = 0; i < materialGameObjects.size(); i++) {
			RigidRenderingSystem::RemoveByTransform(materialGameObjects[i].textureID, &materialGameObjects[i].transform);
		}
		materialGameObjects.clear();
		eweObjects.clear();
		printf("after clearing ewe \n");
		auto clearTextures = RigidRenderingSystem::CheckAndClearTextures();
		printf("afterr clear texutres \n");
		Texture_Manager* tmPtr = Texture_Manager::GetTextureManagerPtr();
		for (auto& texture : clearTextures) {
			//printf("each texture : %zu \n", texture);
			tmPtr->RemoveMaterialTexture(texture);
		}
		for (int i = 0; i < clearTextures.size(); i++) {
			//printf("each smart texture : %d \n", clearTextures[i]);
			tmPtr->ClearSceneTextures();
		}
		printf("after removing play objects \n");
	}
}