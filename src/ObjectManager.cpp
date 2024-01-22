#include "EWEngine/ObjectManager.h"

#include <iostream>
#include "EWEngine/Graphics/Textures/Texture_Manager.h"


namespace EWE {

#if LEVEL_BUILDER
	void ObjectManager::clearBuilders() {
		lBuilderObjects.clear();
		spriteBuildObjects.clear();
		resetBuilders();
	}
	void ObjectManager::resetBuilders() {
		auto materialHandler = MaterialHandler::getMaterialHandlerInstance();
		for (int i = 0; i < dynamicBuildObjects.size(); i++) {
			materialHandler->removeByTransform(dynamicBuildObjects[i].textureID, &dynamicBuildObjects[i].transform);
		}
		dynamicBuildObjects.clear();
		auto clearTextures = MaterialHandler::getMaterialHandlerInstance()->checkAndClearTextures();
		for (int i = 0; i < clearTextures.size(); i++) {
			EWETexture::removeSmartTexture(clearTextures[i]);
		}
	}

#endif
	void ObjectManager::clearSceneObjects(EWEDevice& device) {
		texturedGameObjects.clear();
		grassField.clear();
		printf("clearing ewe objects \n");


		auto materialHandler = MaterialHandler::getMaterialHandlerInstance();
		for (int i = 0; i < dynamicGameObjects.size(); i++) {
			materialHandler->removeByTransform(dynamicGameObjects[i].textureID, &dynamicGameObjects[i].transform);
		}
		dynamicGameObjects.clear();
		eweObjects.clear();
		printf("after clearing ewe \n");
		auto clearTextures = materialHandler->checkAndClearTextures();
		printf("afterr clear texutres \n");
		Texture_Manager* tmPtr = Texture_Manager::getTextureManagerPtr();
		for (auto& texture : clearTextures) {
			printf("each texture : %d \n", texture);
			tmPtr->removeMaterialTexture(texture);
		}
		for (int i = 0; i < clearTextures.size(); i++) {
			printf("each smart texture : %d \n", clearTextures[i]);
			tmPtr->clearSceneTextures(device);
		}
		printf("after removing play objects \n");
	}
}