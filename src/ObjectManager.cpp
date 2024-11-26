#include "EWEngine/ObjectManager.h"

#include <iostream>
#include "EWEngine/Graphics/Texture/Image_Manager.h"


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

		eweObjects.clear();
		printf("after clearing ewe \n");
		assert(false && " need to fix this bakc up");
		/* need to come back here
		auto clearTextures = RigidRenderingSystem::CheckAndClearTextures();
		printf("afterr clear texutres \n");
		Image_Manager* imPtr = Image_Manager::GetTextureManagerPtr();
		for (auto& texture : clearTextures) {
			//printf("each texture : %zu \n", texture);
			//imPtr->RemoveMaterialTexture(texture);
		}
		for (int i = 0; i < clearTextures.size(); i++) {
			//printf("each smart texture : %d \n", clearTextures[i]);
			//imPtr->ClearSceneTextures();
		}
		printf("after removing play objects \n");
		*/
	}
}