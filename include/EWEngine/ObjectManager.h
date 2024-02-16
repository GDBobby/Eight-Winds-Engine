#pragma once
#include "EWE_GameObject.h"
#include "EWEngine/Graphics/PointLight.h"
//#include "FloatingRockSystem.h"
#include "GameObject2D.h"
//#include "EWEngine/graphicsAssimpModel.h"
#include "EWEngine/Graphics/EWE_Object.h"
#include "Collision.h"
#include "EWEngine/Systems/Rendering/Rigid/RigidRS.h"

namespace EWE {
	class ObjectManager {
	public:
		~ObjectManager() {
			printf("object manager deconstructor \n");
		}

		//FOR A DAY IN THE FUTURE
		//std::map<Pipeline_Enum, std::vector<EWEGameObject>> objectMap;

		std::vector<EWEGameObject> texturedGameObjects{};
		std::vector<EWEGameObject> materialGameObjects{};
		std::vector<EWEGameObject> transparentGameObjects{};
		std::vector<EWEGameObject> grassField{};

		std::vector<EweObject> eweObjects{};

		//this will get strange with transparent objects
		//std::unordered_map<uint8_t, std::pair<std::function<void(FrameInfo frameInfo)>, std::vector<EweObject>>> renderObjects;

		std::vector<PointLight> pointLights{};

		// not currently active
		//std::vector<SpotLight> spotLights;

		//global right now because i only have 1, need to make it scene based
		std::pair<std::unique_ptr<EWEModel>, TextureDesc> skybox{}; //model and textureID


		void initCollision() {
			Collision::collisionObjects.clear();
			Collision::collisionObjects.push_back(&texturedGameObjects);
			Collision::collisionObjects.push_back(&materialGameObjects);
		}

#if LEVEL_BUILDER
		std::map<uint32_t, EWEGameObject> lBuilderObjects; //grid, and other gridlike objects?????????? might nto need a map for this, or might want to consolidate
		//std::map<uint32_t, EWEGameObject> builderObjects;
		std::map<uint32_t, EWEGameObject> dynamicBuildObjects;
		//std::map<uint32_t, EWEGameObject> transparentBuildObjects;
		std::map<uint32_t, EWEGameObject> spriteBuildObjects;
		
#endif

		//std::pair<MaterialFlags, TextureID> targetTexturePair{0,0};
		//uint32_t maxTargets = 0;
		//uint32_t currentActiveTargets = 0;
		TextureDesc grassTextureID{TEXTURE_UNBINDED_DESC};
#if LEVEL_BUILDER
		void clearBuilders();
		void resetBuilders();
#endif
		void clearSceneObjects(EWEDevice& device);

		/*
		add callbacks, when the first item is added to a vector, bind the pipeline
		*/

	};
}