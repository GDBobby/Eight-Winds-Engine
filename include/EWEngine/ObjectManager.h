#pragma once
#include "EWEngine/EWE_GameObject.h"
#include "EWEngine/Graphics/PointLight.h"
//#include "FloatingRockSystem.h"
#include "EWEngine/GameObject2D.h"
//#include "EWEngine/graphicsAssimpModel.h"
#include "EWEngine/Collision.h"
#include "EWEngine/Systems/Rendering/Rigid/RigidRS.h"
#include "EWEngine/Graphics/EWE_Object.h"

namespace EWE {
	class ObjectManager {
	public:
		~ObjectManager() {
#if DECONSTRUCTION_DEBUG
			printf("object manager deconstructor \n");
#endif
		}

		std::vector<EWEGameObject> texturedGameObjects{};
		std::vector<EWEGameObject> transparentGameObjects{};
		std::vector<EWEGameObject> grassField{};

		std::vector<EweObject> eweObjects{};

		//this will get strange with transparent objects
		//std::unordered_map<uint8_t, std::pair<std::function<void(FrameInfo frameInfo)>, std::vector<EweObject>>> renderObjects;

		std::vector<PointLight> pointLights{};


		void InitCollision() {
			Collision::collisionObjects.clear();
			Collision::collisionObjects.push_back(&texturedGameObjects);
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
#if LEVEL_BUILDER
		void ClearBuilders();
		void ResetBuilders();
#endif
		void ClearSceneObjects();

		/*
		add callbacks, when the first item is added to a vector, bind the pipeline
		*/

	};
}